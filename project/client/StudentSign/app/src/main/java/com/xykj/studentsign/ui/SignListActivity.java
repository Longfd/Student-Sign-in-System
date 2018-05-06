package com.xykj.studentsign.ui;

import android.content.Context;
import android.os.Bundle;
import android.os.Handler;
import android.support.annotation.Nullable;
import android.support.v4.widget.SwipeRefreshLayout;
import android.support.v7.widget.DividerItemDecoration;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;

import com.xykj.studentsign.R;
import com.xykj.studentsign.adapter.SimpleRvAdapter;
import com.xykj.studentsign.entity.Result;
import com.xykj.studentsign.entity.UserInfo;
import com.xykj.studentsign.net.Api;
import com.xykj.studentsign.ui.base.BaseActivity;

import java.util.List;

import butterknife.BindColor;
import butterknife.BindView;
import butterknife.ButterKnife;

public class SignListActivity extends BaseActivity {

    public static final String ACTIVE_ID = "activeId";
    @BindView(R.id.srl_refresh)
    SwipeRefreshLayout mSrlRefresh;
    @BindView(R.id.rv_sign)
    RecyclerView mRvSign;
    @BindColor(R.color.txt_gray)
    int txtGray;
    @BindColor(R.color.txt_red)
    int txtRed;


    private SimpleRvAdapter<UserInfo> mAdapter;
    private Api mApi;
    private String mActiveId;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_sign_list);
        ButterKnife.bind(this);

        mActiveId = getIntent().getStringExtra(ACTIVE_ID);

        mApi = Api.getInstance(this);
        mAdapter = new SimpleRvAdapter<UserInfo>(this) {
            @Override
            protected void getItemClick(UserInfo item, int adapterPosition) {

            }

            @Override
            public String getItemName(UserInfo item) {
                return item.getUserName();
            }

            @Override
            protected void bindItem(SimpleViewHolder holder, UserInfo userInfo) {
                super.bindItem(holder, userInfo);
                int color;
                if ("1".equals(userInfo.getSignState())) {
                    color = txtGray;
                } else {
                    color = txtRed;
                }
                holder.name.setTextColor(color);
            }
        };
        mRvSign.setLayoutManager(new LinearLayoutManager(this));
        mRvSign.addItemDecoration(new DividerItemDecoration(this, DividerItemDecoration.VERTICAL));

        mRvSign.setAdapter(mAdapter);

        mSrlRefresh.setOnRefreshListener(new SwipeRefreshLayout.OnRefreshListener() {
            @Override
            public void onRefresh() {
                getSignList();
            }
        });
        getData();
    }

    private void getData() {
        new Handler(getMainLooper()).postDelayed(new Runnable() {
            @Override
            public void run() {
                mSrlRefresh.setRefreshing(true);
                getSignList();
            }
        }, 100);
    }

    private void getSignList() {
        mApi.getSignList(mActiveId, new Api.Callback<Result>() {
            @Override
            public void OnSuccess(Result data) {
                mSrlRefresh.setRefreshing(false);
                if (Result.RESULT_SUCCESS.equals(data.getResult())) {
                    List<UserInfo> activityInfo = data.getSignInfo();
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


}
