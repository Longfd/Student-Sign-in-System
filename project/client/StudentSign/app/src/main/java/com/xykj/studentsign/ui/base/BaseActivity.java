package com.xykj.studentsign.ui.base;

import android.app.ProgressDialog;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v7.app.AlertDialog;
import android.support.v7.app.AppCompatActivity;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.Toast;

import com.uuzuche.lib_zxing.activity.CodeUtils;
import com.xykj.studentsign.R;

public abstract class BaseActivity extends AppCompatActivity {
    protected String TAG = getClass().getSimpleName();
    public static final String RESULT_REFRESH = "refresh";
    public static final int REQUEST_CODE = 1001;
    private Toast mToast;
    private ProgressDialog mProgressDialog;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        // 竖屏
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
    }

    protected void showProgress() {
        if (mProgressDialog == null) {
            mProgressDialog = new ProgressDialog(this);
            mProgressDialog.setMessage(getResources().getString(R.string.network_request));
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

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
//        super.onActivityResult(requestCode, resultCode, data);

        if (requestCode == REQUEST_CODE && data != null) {
            boolean refresh = data.getBooleanExtra(RESULT_REFRESH, false);
            if (refresh) {
                refresh();
            }
        }
    }

    protected void refresh() {

    }

    protected void showQrCode(String content) {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        ImageView imageView = new ImageView(this);
        ViewGroup.LayoutParams params = new ViewGroup.LayoutParams(500, 500);
        imageView.setLayoutParams(params);
        Bitmap bitmap = CodeUtils.createImage(content, 400, 400,
                BitmapFactory.decodeResource(getResources(), 0));
        imageView.setImageBitmap(bitmap);
        builder.setView(imageView).show();
    }
}