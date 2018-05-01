package com.xykj.studentsign.entity;

public class Result {
    public static final String RESULT_SUCCESS = "1";
    public static final String RESULT_FAILED = "0";

    private String result;
    private String msg;

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
}
