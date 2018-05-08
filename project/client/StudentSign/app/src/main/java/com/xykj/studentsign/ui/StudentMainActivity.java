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
import com.xykj.studentsign.ui.base.BaseActivity;

import butterknife.BindString;
import butterknife.BindView;
import butterknife.ButterKnife;

public class StudentMainActivity extends BaseActivity {

    public static final int REQUEST_CODE_SIGN = 2001;
    public static final int REQUEST_CODE_JOIN_CLASS = 2002;
    @BindView(R.id.tv_info)
    TextView mTvInfo;
    @BindString(R.string.user_holder)
    String strUserHolder;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_student_main);
        ButterKnife.bind(this);

        mTvInfo.setText(String.format(strUserHolder, App.userName, App.userId, App.userInfo.getClassName()));
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
                    //todo sign
                } else if (requestCode == REQUEST_CODE_JOIN_CLASS) {
                    //todo join class
                }
            } else if (bundle.getInt(CodeUtils.RESULT_TYPE) == CodeUtils.RESULT_FAILED) {
                Toast.makeText(this, "解析二维码失败", Toast.LENGTH_LONG).show();
            }

        }


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
