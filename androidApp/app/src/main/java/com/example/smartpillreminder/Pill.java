package com.example.smartpillreminder;

public class Pill {
    private String pillId;
    private String pillName;
    private String time;
    private String dosage;

    public Pill() {
        // Required for Firebase
    }

    public Pill(String pillId, String pillName, String time, String dosage) {
        this.pillId = pillId;
        this.pillName = pillName;
        this.time = time;
        this.dosage = dosage;
    }

    // Getters and setters
    public String getPillId() { return pillId; }
    public String getPillName() { return pillName; }
    public String getTime() { return time; }
    public String getDosage() { return dosage; }
}