package com.xykj.studentsign;

import android.os.Bundle;
import android.support.annotation.Nullable;
import android.view.View;
import android.widget.EditText;
import android.widget.RadioButton;
import android.widget.RadioGroup;

import butterknife.BindView;
import butterknife.ButterKnife;

public class RegisterActivity extends BaseActivity {

    @BindView(R.id.et_no)
    EditText mEtNo;
    @BindView(R.id.et_name)
    EditText mEtName;
    @BindView(R.id.et_pwd)
    EditText mEtPwd;
    @BindView(R.id.rb_student)
    RadioButton mRbStudent;
    @BindView(R.id.rb_teacher)
    RadioButton mRbTeacher;
    @BindView(R.id.rg_role)
    RadioGroup mRgRole;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_register);
        ButterKnife.bind(this);
        setTitle("注册");

    }

    public void cancel(View view) {
    }

    public void register(View view) {
    }
}
