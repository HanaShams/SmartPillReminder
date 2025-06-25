#define BLYNK_TEMPLATE_ID "TMPL6G2Y8ps1N"
#define BLYNK_TEMPLATE_NAME "SmartPillReminder"
#define BLYNK_AUTH_TOKEN "yC5TDHfvQNJJDmLRErqh0-B8ydAPrx7E"

#include <WiFi.h>
#include <Wire.h>
#include <RTClib.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <BlynkSimpleEsp32.h>
#include <FirebaseESP32.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Set your timezone offset in seconds (GMT+5:30 = 5.5*3600 = 19800)
#define TIMEZONE_OFFSET 19800 

// Firebase credentials
#define API_KEY "AIzaSyBFw9EyVjBCCBXTdXtiLdy6qSQlX1b3P0g"
#define DATABASE_URL "https://smartpillreminder-635cc-default-rtdb.firebaseio.com/"
#define USER_EMAIL "hanashams@gmail.com"
#define USER_PASSWORD "123456"

// WiFi credentials
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// NTP Client
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

// Hardware configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define PIR_PIN 4
#define BUZZER_PIN 33
#define LED_PIN 32

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
RTC_DS3231 rtc;

// Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

struct Pill {
  String time;
  String name;
  String dosage;
  bool taken;
};

Pill currentPill;
bool hasPillData = false;
unsigned long lastPirTrigger = 0;
const int pirCooldown = 5000;

// Time tracking variables
unsigned long lastTimeDisplayUpdate = 0;
const long timeDisplayInterval = 60000; // 1 minute
unsigned long lastTimeSync = 0;
const long timeSyncInterval = 3600000; // Sync with NTP every hour

// Function to compare pill times
bool isTimeToTakePill(const String &scheduledTime, const DateTime &now) {
    int colonPos = scheduledTime.indexOf(':');
    int scheduledHour = scheduledTime.substring(0, colonPos).toInt();
    int scheduledMinute = scheduledTime.substring(colonPos + 1).toInt();
    
    Serial.print("Time check: Current ");
    Serial.print(now.hour());
    Serial.print(":");
    Serial.print(now.minute());
    Serial.print(" vs Scheduled ");
    Serial.print(scheduledHour);
    Serial.print(":");
    Serial.println(scheduledMinute);
    
    return (now.hour() == scheduledHour && now.minute() == scheduledMinute);
}

void initializeOLED() {
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED initialization failed!");
    while(1);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.display();
}

void printRTCTime() {
  DateTime now = rtc.now();
  Serial.print(now.day());
  Serial.print("-");
  Serial.print(now.month());
  Serial.print("-");
  Serial.print(now.year());
  Serial.print(" ");
  Serial.print(now.hour());
  Serial.print(":");
  if (now.minute() < 10) Serial.print("0");
  Serial.print(now.minute());
  Serial.print(":");
  if (now.second() < 10) Serial.print("0");
  Serial.println(now.second());
}

void syncWithNTP() {
  Serial.println("Syncing RTC with NTP...");
  
  // Try multiple times to get NTP time
  for (int i = 0; i < 5; i++) {
    if (timeClient.update()) {
      break;
    }
    delay(1000);
  }
  
  if (!timeClient.isTimeSet()) {
    Serial.println("Failed to get NTP time!");
    return;
  }
  
  // Get current UTC time and apply offset
  time_t utc = timeClient.getEpochTime();
  time_t local = utc + TIMEZONE_OFFSET;
  
  // Set RTC with proper local time
  rtc.adjust(DateTime(local));
  
  Serial.print("NTP Time (UTC): ");
  Serial.println(timeClient.getFormattedTime());
  Serial.print("RTC Time (Local): ");
  printRTCTime();
}

void initializeRTC() {
  if(!rtc.begin()) {
    Serial.println("RTC initialization failed!");
    while(1);
  }
  
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, setting default time");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  
  Serial.print("Initial RTC time: ");
  printRTCTime();
}

void initializeFirebase() {
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  
  Serial.print("Authenticating");
  while(!Firebase.ready()) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nAuthenticated!");
}

