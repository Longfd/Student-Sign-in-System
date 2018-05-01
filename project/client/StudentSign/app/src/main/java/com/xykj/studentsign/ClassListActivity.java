package com.xykj.studentsign;

import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.support.annotation.Nullable;
import android.support.v4.widget.SwipeRefreshLayout;
import android.support.v7.widget.DividerItemDecoration;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import com.xykj.studentsign.entity.ClassInfo;
import com.xykj.studentsign.entity.Result;
import com.xykj.studentsign.net.Api;
import com.xykj.studentsign.weiget.CreateClassDialog;

import java.util.ArrayList;
import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;

import static com.xykj.studentsign.StudentListActivity.DATA_CLASS_INFO;

public class ClassListActivity extends BaseActivity {


    @BindView(R.id.rv_class_list)
    RecyclerView mRvClassList;

    List<ClassInfo> mClassInfos;
    @BindView(R.id.srl_refresh)
    SwipeRefreshLayout mSrlRefresh;
    private ClassAdapter mAdapter;
    private Api mApi;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_class);
        ButterKnife.bind(this);

        mApi = Api.getInstance(this);
        mRvClassList.setLayoutManager(new LinearLayoutManager(this));
        mRvClassList.addItemDecoration(new DividerItemDecoration(this, DividerItemDecoration.VERTICAL));
        mClassInfos = new ArrayList<>();
        mAdapter = new ClassAdapter();
        mRvClassList.setAdapter(mAdapter);

        mSrlRefresh.setOnRefreshListener(new SwipeRefreshLayout.OnRefreshListener() {
            @Override
            public void onRefresh() {
                getClassList();
            }
        });
        getData();
    }

    private void getData() {
        new Handler(getMainLooper()).postDelayed(new Runnable() {
            @Override
            public void run() {
                mSrlRefresh.setRefreshing(true);
                getClassList();
            }
        }, 100);
    }

    private void getClassList() {
        mApi.getClassList(new Api.Callback<Result>() {
            @Override
            public void OnSuccess(Result data) {
                closeProgress();
                if (Result.RESULT_SUCCESS.equals(data.getResult())) {
                    List<ClassInfo> classInfo = data.getClassInfo();

                    if (classInfo != null) {

                        mClassInfos.clear();
                        mClassInfos.addAll(classInfo);
                        mAdapter.notifyDataSetChanged();
                    } else {
                        showToast("暂无数据!");
                    }
                } else {
                    showToast(data.getMsg());
                }
            }

            @Override
            public void OnFailed(Exception e) {
                showNetError();
            }
        });
    }

    public void createClass(View view) {
        CreateClassDialog createClassDialog = new CreateClassDialog(this);
        createClassDialog.setInputListener(new CreateClassDialog.OnInputListener() {
            @Override
            public void onInput(String className) {
                createClass(className);
            }
        });
        createClassDialog.show();
    }

    private void createClass(String className) {
        showProgress();
        mApi.createClass(className, new Api.Callback<Result>() {
            @Override
            public void OnSuccess(Result data) {
                closeProgress();
                if (Result.RESULT_SUCCESS.equals(data.getResult())) {
                    //创建成功
                    showToast("创建班级成功!");
                    //刷新列表
                    getData();
                } else {
                    showToast(data.getMsg());
                }
            }

            @Override
            public void OnFailed(Exception e) {
                closeProgress();
                e.printStackTrace();
                showNetError();
            }
        });
    }

    class ClassAdapter extends RecyclerView.Adapter<ViewHolder> {

        @Override
        public ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
            View view = getLayoutInflater().inflate(R.layout.item_class, parent, false);
            return new ViewHolder(view);
        }

        @Override
        public void onBindViewHolder(ViewHolder holder, int position) {
            ClassInfo classInfo = mClassInfos.get(position);
            holder.name.setText(classInfo.getClassName());
            Intent intent = new Intent(ClassListActivity.this, StudentListActivity.class);
            intent.putExtra(DATA_CLASS_INFO, classInfo);
            startActivity(intent);
        }

        @Override
        public int getItemCount() {
            return mClassInfos.size();
        }
    }

    class ViewHolder extends RecyclerView.ViewHolder {

        TextView name;

        public ViewHolder(View itemView) {
            super(itemView);

            name = itemView.findViewById(R.id.tv_name);
        }
    }
}
