package com.xykj.studentsign.entity;

public class UserInfo {

    public static final String ROLE_STUDENT = "0";//角色 0 学生
    public static final String ROLE_TEACHER = "1";//角色 1 教师

    private String userName;//学号,
    private String userId;//姓名,
    private String role;//角色 0-学生, 1-教师
    private String pwd;//密码

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
}
