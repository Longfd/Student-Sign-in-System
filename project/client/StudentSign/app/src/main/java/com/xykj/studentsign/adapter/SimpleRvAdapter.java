package com.xykj.studentsign.adapter;

import android.content.Context;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import com.xykj.studentsign.R;

import java.util.ArrayList;
import java.util.List;

public abstract class SimpleRvAdapter<T> extends RecyclerView.Adapter<RecyclerView.ViewHolder> {

    private final Context mContext;
    private List<T> mData;

    public SimpleRvAdapter(Context context) {
        mContext = context;
        mData = new ArrayList<>();
    }

    public SimpleRvAdapter setData(List<T> ts) {
        mData.clear();
        if (ts != null && ts.size() > 0) {
            mData.addAll(ts);
        }
        notifyDataSetChanged();
        return this;
    }

    @Override
    public RecyclerView.ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        View view = LayoutInflater.from(mContext).inflate(R.layout.item_class, parent, false);
        return new SimpleViewHolder(view);
    }

    @Override
    public void onBindViewHolder(final RecyclerView.ViewHolder holder, int position) {
        final T t = mData.get(position);
        bindItem((SimpleViewHolder) holder, t);
        holder.itemView.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                getItemClick(t, holder.getAdapterPosition());
            }
        });
    }

    protected void bindItem(SimpleViewHolder holder, T t) {

        holder.name.setText(getItemName(t));
    }

    protected abstract void getItemClick(T item, int adapterPosition);

    public abstract String getItemName(T item);


    @Override
    public int getItemCount() {
        return mData.size();
    }

    public class SimpleViewHolder extends RecyclerView.ViewHolder {

        public TextView name;

        public SimpleViewHolder(View itemView) {
            super(itemView);

            name = itemView.findViewById(R.id.tv_name);
        }
    }
}
