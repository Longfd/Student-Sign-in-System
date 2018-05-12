package com.xykj.studentsign.entity;

import java.util.List;

public class Result {
    public static final String RESULT_SUCCESS = "0";
    public static final String RESULT_FAILED = "1";

    private String result;
    private String msg;

    private List<ClassInfo> classInfo;
    private List<ActiveInfo> activityInfo;
    private List<UserInfo> signInfo;

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
