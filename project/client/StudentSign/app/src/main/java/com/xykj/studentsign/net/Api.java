package com.xykj.studentsign.net;

import android.content.Context;
import android.util.Log;

import com.google.gson.Gson;
import com.google.gson.JsonSyntaxException;
import com.xykj.studentsign.entity.Result;
import com.xykj.studentsign.entity.UserInfo;

import java.util.HashMap;
import java.util.Map;

public class Api {
    private static final String TAG = "Api";
    private static Api sInstance;
    private static String ip = "65.49.226.247";
    private static int port = 8000;
    private static int TIME_OUT = 10000;
    private final SocketManager mSocketManager;

    private static final int REQUEST_TYPE_REGISTER = 1000;//用户注册
    private static final int REQUEST_TYPE_LOGIN = 1001;//用户登录
    private static final int REQUEST_TYPE_ADD_CLASS = 1002;//创建班级
    private static final int REQUEST_TYPE_QUERY_CLASS = 1003; //查询班级
    private static final int REQUEST_TYPE_ADD_ACTIVITY = 1004;//创建活动
    private Gson mGson;


    private Api(Context context) {
        mSocketManager = new SocketManager.Builder(context)
                .setIp(ip)
                .setPort(port)
                .setTimeOut(TIME_OUT)
                .build();
        mGson = new Gson();
    }

    public synchronized static Api getInstance(Context context) {
        if (sInstance == null) {
            sInstance = new Api(context);
        }
        return sInstance;
    }

    /**
     * 1.登录
     *
     * @param user     对象实例
     * @param callback 结果回调
     */
    public void register(UserInfo user, final Callback<Result> callback) {

        String data = mGson.toJson(user);
        Log.d(TAG, "register: " + data);
        mSocketManager.sendMsg(REQUEST_TYPE_REGISTER, data, new SocketManager.SocketCallback() {
            @Override
            public void OnSuccess(String content) {
                Result result = null;
                try {
                    result = mGson.fromJson(content, Result.class);
                } catch (JsonSyntaxException e) {
                    callback.OnFailed(e);
                }
                callback.OnSuccess(result);
            }

            @Override
            public void OnFailed(Exception e) {
                callback.OnFailed(e);
            }
        });
    }


    public void login(String userId, String pwd, final Callback<UserInfo> callback) {
        Map<String, String> map = new HashMap<>();
        map.put("userId", userId);
        map.put("pwd", pwd);

        String data = mGson.toJson(map);
        Log.d(TAG, "login: " + data);
        mSocketManager.sendMsg(REQUEST_TYPE_LOGIN, data, new SocketManager.SocketCallback() {
            @Override
            public void OnSuccess(String content) {
                UserInfo userInfo = null;
                try {
                    userInfo = mGson.fromJson(content, UserInfo.class);
                } catch (JsonSyntaxException e) {
                    callback.OnFailed(e);
                }
                callback.OnSuccess(userInfo);
            }

            @Override
            public void OnFailed(Exception e) {
                callback.OnFailed(e);
            }
        });


    }

    public interface Callback<T> {

        void OnSuccess(T data);

        void OnFailed(Exception e);
    }
}
