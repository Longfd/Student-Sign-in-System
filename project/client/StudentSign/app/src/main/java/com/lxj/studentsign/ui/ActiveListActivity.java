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
import com.lxj.studentsign.entity.ActiveInfo;
import com.lxj.studentsign.entity.Result;
import com.lxj.studentsign.net.Api;
import com.lxj.studentsign.ui.base.BaseActivity;

import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;

import static com.lxj.studentsign.ui.SignListActivity.ACTIVE_ID;

/**
 * 活动列表界面
 */
public class ActiveListActivity extends BaseActivity {

    @BindView(R.id.rv_active)
    RecyclerView mRvActive;
    @BindView(R.id.srl_refresh)
    SwipeRefreshLayout mSrlRefresh;
    private SimpleRvAdapter<ActiveInfo> mAdapter;
    private Api mApi;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_active_list);
        ButterKnife.bind(this);

        setTitle("活动列表");
        mApi = Api.getInstance(this);

        mRvActive.setLayoutManager(new LinearLayoutManager(this));
        mRvActive.addItemDecoration(new DividerItemDecoration(this, DividerItemDecoration.VERTICAL));

        mAdapter = new SimpleRvAdapter<ActiveInfo>(this) {
            @Override
            protected void getItemClick(ActiveInfo item, int adapterPosition) {
                Intent intent = new Intent(ActiveListActivity.this, SignListActivity.class);
                intent.putExtra(ACTIVE_ID, item.getId());
                startActivity(intent);

            }

            @Override
            protected boolean getLongClick(ActiveInfo activeInfo, int adapterPosition) {
                showQrCode(activeInfo.getId());
                return true;
            }

            @Override
            public String getItemName(ActiveInfo item) {
                return item.getName();
            }
        };
        mRvActive.setAdapter(mAdapter);

        //监听下拉刷新
        mSrlRefresh.setOnRefreshListener(new SwipeRefreshLayout.OnRefreshListener() {
            @Override
            public void onRefresh() {
                getActiveList();
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
                getActiveList();
            }
        }, 100);
    }

    /**
     * 获取活动列表
     */
    private void getActiveList() {
        mApi.getActiveList(new Api.Callback<Result>() {
            @Override
            public void OnSuccess(Result data) {
                mSrlRefresh.setRefreshing(false);
                if (Result.RESULT_SUCCESS.equals(data.getResult())) {
                    List<ActiveInfo> activityInfo = data.getActivityInfo();
                    if (activityInfo != null) {
                        mAdapter.setData(activityInfo);
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
     * 跳转到 新建活动界面
     */
    public void createActivity(View view) {
        Intent intent = new Intent(this, CreateActiveActivity.class);
        startActivityForResult(intent, REQUEST_CODE);
    }

    @Override
    protected void refresh() {
        getData();
    }
}
