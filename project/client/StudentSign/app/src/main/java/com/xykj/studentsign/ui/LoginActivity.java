package com.xykj.studentsign.ui;

import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.design.widget.TextInputEditText;
import android.support.design.widget.TextInputLayout;
import android.support.v4.app.ActivityCompat;
import android.text.TextUtils;
import android.util.Log;
import android.view.View;

import com.xykj.studentsign.App;
import com.xykj.studentsign.R;
import com.xykj.studentsign.entity.UserInfo;
import com.xykj.studentsign.net.Api;
import com.xykj.studentsign.ui.base.BaseActivity;

import java.util.ArrayList;
import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;

import static com.xykj.studentsign.entity.UserInfo.ROLE_STUDENT;

public class LoginActivity extends BaseActivity {
    public static final int PERMISSION_REQUEST_CODE = 0xab;
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
        request();
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
        showProgress();
        Api.getInstance(getApplicationContext()).login(mNo, mPwd, new Api.Callback<UserInfo>() {
            @Override
            public void OnSuccess(UserInfo data) {
                closeProgress();

                App.userId = data.getUserId();
                App.userName = data.getUserName();
                App.role = data.getRole();
                App.userInfo = data;

                showToast("登录成功!");
                Class cls = TeacherMainActivity.class;
                if (ROLE_STUDENT.equals(App.role)) {
                    cls = StudentMainActivity.class;
                }
                startActivity(new Intent(LoginActivity.this, cls));
                finish();
            }

            @Override
            public void OnFailed(Exception e) {
                closeProgress();
                e.printStackTrace();
                showNetError();
            }
        });

    }

    public void register(View view) {
        startActivity(new Intent(this, RegisterActivity.class));
    }

    private void request() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            List<String> pList = new ArrayList<>();
            for (String permission : App.permissions) {
                if (ActivityCompat.checkSelfPermission(this, permission) != PackageManager.PERMISSION_GRANTED) {
                    pList.add(permission);
                }
            }
            String[] pArray = pList.toArray(new String[]{});
            if (pArray.length <= 0) {
                return;
            }
            ActivityCompat.requestPermissions(this, pArray, PERMISSION_REQUEST_CODE);
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        for (int result : grantResults) {
            Log.d(TAG, "grantResults: " + result);
            if (result == PackageManager.PERMISSION_DENIED) {
//                finish();
                break;
            }
        }
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
    }
}
