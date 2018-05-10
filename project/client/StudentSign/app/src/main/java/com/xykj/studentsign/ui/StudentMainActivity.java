package com.xykj.studentsign.ui;

import android.content.Intent;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.view.View;
import android.widget.TextView;
import android.widget.Toast;

import com.uuzuche.lib_zxing.activity.CaptureActivity;
import com.uuzuche.lib_zxing.activity.CodeUtils;
import com.xykj.studentsign.App;
import com.xykj.studentsign.R;
import com.xykj.studentsign.entity.Result;
import com.xykj.studentsign.net.Api;
import com.xykj.studentsign.ui.base.BaseActivity;

import butterknife.BindView;
import butterknife.ButterKnife;

public class StudentMainActivity extends BaseActivity {

    public static final int REQUEST_CODE_SIGN = 2001;
    public static final int REQUEST_CODE_JOIN_CLASS = 2002;
    @BindView(R.id.tv_class)
    TextView mTvClass;
    @BindView(R.id.tv_name)
    TextView mTvName;
    @BindView(R.id.tv_number)
    TextView mTvNumber;
    private Api mApi;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_student_main);
        ButterKnife.bind(this);
        mApi = Api.getInstance(this);

        mTvClass.setText(App.userInfo.getClassName());
        mTvName.setText(App.userName);
        mTvNumber.setText(App.userId);
    }


    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        //处理扫描结果（在界面上显示）
        if (null != data) {
            Bundle bundle = data.getExtras();
            if (bundle == null) {
                return;
            }
            if (bundle.getInt(CodeUtils.RESULT_TYPE) == CodeUtils.RESULT_SUCCESS) {
                String result = bundle.getString(CodeUtils.RESULT_STRING);
                Toast.makeText(this, "解析结果:" + result, Toast.LENGTH_LONG).show();

                if (requestCode == REQUEST_CODE_SIGN) {
                    signActive(result);
                } else if (requestCode == REQUEST_CODE_JOIN_CLASS) {
                    joinCLass(result);
                }
            } else if (bundle.getInt(CodeUtils.RESULT_TYPE) == CodeUtils.RESULT_FAILED) {
                Toast.makeText(this, "解析二维码失败", Toast.LENGTH_LONG).show();
            }
        }
    }

    private void joinCLass(String code) {

        showProgress();
        mApi.getJoinClass(code, new Api.Callback<Result>() {
            @Override
            public void OnSuccess(Result data) {
                closeProgress();
                if (Result.RESULT_SUCCESS.equals(data.getResult())) {
                    String msg = data.getMsg();
                    showToast("加入班级成功");

                    mTvClass.setText(msg);
                } else {
                    showToast(data.getMsg());
                }
            }

            @Override
            public void OnFailed(Exception e) {
                closeProgress();
                showNetError();
            }
        });
    }

    private void signActive(String code) {
        showProgress();
        mApi.getSignActive(code, new Api.Callback<Result>() {
            @Override
            public void OnSuccess(Result data) {
                closeProgress();
                if (Result.RESULT_SUCCESS.equals(data.getResult())) {
                    String msg = data.getMsg();
                    showToast(msg);
                } else {
                    showToast(data.getMsg());
                }
            }

            @Override
            public void OnFailed(Exception e) {
                closeProgress();
                showNetError();
            }
        });
    }


    public void sign(View view) {
        scan(REQUEST_CODE_SIGN);
    }

    public void joinClass(View view) {
        scan(REQUEST_CODE_JOIN_CLASS);
    }

    public void scan(int requestCode) {
        Intent intent = new Intent(this, CaptureActivity.class);
        startActivityForResult(intent, requestCode);
    }
}