bool fetchNextPill() {
  if(auth.token.uid.length() == 0) {
    Serial.println("User not authenticated");
    return false;
  }

  String path = "/pills/" + String(auth.token.uid.c_str());
  
  if(Firebase.getJSON(fbdo, path.c_str())) {
    FirebaseJson &json = fbdo.jsonObject();
    size_t count = json.iteratorBegin();
    
    DateTime now = rtc.now();
    Serial.print("Current RTC time in fetch: ");
    printRTCTime();
    
    Serial.println("\nChecking scheduled pills:");
    
    for(size_t i = 0; i < count; i++) {
      int type = 0;
      String key, value;
      json.iteratorGet(i, type, key, value);
      
      String pillPath = path + "/" + key;
      if(Firebase.getString(fbdo, (pillPath + "/time").c_str())) {
        String pillTime = fbdo.to<String>();
        Serial.print("Found scheduled pill at: ");
        Serial.print(pillTime);
        
        if(isTimeToTakePill(pillTime, now)) {
          Serial.println(" - MATCHED");
          if(Firebase.getString(fbdo, (pillPath + "/pillName").c_str())) {
            currentPill.name = fbdo.to<String>();
          }
          if(Firebase.getString(fbdo, (pillPath + "/dosage").c_str())) {
            currentPill.dosage = fbdo.to<String>();
          }
          currentPill.time = pillTime;
          currentPill.taken = false;
          hasPillData = true;
          json.iteratorEnd();
          return true;
        } else {
          Serial.println(" - Not matching");
        }
      }
    }
    json.iteratorEnd();
  }
  hasPillData = false;
  return false;
}

void triggerReminder() {
  for (int i = 0; i < 10; i++) {
    tone(BUZZER_PIN, 1000); // Start tone
    delay(500);
    noTone(BUZZER_PIN);     // Stop tone
    delay(300);
  }

  // // Buzzer pattern
  // tone(BUZZER_PIN, 1000, 500);
  // delay(1000);
  // tone(BUZZER_PIN, 1500, 500);
  

  Serial.println("Triggering reminder for: " + currentPill.name + " at " + currentPill.time);
  
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("TAKE YOUR PILL:");
  display.println(currentPill.name);
  display.print("Dosage: ");
  display.println(currentPill.dosage);
  display.display();

  lcd.clear();
  lcd.print("TAKE:");
  lcd.setCursor(0, 1);
  String pillLine = currentPill.name + " " + currentPill.dosage;
  lcd.print(pillLine.substring(0, 16));

  // Buzzer pattern
  // tone(BUZZER_PIN, 1000, 500);
  // delay(1000);
  // tone(BUZZER_PIN, 1500, 500);
  
  // Blynk notification
  String msg = "Take " + currentPill.name + " (" + currentPill.dosage + ") now!";
  Blynk.logEvent("pill_reminder", msg);
}

void logPillTaken() {
  // 1. Verify Firebase is ready
  if (!Firebase.ready()) {
    Serial.println("Firebase not ready - cannot log pill");
    return;
  }

  // 2. Create a filesystem-safe timestamp
  DateTime now = rtc.now();
  String timestamp = String(now.year()) + "_" + 
                   String(now.month()) + "_" + 
                   String(now.day()) + "_" + 
                   String(now.hour()) + "_" + 
                   (now.minute() < 10 ? "0" : "") + String(now.minute());

  Serial.println("Attempting to log pill at: " + timestamp);

  // 3. Create the database path
  String path = "/pill_records/" + String(auth.token.uid.c_str()) + "/" + timestamp;
  Serial.println("Database path: " + path);

  // 4. Prepare the data
  FirebaseJson json;
  json.set("pillName", currentPill.name.c_str());
  json.set("dosage", currentPill.dosage.c_str());
  json.set("scheduledTime", currentPill.time.c_str());  // The originally scheduled time
  json.set("actualTime", timestamp.c_str());          // When it was actually taken
  json.set("taken", true);

  // 5. Print JSON for debugging
  String jsonStr;
  json.toString(jsonStr, true);
  Serial.println("JSON payload:");
  Serial.println(jsonStr);

  // 6. Attempt write with retries
  int maxRetries = 3;
  for (int i = 0; i < maxRetries; i++) {
    if (Firebase.pushJSON(fbdo, path.c_str(), json)) {
      Serial.println("Pill intake logged successfully");
      currentPill.taken = true;
      return; // Success - exit function
    }
    
    Serial.print("Attempt " + String(i+1) + " failed: ");
    Serial.println(fbdo.errorReason());
    
    if (i < maxRetries - 1) {
      delay(1000); // Wait before retrying
    }
  }

  // 7. Final failure handling
  Serial.println("All attempts failed to log pill data");
  Serial.println("Error: " + fbdo.errorReason());
  Serial.println("Error code: " + String(fbdo.errorCode()));

  
}

