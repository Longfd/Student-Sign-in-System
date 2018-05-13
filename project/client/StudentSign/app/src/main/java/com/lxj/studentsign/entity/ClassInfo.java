package com.lxj.studentsign.entity;

import com.google.gson.annotations.SerializedName;

import java.io.Serializable;
import java.util.List;
import java.util.Objects;

/**
 * 班级实体类
 */
public class ClassInfo implements Serializable {
    @SerializedName("cls_no")
    private String classId;// 所在班级
    @SerializedName("cls_name")
    private String className;//班级名称
    private String id;//班级创建人

    private List<UserInfo> students;

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

    public List<UserInfo> getStudents() {
        return students;
    }

    public void setStudents(List<UserInfo> students) {
        this.students = students;
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        ClassInfo classInfo = (ClassInfo) o;
        return Objects.equals(classId, classInfo.classId);
    }

    @Override
    public int hashCode() {

        return Objects.hash(classId);
    }
}
