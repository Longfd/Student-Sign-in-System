package com.lxj.studentsign.ui;

import android.os.Bundle;
import android.support.annotation.Nullable;
import android.text.TextUtils;
import android.view.View;
import android.widget.EditText;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.Toast;

import com.lxj.studentsign.R;
import com.lxj.studentsign.entity.Result;
import com.lxj.studentsign.entity.UserInfo;
import com.lxj.studentsign.net.Api;
import com.lxj.studentsign.ui.base.BaseActivity;

import butterknife.BindView;
import butterknife.ButterKnife;

import static com.lxj.studentsign.entity.Result.RESULT_SUCCESS;
import static com.lxj.studentsign.entity.UserInfo.ROLE_STUDENT;
import static com.lxj.studentsign.entity.UserInfo.ROLE_TEACHER;

/**
 * 注册界面
 */
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
    private Api mApi;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        //初始化
        setContentView(R.layout.activity_register);
        ButterKnife.bind(this);
        setTitle("注册");

        mRgRole.check(R.id.rb_student);

        mApi = Api.getInstance(this.getApplicationContext());
    }

    public void cancel(View view) {
        finish();
    }

    /**
     * 请求服务器 注册账号
     */
    public void register(View view) {
        String userName = mEtName.getText().toString().trim();
        String userId = mEtNo.getText().toString().trim();
        String role = mRgRole.getCheckedRadioButtonId() == R.id.rb_teacher ? ROLE_TEACHER : ROLE_STUDENT;
        String pwd = mEtPwd.getText().toString().trim();

        //check
        if (TextUtils.isEmpty(userId) ||
                TextUtils.isEmpty(userName) ||
                TextUtils.isEmpty(pwd)) {
            Toast.makeText(this, "请填写完整数据", Toast.LENGTH_SHORT).show();
            return;
        }

        UserInfo userInfo = new UserInfo(userName, userId, role, pwd);
        showProgress();
        mApi.register(userInfo, new Api.Callback<Result>() {
            @Override
            public void OnSuccess(Result data) {
                closeProgress();
                if (RESULT_SUCCESS.equals(data.getResult())) {
                    Toast.makeText(RegisterActivity.this, "注册成功!", Toast.LENGTH_SHORT).show();
                    finish();
                } else {
                    Toast.makeText(RegisterActivity.this, data.getMsg(), Toast.LENGTH_SHORT).show();
                }
            }

            @Override
            public void OnFailed(Exception e) {
                closeProgress();
                e.printStackTrace();
                Toast.makeText(RegisterActivity.this, "网络错误!", Toast.LENGTH_SHORT).show();
            }
        });
    }
}
