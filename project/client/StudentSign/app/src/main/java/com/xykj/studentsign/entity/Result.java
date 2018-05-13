package com.xykj.studentsign.entity;

import java.util.List;

/**
 * 请求实体类
 */
public class Result {
    public static final String RESULT_SUCCESS = "0";//请求成功
    public static final String RESULT_FAILED = "1";//请求失败

    private String result;//请求结果
    private String msg;//请求内容

    private List<ClassInfo> classInfo;//班级列表
    private List<ActiveInfo> activityInfo;//活动列表
    private List<UserInfo> signInfo;//学生列表

    public String getResult() {
        return result;
    }

    public void setResult(String result) {
        this.result = result;
    }

    public String getMsg() {
        return msg;
    }

    public void setMsg(String msg) {
        this.msg = msg;
    }

    public List<ClassInfo> getClassInfo() {
        return classInfo;
    }

    public void setClassInfo(List<ClassInfo> classInfo) {
        this.classInfo = classInfo;
    }

    public List<ActiveInfo> getActivityInfo() {
        return activityInfo;
    }

    public void setActivityInfo(List<ActiveInfo> activityInfo) {
        this.activityInfo = activityInfo;
    }

    public List<UserInfo> getSignInfo() {
        return signInfo;
    }

    public void setSignInfo(List<UserInfo> signInfo) {
        this.signInfo = signInfo;
    }
}
