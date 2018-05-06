package com.xykj.studentsign.ui;

import android.content.Intent;
import android.os.Bundle;
import android.view.View;

import com.xykj.studentsign.R;
import com.xykj.studentsign.ui.base.BaseActivity;

public class TeacherMainActivity extends BaseActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
    }

    public void myActivity(View view) {
        Intent intent = new Intent(this, ActiveListActivity.class);
        startActivity(intent);
    }

    public void myClass(View view) {
        Intent intent = new Intent(this, ClassListActivity.class);
        startActivity(intent);
    }
}
