package com.lxj.studentsign.ui;

import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.EditText;
import android.widget.Toast;

import com.lxj.studentsign.R;
import com.lxj.studentsign.net.Api;

public class SettingActivity extends AppCompatActivity {

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_setting);
    }

    public void confirm(View view) {
        String ip = ((EditText) findViewById(R.id.et_ip)).getText().toString().trim();
        int port = Integer.parseInt(((EditText) findViewById(R.id.et_port)).getText().toString().trim());

        Api.ip = ip;
        Api.port = port;

        Toast.makeText(this, "设置成功!", Toast.LENGTH_SHORT).show();
        finish();
    }
}
