package com.xykj.studentsign.weiget;

import android.app.Dialog;
import android.content.Context;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.text.TextUtils;
import android.view.View;
import android.widget.EditText;
import android.widget.Toast;

import com.xykj.studentsign.R;

public class CreateClassDialog extends Dialog {

    private OnInputListener mListener;

    public CreateClassDialog(@NonNull Context context) {
        super(context);
    }

    public CreateClassDialog(@NonNull Context context, int themeResId) {
        super(context, themeResId);
    }

    protected CreateClassDialog(@NonNull Context context, boolean cancelable, @Nullable OnCancelListener cancelListener) {
        super(context, cancelable, cancelListener);
    }


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.dialog_create_class);


        findViewById(R.id.btn_create).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                createClass();
            }
        });
    }

    private void createClass() {
        String className = ((EditText) findViewById(R.id.et_class_name)).getText().toString().trim();
        if (TextUtils.isEmpty(className)) {
            Toast.makeText(getContext(), "班级名不能为空", Toast.LENGTH_SHORT).show();
            return;
        }

        if (mListener != null) {
            mListener.onInput(className);
        }
    }

    public void setInputListener(OnInputListener listener) {
        mListener = listener;
    }

    public interface OnInputListener {
        void onInput(String className);
    }
}
