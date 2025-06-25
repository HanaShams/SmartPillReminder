package com.example.smartpillreminder;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.List;
import java.util.Locale;

public class PillAdapter extends RecyclerView.Adapter<PillAdapter.PillViewHolder> {

    private Context context;
    private List<Pill> pillList;

    public PillAdapter(Context context, List<Pill> pillList) {
        this.context = context;
        this.pillList = pillList;
    }

    @NonNull
    @Override
    public PillViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        View view = LayoutInflater.from(context).inflate(R.layout.activity_schedule_adapter, parent, false);
        return new PillViewHolder(view);
    }

    @Override
    public void onBindViewHolder(@NonNull PillViewHolder holder, int position) {
        Pill pill = pillList.get(position);

        holder.tvPillName.setText(pill.getPillName());
        holder.tvTime.setText(formatTime(pill.getTime()));
        holder.tvDosage.setText(pill.getDosage());

        // Set reminder notification
        holder.itemView.setOnClickListener(v -> {
            Toast.makeText(context, "Reminder set for " + pill.getTime(), Toast.LENGTH_SHORT).show();
        });
    }

    private String formatTime(String time) {
        try {
            SimpleDateFormat fmt = new SimpleDateFormat("HH:mm", Locale.getDefault());
            Date date = fmt.parse(time);
            return new SimpleDateFormat("hh:mm a", Locale.getDefault()).format(date);
        } catch (Exception e) {
            return time;
        }
    }

    @Override
    public int getItemCount() {
        return pillList.size();
    }

    public static class PillViewHolder extends RecyclerView.ViewHolder {
        TextView tvPillName, tvTime, tvDosage;

        public PillViewHolder(@NonNull View itemView) {
            super(itemView);
            tvPillName = itemView.findViewById(R.id.tvPillName);
            tvTime = itemView.findViewById(R.id.tvTime);
            tvDosage = itemView.findViewById(R.id.tvDosage);
        }
    }
}