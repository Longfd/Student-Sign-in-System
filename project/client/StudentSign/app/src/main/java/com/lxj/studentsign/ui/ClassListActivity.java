package com.lxj.studentsign.ui;

import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.support.annotation.Nullable;
import android.support.v4.widget.SwipeRefreshLayout;
import android.support.v7.widget.DividerItemDecoration;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.view.View;

import com.lxj.studentsign.R;
import com.lxj.studentsign.adapter.SimpleRvAdapter;
import com.lxj.studentsign.entity.ClassInfo;
import com.lxj.studentsign.entity.Result;
import com.lxj.studentsign.net.Api;
import com.lxj.studentsign.ui.base.BaseActivity;
import com.lxj.studentsign.weiget.CreateClassDialog;

import java.util.ArrayList;
import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;

import static com.lxj.studentsign.ui.StudentListActivity.DATA_CLASS_INFO;

/**
 * 班级列表界面
 */
public class ClassListActivity extends BaseActivity {


    @BindView(R.id.rv_class_list)
    RecyclerView mRvClassList;

    List<ClassInfo> mClassInfos;
    @BindView(R.id.srl_refresh)
    SwipeRefreshLayout mSrlRefresh;
    private Api mApi;
    private SimpleRvAdapter<ClassInfo> mAdapter;
    private CreateClassDialog mCreateClassDialog;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_class);
        ButterKnife.bind(this);

        setTitle("班级列表");
        //初始化
        mApi = Api.getInstance(this);
        mRvClassList.setLayoutManager(new LinearLayoutManager(this));
        mRvClassList.addItemDecoration(new DividerItemDecoration(this, DividerItemDecoration.VERTICAL));
        mClassInfos = new ArrayList<>();
        mAdapter = new SimpleRvAdapter<ClassInfo>(this) {
            @Override
            protected void getItemClick(ClassInfo item, int adapterPosition) {
                Intent intent = new Intent(ClassListActivity.this, StudentListActivity.class);
                intent.putExtra(DATA_CLASS_INFO, item);
                startActivity(intent);
            }

            @Override
            protected boolean getLongClick(ClassInfo item, int adapterPosition) {
                showQrCode(item.getClassId());
                return true;
            }

            @Override
            public String getItemName(ClassInfo item) {
                return item.getClassName();
            }
        };
        mRvClassList.setAdapter(mAdapter);
        //监听刷新
        mSrlRefresh.setOnRefreshListener(new SwipeRefreshLayout.OnRefreshListener() {
            @Override
            public void onRefresh() {
                getClassList();
            }
        });
        getData();
    }

    /**
     * 获取数据
     */
    private void getData() {
        new Handler(getMainLooper()).postDelayed(new Runnable() {
            @Override
            public void run() {
                mSrlRefresh.setRefreshing(true);
                getClassList();
            }
        }, 100);
    }

    /**
     * 获取班级列表
     */
    private void getClassList() {
        mApi.getClassList(new Api.Callback<Result>() {
            @Override
            public void OnSuccess(Result data) {
                mSrlRefresh.setRefreshing(false);
                if (Result.RESULT_SUCCESS.equals(data.getResult())) {
                    List<ClassInfo> classInfo = data.getClassInfo();

                    if (classInfo != null) {

                        mClassInfos.clear();
                        mClassInfos.addAll(classInfo);
                        mAdapter.setData(mClassInfos);
                    } else {
                        showToast("暂无数据!");
                    }
                } else {
                    showToast(data.getMsg());
                }
            }

            @Override
            public void OnFailed(Exception e) {
                mSrlRefresh.setRefreshing(false);
                showNetError();
            }
        });
    }

    /**
     * 创建班级
     */
    public void createClass(View view) {
        mCreateClassDialog = new CreateClassDialog(this);
        mCreateClassDialog.setInputListener(new CreateClassDialog.OnInputListener() {
            @Override
            public void onInput(String className) {
                createClass(className);
            }
        });
        mCreateClassDialog.show();
    }

    /**
     * 请求服务器 创建班级
     */
    private void createClass(String className) {
        showProgress();
        mApi.createClass(className, new Api.Callback<Result>() {
            @Override
            public void OnSuccess(Result data) {
                //请求成功
                closeProgress();
                if (Result.RESULT_SUCCESS.equals(data.getResult())) {
                    //创建成功
                    showToast("创建班级成功!");
                    if (mCreateClassDialog != null) {
                        mCreateClassDialog.dismiss();
                    }
                    //刷新列表
                    getData();
                } else {
                    showToast(data.getMsg());
                }
            }

            @Override
            public void OnFailed(Exception e) {
                //请求失败
                closeProgress();
                e.printStackTrace();
                showNetError();
            }
        });
    }

}
