package com.xykj.studentsign.ui;

import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v7.widget.DividerItemDecoration;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;

import com.xykj.studentsign.R;
import com.xykj.studentsign.adapter.SimpleRvAdapter;
import com.xykj.studentsign.entity.ClassInfo;
import com.xykj.studentsign.entity.UserInfo;
import com.xykj.studentsign.ui.base.BaseActivity;

import butterknife.BindView;
import butterknife.ButterKnife;

public class StudentListActivity extends BaseActivity {
    public static final String DATA_CLASS_INFO = "classInfo";
    @BindView(R.id.rv_student)
    RecyclerView mRvStudent;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_student_list);
        ButterKnife.bind(this);

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
