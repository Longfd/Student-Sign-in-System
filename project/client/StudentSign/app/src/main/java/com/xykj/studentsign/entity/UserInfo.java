package com.xykj.studentsign.entity;

import com.google.gson.annotations.SerializedName;

import java.io.Serializable;

public class UserInfo implements Serializable {

    public static final String ROLE_STUDENT = "0";//角色 0 学生
    public static final String ROLE_TEACHER = "1";//角色 1 教师

    private String userName;//学号,
    private String userId;//姓名,
    private String role;//角色 0-学生, 1-教师
    private String pwd;//密码

    private String result;//
    private String msg;
    @SerializedName("cls_no")
    private String classId;// 所在班级
    @SerializedName("cls_name")
    private String className;//班级名称
    private String id;//班级创建人

    public UserInfo(String userName, String userId, String role, String pwd) {
        this.userName = userName;
        this.userId = userId;
        this.role = role;
        this.pwd = pwd;
    }

    public String getUserName() {
        return userName;
    }

    public void setUserName(String userName) {
        this.userName = userName;
    }

    public String getUserId() {
        return userId;
    }

    public void setUserId(String userId) {
        this.userId = userId;
    }

    public String getRole() {
        return role;
    }

    public void setRole(String role) {
        this.role = role;
    }

    public String getPwd() {
        return pwd;
    }

    public void setPwd(String pwd) {
        this.pwd = pwd;
    }

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

    public String getClassId() {
        return classId;
    }

    public void setClassId(String classId) {
        this.classId = classId;
    }

    public String getClassName() {
        return className;
    }

    public void setClassName(String className) {
        this.className = className;
    }

    public String getId() {
        return id;
    }

    public void setId(String id) {
        this.id = id;
    }
}
