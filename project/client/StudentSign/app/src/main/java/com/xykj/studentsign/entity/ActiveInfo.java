package com.xykj.studentsign.entity;

import com.google.gson.annotations.SerializedName;

/**
 * 活动实体类
 */
public class ActiveInfo {
    @SerializedName("act_name")
    private String name;//活动名
    @SerializedName("act_no")
    private String id;//活动Id

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
