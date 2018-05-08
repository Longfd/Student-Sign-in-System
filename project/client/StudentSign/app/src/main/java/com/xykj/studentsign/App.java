package com.xykj.studentsign;

import android.Manifest;
import android.app.Application;

import com.uuzuche.lib_zxing.activity.ZXingLibrary;
import com.xykj.studentsign.entity.UserInfo;

public class App extends Application {

    public static String userName;
    public static String userId;
    public static String role;
    public static UserInfo userInfo;

    public static String[] permissions = new String[]{
            Manifest.permission.SEND_SMS,
            Manifest.permission.CAMERA,
            Manifest.permission.READ_PHONE_STATE,
            Manifest.permission.READ_EXTERNAL_STORAGE
    };

    @Override
    public void onCreate() {
        super.onCreate();
        ZXingLibrary.initDisplayOpinion(this);
    }

}
