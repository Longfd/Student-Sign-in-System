package com.xykj.studentsign;

import android.app.ProgressDialog;
import android.support.v7.app.AppCompatActivity;
import android.widget.Toast;

public abstract class BaseActivity extends AppCompatActivity {


    private Toast mToast;
    private ProgressDialog mProgressDialog;


    protected void showProgress() {
        if (mProgressDialog == null) {
            mProgressDialog = new ProgressDialog(this);
            mProgressDialog.setTitle(R.string.network_request);
        }
        mProgressDialog.show();
    }

    protected void closeProgress() {
        if (mProgressDialog != null && mProgressDialog.isShowing()) {
            mProgressDialog.dismiss();
        }
    }


    protected void showNetError() {
        showToast(R.string.network_error);
    }

    private void showToast(int resId) {
        showToast(getResources().getString(resId));
    }

    protected void showToast(String content) {
        if (mToast == null) {
            mToast = Toast.makeText(this, "", Toast.LENGTH_SHORT);
        }
        mToast.setText(content);
        mToast.show();
    }
}
