package com.lxj.studentsign.ui;

import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v7.widget.DividerItemDecoration;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;

import com.lxj.studentsign.R;
import com.lxj.studentsign.adapter.SimpleRvAdapter;
import com.lxj.studentsign.entity.ClassInfo;
import com.lxj.studentsign.entity.Result;
import com.lxj.studentsign.entity.UserInfo;
import com.lxj.studentsign.net.Api;
import com.lxj.studentsign.ui.base.BaseActivity;

import butterknife.BindView;
import butterknife.ButterKnife;

/**
 * 学生列表界面
 */
public class StudentListActivity extends BaseActivity {
    public static final String DATA_CLASS_INFO = "classInfo";
    @BindView(R.id.rv_student)
    RecyclerView mRvStudent;
    private Api mApi;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        //初始化
        setContentView(R.layout.activity_student_list);
        ButterKnife.bind(this);
        setTitle("学生列表");
        ClassInfo classInfo = (ClassInfo) getIntent().getSerializableExtra(DATA_CLASS_INFO);

        mRvStudent.setLayoutManager(new LinearLayoutManager(this));
        mRvStudent.addItemDecoration(new DividerItemDecoration(this, DividerItemDecoration.VERTICAL));

        mRvStudent.setAdapter(new SimpleRvAdapter<UserInfo>(this) {
            @Override
            protected void getItemClick(UserInfo item, int adapterPosition) {

            }

            @Override
            public String getItemName(UserInfo item) {
                return item.getUserName();
            }
        }.setData(classInfo.getStudents()));

    }


}
