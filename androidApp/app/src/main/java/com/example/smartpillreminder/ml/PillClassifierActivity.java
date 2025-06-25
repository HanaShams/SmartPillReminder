package com.example.smartpillreminder.ml;

import android.os.Bundle;

import androidx.activity.EdgeToEdge;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.graphics.Insets;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;

import com.example.smartpillreminder.R;
import com.google.android.material.appbar.MaterialToolbar;


import android.Manifest;
import android.content.pm.PackageManager;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import android.content.Intent;
import android.graphics.Bitmap;
import android.net.Uri;
import android.os.Bundle;
import android.provider.MediaStore;
import android.util.Log;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;

import androidx.activity.EdgeToEdge;
import androidx.annotation.Nullable;
import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;

import org.tensorflow.lite.Interpreter;

import java.io.FileInputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.MappedByteBuffer;
import java.nio.channels.FileChannel;

public class PillClassifierActivity extends AppCompatActivity {
    private static final int SELECT_IMAGE = 1;
    private ImageView imageView;
    private TextView textViewResult;
    private Interpreter tflite;
    private final int INPUT_SIZE = 224;
    private final int PIXEL_SIZE = 3;
    private static final int TAKE_PHOTO = 2;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        EdgeToEdge.enable(this);
        setContentView(R.layout.activity_pill_classifier);

        ImageView backArrow = findViewById(R.id.backArr);
        backArrow.setOnClickListener(v -> onBackPressed());

        if (ContextCompat.checkSelfPermission(this, Manifest.permission.CAMERA)
                != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.CAMERA}, 100);
        }

        imageView = findViewById(R.id.imageView);
        textViewResult = findViewById(R.id.textViewResult);
        Button buttonSelect = findViewById(R.id.buttonSelect);

        try {
            tflite = new Interpreter(loadModelFile());
        } catch (IOException e) {
            e.printStackTrace();
        }

        buttonSelect.setOnClickListener(v -> {
            String[] options = {"Take Photo", "Choose from Gallery"};
            AlertDialog.Builder builder = new AlertDialog.Builder(PillClassifierActivity.this);
            builder.setTitle("Select Image Source");
            builder.setItems(options, (dialog, which) -> {
                if (which == 0) {
                    // Open camera
                    Intent cameraIntent = new Intent(MediaStore.ACTION_IMAGE_CAPTURE);
                    if (cameraIntent.resolveActivity(getPackageManager()) != null) {
                        startActivityForResult(cameraIntent, TAKE_PHOTO);
                    }
                } else {
                    // Open gallery
                    Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
                    intent.setType("image/*");
                    intent.addCategory(Intent.CATEGORY_OPENABLE);
                    startActivityForResult(Intent.createChooser(intent, "Select Image"), SELECT_IMAGE);
                }
            });
            builder.show();
        });

    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, @Nullable Intent data) {
        super.onActivityResult(requestCode, resultCode, data);

        if (resultCode == RESULT_OK) {
            Bitmap bitmap = null;

            if (requestCode == SELECT_IMAGE && data != null) {
                Uri imageUri = data.getData();
                try {
                    bitmap = MediaStore.Images.Media.getBitmap(this.getContentResolver(), imageUri);
                } catch (IOException e) {
                    e.printStackTrace();
                }
            } else if (requestCode == TAKE_PHOTO && data != null) {
                bitmap = (Bitmap) data.getExtras().get("data"); // 👈 Camera image (thumbnail)
            }

            if (bitmap != null) {
                imageView.setImageBitmap(bitmap);
                classifyImage(Bitmap.createScaledBitmap(bitmap, INPUT_SIZE, INPUT_SIZE, false));
            }
        }
    }


    private MappedByteBuffer loadModelFile() throws IOException {
        FileInputStream fileInputStream = new FileInputStream(getAssets().openFd("model_unquant.tflite").getFileDescriptor());
        FileChannel fileChannel = fileInputStream.getChannel();
        long startOffset = getAssets().openFd("model_unquant.tflite").getStartOffset();
        long declaredLength = getAssets().openFd("model_unquant.tflite").getDeclaredLength();
        return fileChannel.map(FileChannel.MapMode.READ_ONLY, startOffset, declaredLength);
    }

    private void classifyImage(Bitmap bitmap) {
        ByteBuffer inputBuffer = ByteBuffer.allocateDirect(4 * INPUT_SIZE * INPUT_SIZE * PIXEL_SIZE);
        inputBuffer.order(ByteOrder.nativeOrder());

        int[] intValues = new int[INPUT_SIZE * INPUT_SIZE];
        bitmap.getPixels(intValues, 0, INPUT_SIZE, 0, 0, INPUT_SIZE, INPUT_SIZE);

        for (int val : intValues) {
            inputBuffer.putFloat(((val >> 16) & 0xFF) / 255.f); // R
            inputBuffer.putFloat(((val >> 8) & 0xFF) / 255.f);  // G
            inputBuffer.putFloat((val & 0xFF) / 255.f);         // B
        }

        float[][] output = new float[1][6]; // Change to 6 for 6 categories
        tflite.run(inputBuffer, output);

        // Find the index with the highest confidence
        int maxIndex = 0;
        float maxConfidence = output[0][0];
        for (int i = 1; i < 6; i++) {
            if (output[0][i] > maxConfidence) {
                maxConfidence = output[0][i];
                maxIndex = i;
            }
        }

        // Update with your pill categories
        String[] labels = {"PainKiller", "ColdRelief", "Supplement", "BeautyCare", "Diabetic", "BPControl"};
        String label = labels[maxIndex];

        // Display result
        textViewResult.setText("Prediction: " + label + "\nConfidence: " + (maxConfidence * 100) + "%");
    }



}
