package com.xykj.studentsign.net;

import android.content.Context;
import android.util.Log;

import com.google.gson.Gson;
import com.google.gson.JsonSyntaxException;
import com.xykj.studentsign.App;
import com.xykj.studentsign.entity.Result;
import com.xykj.studentsign.entity.UserInfo;

import java.util.HashMap;
import java.util.Map;

public class Api {
    private static final String TAG = "Api";
    private static Api sInstance;
    private static String ip = "47.92.75.34";
    private static int port = 7000;
    private static int TIME_OUT = 10000;
    private final SocketManager mSocketManager;

    private static final int REQUEST_TYPE_REGISTER = 1000;//用户注册
    private static final int REQUEST_TYPE_LOGIN = 1001;//用户登录
    private static final int REQUEST_TYPE_ADD_CLASS = 1002;//创建班级
    private static final int REQUEST_TYPE_QUERY_CLASS = 1003; //查询班级
    private static final int REQUEST_TYPE_JOIN_CLASS = 1004;//加入班级
    private static final int REQUEST_TYPE_ADD_ACTIVITY = 1005;//创建活动
    private static final int REQUEST_TYPE_SIGN_ACTIVITY = 1006;//活动签到
    private static final int REQUEST_TYPE_QUERY_SIGN = 1007;//查询签到

    private static final int REQUEST_TYPE_QUERY_ACTIVITY = 1008;//查询活动
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
     * 1.注册
     *
     * @param user     对象实例
     * @param callback 结果回调
     */
    public void register(UserInfo user, Callback<Result> callback) {

        String data = mGson.toJson(user);
        Log.d(TAG, "register: " + data);
        mSocketManager.sendMsg(REQUEST_TYPE_REGISTER, data, new ResultCallback<>(callback, Result.class));
    }

    /**
     * 2.登录
     *
     * @param userId   userId
     * @param pwd      pwd
     * @param callback callback
     */
    public void login(String userId, String pwd, Callback<UserInfo> callback) {
        Map<String, String> map = new HashMap<>();
        map.put("userId", userId);
        map.put("pwd", pwd);

        String data = mGson.toJson(map);
        Log.d(TAG, "login: " + data);
        mSocketManager.sendMsg(REQUEST_TYPE_LOGIN, data, new ResultCallback<>(callback, UserInfo.class));
    }

    /**
     * 创建班级
     *
     * @param className 班级名
     */
    public void createClass(String className, Callback<Result> callback) {
        Map<String, String> map = new HashMap<>();
        map.put("userId", App.userId);
        map.put("cls_name", className);

        String data = mGson.toJson(map);
        Log.d(TAG, "createClass: " + data);

        mSocketManager.sendMsg(REQUEST_TYPE_ADD_CLASS, data, new ResultCallback<>(callback, Result.class));
    }

    /**
     * 班级列表
     */
    public void getClassList(Callback<Result> callback) {
        Map<String, String> map = new HashMap<>();
        map.put("userId", App.userId);
        String data = mGson.toJson(map);
        Log.d(TAG, "getClassList: " + data);
        mSocketManager.sendMsg(REQUEST_TYPE_QUERY_CLASS, data, new ResultCallback<>(callback, Result.class));
    }

    public void createActive(String activityName, String cls, Callback<Result> callback) {
        Map<String, String> map = new HashMap<>();
        map.put("userId", App.userId);
        map.put("activityName", activityName);
        map.put("classList", cls);
        String data = mGson.toJson(map);
        Log.d(TAG, "createActive: " + data);
        mSocketManager.sendMsg(REQUEST_TYPE_ADD_ACTIVITY, data, new ResultCallback<>(callback, Result.class));
    }

    public void getActiveList(Callback<Result> callback) {
        Map<String, String> map = new HashMap<>();
        map.put("userId", App.userId);
        String data = mGson.toJson(map);
        Log.d(TAG, "getClassList: " + data);
        mSocketManager.sendMsg(REQUEST_TYPE_QUERY_ACTIVITY, data, new ResultCallback<>(callback, Result.class));
    }

    public void getSignList(String activeId, Callback<Result> callback) {
        Map<String, String> map = new HashMap<>();
        map.put("userId", App.userId);
        map.put("activeId", activeId);
        String data = mGson.toJson(map);
        Log.d(TAG, "getClassList: " + data);
        mSocketManager.sendMsg(REQUEST_TYPE_QUERY_SIGN, data, new ResultCallback<>(callback, Result.class));
    }

    public void getJoinClass(String classId, Callback<Result> callback) {
        Map<String, String> map = new HashMap<>();
        map.put("userId", App.userId);
        map.put("classId", classId);
        String data = mGson.toJson(map);
        Log.d(TAG, "getClassList: " + data);
        mSocketManager.sendMsg(REQUEST_TYPE_JOIN_CLASS, data, new ResultCallback<>(callback, Result.class));
    }


    public void getSignActive(String activeId, Callback<Result> callback) {
        Map<String, String> map = new HashMap<>();
        map.put("userId", App.userId);
        map.put("activeId", activeId);
        String data = mGson.toJson(map);
        Log.d(TAG, "getClassList: " + data);
        mSocketManager.sendMsg(REQUEST_TYPE_SIGN_ACTIVITY, data, new ResultCallback<>(callback, Result.class));
    }

    class ResultCallback<T> implements SocketManager.SocketCallback {

        private final Callback<T> mCallback;
        private final Class<T> mClazz;

        ResultCallback(Callback<T> callback, Class<T> clazz) {
            mCallback = callback;
            mClazz = clazz;
        }

        @Override
        public void OnSuccess(String content) {
            T t = null;
            try {
                t = mGson.fromJson(content, mClazz);
            } catch (JsonSyntaxException e) {
                mCallback.OnFailed(e);
            }

            if (t == null) {
                mCallback.OnFailed(new Exception("data is null"));
                return;
            }
            mCallback.OnSuccess(t);
        }

        @Override
        public void OnFailed(Exception e) {
            mCallback.OnFailed(e);
        }
    }

    public interface Callback<T> {

        void OnSuccess(T data);

        void OnFailed(Exception e);
    }
}
