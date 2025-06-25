package com.example.smartpillreminder;

import android.os.Bundle;
import android.text.TextUtils;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TimePicker;
import android.widget.Toast;

import androidx.activity.EdgeToEdge;
import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.Toolbar;
import androidx.core.graphics.Insets;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;
// Firebase Database imports
import com.google.firebase.auth.FirebaseAuth;
import com.google.firebase.database.DatabaseReference;
import com.google.firebase.database.FirebaseDatabase;

import java.util.Locale;

public class AddScheduleActivity extends AppCompatActivity {

    private EditText etPillName, etDosage;
    private TimePicker timePicker;
    private DatabaseReference database;
    private String userId;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_add_schedule);

        // Toolbar with back button
        //Toolbar toolbar = findViewById(R.id.toolbar);
        //setSupportActionBar(toolbar);
        // getSupportActionBar().setDisplayHomeAsUpEnabled(true);

        // Firebase setup
        userId = FirebaseAuth.getInstance().getCurrentUser().getUid();
        database = FirebaseDatabase.getInstance().getReference("pills").child(userId);

        // Initialize views
        etPillName = findViewById(R.id.etPillName);
        etDosage = findViewById(R.id.etDosage);
        timePicker = findViewById(R.id.timePicker);
        Button btnSave = findViewById(R.id.btnSave);

        btnSave.setOnClickListener(v -> {
            String pillName = etPillName.getText().toString().trim();
            String dosage = etDosage.getText().toString().trim();
            int hour = timePicker.getHour();
            int minute = timePicker.getMinute();

            // Format time as HH:MM
            String time = String.format(Locale.getDefault(), "%02d:%02d", hour, minute);

            if (TextUtils.isEmpty(pillName)) {
                etPillName.setError("Pill name is required");
                return;
            }

            if (TextUtils.isEmpty(dosage)) {
                etDosage.setError("Dosage is required");
                return;
            }

            savePillSchedule(pillName, time, dosage);
        });
    }

    private void savePillSchedule(String pillName, String time, String dosage) {
        String pillId = database.push().getKey();
        Pill pill = new Pill(pillId, pillName, time, dosage);

        database.child(pillId).setValue(pill)
                .addOnSuccessListener(aVoid -> {
                    Toast.makeText(this, "Pill schedule saved", Toast.LENGTH_SHORT).show();
                    finish();
                })
                .addOnFailureListener(e -> {
                    Toast.makeText(this, "Failed to save: " + e.getMessage(), Toast.LENGTH_SHORT).show();
                });
    }

    @Override
    public boolean onSupportNavigateUp() {
        onBackPressed();
        return true;
    }
}