package com.xykj.studentsign.ui;

import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.TextView;

import com.xykj.studentsign.App;
import com.xykj.studentsign.R;
import com.xykj.studentsign.ui.base.BaseActivity;

import butterknife.BindView;
import butterknife.ButterKnife;

public class TeacherMainActivity extends BaseActivity {

    @BindView(R.id.tv_name)
    TextView mTvName;
    @BindView(R.id.tv_number)
    TextView mTvNumber;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        ButterKnife.bind(this);

        mTvName.setText(App.userName);
        mTvNumber.setText(App.userId);
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
