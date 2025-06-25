# 💊 Smart Pill Reminder System

An IoT-based smart healthcare project that helps users manage their medication using real-time notifications, sensor integration, cloud syncing, and machine learning. Built with Firebase, ESP32, Blynk, and Android, this system ensures timely reminders and verification of pill intake.

---

## 📱 Features

* 🔐 User Registration/Login using **Firebase Authentication (Email/Password)**
* ⏰ Pill Scheduling using **Time Picker** in the mobile app
* 🔔 Real-time pill reminders via:

  * OLED Display
  * Buzzer Alert
  * Blynk Notification
* 🚶 PIR Sensor detects motion (pill taken)
* ☁️ Firebase Realtime Database integration
* 🕒 Real-time clock sync with **RTC + NTP**
* 🤖 ML-based Pill Recognition using **TFLite image classification**

---

## ⚙️ Technologies Used

| Component           | Purpose                                   |
| ------------------- | ----------------------------------------- |
| ESP32               | Main microcontroller                      |
| Firebase            | Authentication + Real-time Database       |
| Blynk IoT Platform  | Notification and remote control interface |
| OLED Display (I2C)  | Visual alerts                             |
| Buzzer              | Audible alerts                            |
| PIR Motion Sensor   | Detect user approach                      |
| RTC Module (DS3231) | Real-time tracking                        |
| NTP Client          | Sync system time via internet             |
| TFLite Model        | Image-based pill classification           |

---

## 🧐 System Workflow

**STEP 1**: User registers and adds pill schedule via app
**STEP 2**: Schedule saved to Firebase under the user ID
**STEP 3**: ESP32 fetches schedule data from Firebase
**STEP 4**: RTC + NTP ensure accurate time tracking
**STEP 5**: When time matches → buzzer + OLED alert + Blynk notification
**STEP 6**: PIR detects motion → pill marked as taken
**STEP 7**: ML model predicts and verifies pill image *(optional step)*

---

## 🔌 Hardware Circuit

| Component    | ESP32 Pin        |
| ------------ | ---------------- |
| OLED Display | SDA: 21, SCL: 22 |
| Buzzer       | GPIO 33 or CMD   |
| PIR Sensor   | GPIO 4           |
| LED (status) | GPIO 32          |
| Power        | 5V via USB       |
| Wi-Fi        | Wokwi-GUEST      |

*Simulated using Wokwi (or real hardware setup)*

---

## 🗖️ Folder Structure

```
SmartPillReminder/
├── androidApp/            # Android Studio app project
├── output/                # Screenshots, images, ML output
├── wokwi_vs/              # Wokwi circuit JSON + ESP32 code
├── README.md              # Project documentation
```

---

## 🚀 Getting Started

### 📱 Android App Setup

1. Open `androidApp/` in Android Studio
2. Connect to Firebase:

   * Enable **Email/Password Auth**
   * Setup **Realtime Database**
   * Add `google-services.json`
3. Use `TimePicker` to schedule pills

### ⚙️ ESP32 Code Setup

1. Open `wokwi_vs/` in Arduino IDE or on [wokwi.com](https://wokwi.com)
2. Install libraries:

   * Firebase ESP32
   * Blynk
   * Adafruit GFX + SSD1306
   * NTPClient
   * LiquidCrystal I2C
3. Update these credentials in the code:

```cpp
#define BLYNK_TEMPLATE_ID "TMPL6G2Y8ps1N"
#define BLYNK_TEMPLATE_NAME "SmartPillReminder"
#define BLYNK_AUTH_TOKEN "yC5TDHfvQNJJDmLRErqh0-B8ydAPrx7E"

#define API_KEY "AIzaSyBFw9EyVjBCCBXTdXtiLdy6qSQlX1b3P0g"
#define DATABASE_URL "https://smartpillreminder-635cc-default-rtdb.firebaseio.com/"
#define USER_EMAIL "hanashams@gmail.com"
#define USER_PASSWORD "123456"

char ssid[] = "Wokwi-GUEST";
char pass[] = "";
```

> ⚠️ Replace the above with your own keys (blynk & firebase) before uploading

---

## 🔔 Blynk Notification Setup

1. Go to [https://blynk.cloud](https://blynk.cloud)
2. Create a Template named `SmartPillReminder`
3. Add an **Event**:

   * **Event Name**: `pill_reminder`
   * **Description**: `Pill time alert`
   * ✅ Enable Notification
   * ✅ Enable for All Devices
   * ⏱ Optionally set limits (e.g., 1 per minute)
4. Use this in code:

```cpp
  Blynk.logEvent("pill_reminder", msg);
```

---

## 🤖 ML Pill Recognition (App)

* Built using TensorFlow Lite model trained with pill images
* Model Input: `224x224x3` RGB image
* In-app features:

  * Upload or take pill image
  * Show predicted label (e.g., BP Control, Pain killer, Suppliment)

**Current labels used:**

* PainKiller
* ColdRelief
* Supplement
* BeautyCare
* Diabetic
* BpControl

*Future enhancement*: 
    Verify that ML prediction matches the scheduled pill name from Firebase.
    You can also enhance accuracy by training the model with real-time pill images labeled with their medical names and adding more pill categories (labels) for broader detection.
---

## 📌 Future Improvements

* 🔊 Add **voice reminders** via speaker or text-to-speech
* 🧠 Full integration: Compare ML prediction with Firebase pill name
* 😶 PIR-triggered **auto-launch** of ML verification screen
* 🔍 App enhancements:

  * Add **search**, **filter**, and better **UI design**
  * Log missed doses

---

## 🔪 Sample Screenshots

![Login Screen](output/login.png)  
![Signup Screen](output/signup.png)  
![Time Schedule](output/time_schedule.png)  
![Firebase User & Data](output/firebase_user_data.png)  
![Scheduled Time](output/scheduled_time.png)  
![Blynk Notification](output/blynk_notification.png)  
![Email Notification](output/email_notification.png)  
![Buzzer Alert](output/buzzer_alert.png)  
![LED On (Motion Detect)](output/motion_detection.png)  
![ML Pill Verification](output/ml_verification.png)  

---

## 🙋‍♀️ Team Members

* Hana Hasmath
* Fathima Nuha
* Fathima Reeshma
* Fathima Kishor

---