void updateTimeDisplay() {
  DateTime now = rtc.now();
  char timeStr[] = "hh:mm:ss";
  char dateStr[] = "DD-MM-YYYY";
  
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Date: ");
  display.println(now.toString(dateStr));
  display.print("Time: ");
  display.println(now.toString(timeStr));
  
  if(hasPillData && !currentPill.taken) {
    display.println("Next Pill:");
    display.println(currentPill.time + " " + currentPill.name);
  } else {
    display.println("No pending pills");
  }
  
  display.display();
}

void setup() {
  Serial.begin(115200);
  
  pinMode(PIR_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  
  initializeOLED();
  initializeRTC();

  Wire.begin(); // If not already initialized
lcd.init();
lcd.backlight();
lcd.print("Pill Reminder");
delay(1000);
lcd.clear();
  
  WiFi.begin(ssid, pass);
  Serial.print("Connecting to WiFi");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected!");
  
  // Initialize NTP with retries
  timeClient.begin();
  timeClient.setTimeOffset(0); // We handle offset manually
  
  // Force initial sync with validation
  for (int i = 0; i < 3; i++) {
    syncWithNTP();
    if (rtc.now().year() > 2020) break; // Check if we got a reasonable date
    delay(1000);
  }
  
  initializeFirebase();
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  
  display.clearDisplay();
  display.println("System Ready");
  display.println("Time synced");
  display.display();
  delay(2000);
  
  fetchNextPill();
}

void loop() {
  Blynk.run();
  
  // Sync with NTP periodically
  if(millis() - lastTimeSync >= timeSyncInterval) {
    syncWithNTP();
    lastTimeSync = millis();
  }

  // Update display and check pills every minute
  if(millis() - lastTimeDisplayUpdate >= timeDisplayInterval) {
    updateTimeDisplay();
    lastTimeDisplayUpdate = millis();
    fetchNextPill();
  }
  
  DateTime now = rtc.now();
  if(hasPillData && !currentPill.taken) {
    if(isTimeToTakePill(currentPill.time, now)) {
      Serial.println("PILL TIME MATCHED - TRIGGERING REMINDER");
      triggerReminder();
    }
  }
  
  // Check PIR sensor
  if(digitalRead(PIR_PIN) == HIGH && millis() - lastPirTrigger > pirCooldown) {
    lastPirTrigger = millis();
    
    if(hasPillData && !currentPill.taken) {
      currentPill.taken = true;
      digitalWrite(LED_PIN, HIGH);
      
      display.clearDisplay();
      display.println("Pill Taken:");
      display.println(currentPill.name);
      display.display();

      // DateTime now = rtc.now();
  lcd.clear();
  lcd.print("PILL TAKEN:");
  lcd.setCursor(0, 1);
  String timeStr = String(now.hour()) + ":" + 
                  (now.minute() < 10 ? "0" : "") + String(now.minute());
  lcd.print(timeStr);
  delay(2000); // Show confirmation for 2 seconds
  lcd.clear();
      
      logPillTaken();
      Blynk.virtualWrite(V1, "Pill Taken: " + currentPill.name);
      
      delay(1000);
      digitalWrite(LED_PIN, LOW);
      updateTimeDisplay();
    }
  }
  
  delay(100);
}