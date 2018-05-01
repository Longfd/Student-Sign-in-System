package com.xykj.studentsign;

import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.design.widget.TextInputEditText;
import android.support.design.widget.TextInputLayout;
import android.text.TextUtils;
import android.view.View;

import butterknife.BindView;
import butterknife.ButterKnife;

public class LoginActivity extends BaseActivity {

    @BindView(R.id.tie_no)
    TextInputEditText mTieNo;
    @BindView(R.id.til_no)
    TextInputLayout mTilNo;
    @BindView(R.id.tie_pwd)
    TextInputEditText mTiePwd;
    @BindView(R.id.til_pwd)
    TextInputLayout mTilPwd;
    private String mNo;
    private String mPwd;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_login);
        ButterKnife.bind(this);

        setTitle("登录");

    }

    public void login(View view) {
        mNo = mTieNo.getText().toString().trim();
        mPwd = mTiePwd.getText().toString().trim();


        if (TextUtils.isEmpty(mNo)) {
            mTilNo.setError("学号不能为空");
            return;
        }
        mTilNo.setError("");
        if (TextUtils.isEmpty(mPwd)) {
            mTilPwd.setError("密码不能为空");
            return;
        }
        mTilPwd.setError("");


    }
}
