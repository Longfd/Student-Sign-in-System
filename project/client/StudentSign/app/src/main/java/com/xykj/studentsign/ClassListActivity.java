package com.xykj.studentsign;

import android.content.Intent;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v7.widget.DividerItemDecoration;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import com.xykj.studentsign.entity.ClassInfo;

import java.util.ArrayList;
import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;

import static com.xykj.studentsign.StudentListActivity.DATA_CLASS_INFO;

public class ClassListActivity extends BaseActivity {


    @BindView(R.id.rv_class_list)
    RecyclerView mRvClassList;

    List<ClassInfo> mClassInfos;
    private ClassAdapter mAdapter;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_class);
        ButterKnife.bind(this);

        mRvClassList.setLayoutManager(new LinearLayoutManager(this));
        mRvClassList.addItemDecoration(new DividerItemDecoration(this, DividerItemDecoration.VERTICAL));
        mClassInfos = new ArrayList<>();
        mAdapter = new ClassAdapter();
        mRvClassList.setAdapter(mAdapter);

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
