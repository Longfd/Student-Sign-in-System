package com.xykj.studentsign.ui;

import android.content.Intent;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v7.widget.DividerItemDecoration;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.text.TextUtils;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.EditText;

import com.xykj.studentsign.R;
import com.xykj.studentsign.entity.ClassInfo;
import com.xykj.studentsign.entity.Result;
import com.xykj.studentsign.net.Api;
import com.xykj.studentsign.ui.base.BaseActivity;

import java.util.ArrayList;
import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;

public class CreateActiveActivity extends BaseActivity {


    @BindView(R.id.et_name)
    EditText mEtName;
    @BindView(R.id.rv_class_list)
    RecyclerView mRvClassList;

    List<ClassInfo> mClassList;
    List<ClassInfo> mCheckedList;
    private Api mApi;
    private ClassAdapter mAdapter;


    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_create_active);
        ButterKnife.bind(this);
        setTitle("创建活动");
        mClassList = new ArrayList<>();
        mCheckedList = new ArrayList<>();
        mApi = Api.getInstance(this);

        mRvClassList.setLayoutManager(new LinearLayoutManager(this));
        mRvClassList.addItemDecoration(new DividerItemDecoration(this, DividerItemDecoration.VERTICAL));
        mAdapter = new ClassAdapter();
        mRvClassList.setAdapter(mAdapter);

        getClassList();
    }

    private void getClassList() {
        showProgress();
        mApi.getClassList(new Api.Callback<Result>() {
            @Override
            public void OnSuccess(Result data) {
                closeProgress();
                if (Result.RESULT_SUCCESS.equals(data.getResult())) {
                    List<ClassInfo> classInfo = data.getClassInfo();
                    if (classInfo != null) {
                        mClassList.clear();
                        mClassList.addAll(classInfo);
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
                closeProgress();
                showNetError();
            }
        });
    }

    public void createActive(View view) {
        String name = mEtName.getText().toString().trim();
        if (TextUtils.isEmpty(name)) {
            showToast("活动名不能为空");
            return;
        }
//        StringBuilder builder = new StringBuilder();
//        if (mCheckedList.size() > 0) {
//            for (ClassInfo classInfo : mCheckedList) {
//                builder.append(classInfo.getClassId())
//                        .append(",");
//            }
//        }
//        String cls = builder.toString().substring(0, builder.length() - 1);

        showProgress();
        mApi.createActive(name, mCheckedList, new Api.Callback<Result>() {
            @Override
            public void OnSuccess(Result data) {
                closeProgress();
                if (Result.RESULT_SUCCESS.equals(data.getResult())) {
                    showToast("创建成功!");
                    Intent intent = new Intent();
                    intent.putExtra(RESULT_REFRESH, true);
                    setResult(1, intent);
                    finish();
                } else {
                    showToast(data.getMsg());
                }
            }

            @Override
            public void OnFailed(Exception e) {
                closeProgress();
                showNetError();
            }
        });

    }

    class ClassAdapter extends RecyclerView.Adapter<ViewHolder> {
        @Override
        public ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
            View view = getLayoutInflater().inflate(R.layout.item_active_check, parent, false);
            return new ViewHolder(view);
        }

        @Override
        public void onBindViewHolder(final ViewHolder holder, int position) {
            final ClassInfo clazz = mClassList.get(position);
            holder.mCbClass.setText(clazz.getClassName());
            holder.mCbClass.setOnCheckedChangeListener(null);
            holder.mCbClass.setChecked(mCheckedList.contains(clazz));
            holder.mCbClass.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
                @Override
                public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                    if (isChecked) {
                        if (!mCheckedList.contains(clazz)) {
                            mCheckedList.add(clazz);
                        }
                    } else {
                        mCheckedList.remove(clazz);
                    }
                }
            });
        }

        @Override
        public int getItemCount() {
            return mClassList.size();
        }
    }


    class ViewHolder extends RecyclerView.ViewHolder {
        CheckBox mCbClass;

        public ViewHolder(View itemView) {
            super(itemView);
            mCbClass = itemView.findViewById(R.id.cb_class);
        }
    }
}
