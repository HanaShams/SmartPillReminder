package com.example.smartpillreminder;

public class User {
    private String uid;
    private String name;
    private String email;
    private long createdAt;

    public User() {
        // Required for Firebase
    }

    public User(String uid, String name, String email) {
        this.uid = uid;
        this.name = name;
        this.email = email;
        this.createdAt = System.currentTimeMillis();
    }

    // Getters and setters
    public String getUid() { return uid; }
    public String getName() { return name; }
    public String getEmail() { return email; }
    public long getCreatedAt() { return createdAt; }
}