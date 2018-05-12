package com.xykj.studentsign.entity;

import com.google.gson.annotations.SerializedName;

public class ActiveInfo {
    @SerializedName("act_name")
    private String name;
    @SerializedName("act_no")
    private String id;

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public String getId() {
        return id;
    }

    public void setId(String id) {
        this.id = id;
    }
}
