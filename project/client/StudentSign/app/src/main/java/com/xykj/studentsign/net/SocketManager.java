package com.xykj.studentsign.net;

import android.content.Context;
import android.os.Handler;
import android.support.annotation.NonNull;
import android.util.Log;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.util.Arrays;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

/**
 * 通信协议
 * 报文格式: 包头 + 包体 + 包尾
 * 包头:
 * <p>
 * 报文类型(INT):   4 Byte(网络序or大端序) 详见报文接口设计
 * 报文长度(INT): 4 Byte(网络序or大端序) 包体长 + 2Byte(包尾校验字)
 * 包头校验(SHORT): 2 Byte(网络序or大端序) [报文类型+报文长度] 单个字节累加之和
 * 包体:
 * <p>
 * 请求包: 业务内容(JSON格式)//待定
 * 返回包: 业务内容(JSON格式)//待定
 * 包尾:
 * <p>
 * 包尾校验(SHORT): 2 Byte(网络序or大端序) [包体内容] 单个字节累加之和
 */
public class SocketManager {
    private static final String TAG = "SocketManager";

    private static final int HEAD_LENGTH = 10;
    private static final int HEAD_TYPE_LENGTH = 4;
    private static final int HEAD_LEN_LENGTH = 4;
    private static final int HEAD_CHECK_LENGTH = 2;
    private static final int END_CHECK = 2;

    private static final int DEFAULT_TIME_OUT = 10000;

    private static final String UTF_8 = "UTF-8";

    private int mTimeOut;
    private String mIp;
    private int mPort;

    private Handler mHandler;
    private ExecutorService mCachedThreadPool;

    public SocketManager(Context context, String ip, int port) {
        this(context, ip, port, DEFAULT_TIME_OUT);
    }

    public SocketManager(Context context, String ip, int port, int timeOut) {
        this.mIp = ip;
        this.mPort = port;
        this.mTimeOut = timeOut;
        mHandler = new Handler(context.getMainLooper());

        mCachedThreadPool = Executors.newCachedThreadPool();
    }

    public static class Builder {
        Context context;
        String ip;
        int port;
        int timeOut;

        public Builder(Context context) {
            this.context = context;
        }

        public Builder setIp(String ip) {
            this.ip = ip;
            return this;
        }

        public Builder setPort(int port) {
            this.port = port;
            return this;
        }

        public Builder setTimeOut(int timeOut) {
            this.timeOut = timeOut;
            return this;
        }

        public SocketManager build() {
            return new SocketManager(context, ip, port, timeOut);
        }

    }

    private Socket connect(String ip, int port) throws IOException {

        Socket socket = new Socket(ip, port);
        socket.setSoTimeout(mTimeOut);
        return socket;

    }


    public void sendMsg(final int type, final String data, final SocketCallback callback) {
        mCachedThreadPool.execute(new Runnable() {
            @Override
            public void run() {
                sendMsgSync(type, data, callback);
            }
        });
    }

    private synchronized void sendMsgSync(int type, String data, SocketCallback callback) {
        //1.建立socket
        Socket socket;
        try {
            socket = connect(mIp, mPort);
        } catch (IOException e) {
            handleFailed(callback, e);
            return;
        }

        //2.发送请求
        try {
            OutputStream os = socket.getOutputStream();
            //咸鱼科技 socket 规范
            byte[] bytes = getBytesByXianYu(type, data);
//            printByte("all", bytes);
            os.write(bytes);
            Log.d(TAG, "send: " + type + "-" + data);
        } catch (Exception e) {
            handleFailed(callback, e);
            return;
        }

        //3.接收相应数据
        try {
            InputStream is = socket.getInputStream();
            byte[] head = new byte[HEAD_LENGTH];
            for (; ; ) {
                int read = is.read(head);
                if (read == HEAD_LENGTH) {
                    break;
                }
            }
            byte[] headType = subArray(head, 0, HEAD_TYPE_LENGTH);
            byte[] headLen = subArray(head, HEAD_TYPE_LENGTH, HEAD_TYPE_LENGTH + HEAD_LEN_LENGTH);
            byte[] headCheck = subArray(head, HEAD_TYPE_LENGTH + HEAD_LEN_LENGTH, HEAD_LENGTH);

            if (headCheck == null
                    || headType == null
                    || headLen == null
                    || !equalByteArray(headCheck, getCheckByteArray(headType, headLen))) {
                throw new RuntimeException("Data validation failure!");
            }

            int dataTotalLen = byteArrayToInt(headLen);
            /*byte[] totalDataByte = new byte[]{};

            byte[] buffer = new byte[1024];

            //循环读取 直到指定长度
            int len = 0;
            int readLen = 0;
            for (; ; ) {
                len = is.read(buffer);
                if (len > 0) {
                    merge(totalDataByte, subArray(buffer, 0, len));
                    readLen += len;
                }

                if (readLen >= dataTotalLen) {
                    break;
                }
            }*/

            byte[] totalDataByte = new byte[dataTotalLen];
            int len = is.read(totalDataByte);

            byte[] dataBytes = subArray(totalDataByte, 0, dataTotalLen - END_CHECK);
            byte[] endCheck = subArray(totalDataByte, dataTotalLen - END_CHECK, dataTotalLen);

            //check
            if (dataBytes == null || !equalByteArray(endCheck, getEndCheckByteArray(dataBytes))) {
                throw new RuntimeException("Data validation failure!");
            }
            String receive = new String(dataBytes, UTF_8);
            Log.d(TAG, "receive: " + type + "-" + receive);
            handleSuccess(callback, receive);
            socket.close();
        } catch (Exception e) {
            handleFailed(callback, e);
        }
    }

    private void printByte(String name, byte[] bytes) {
        StringBuilder builder = new StringBuilder();
        for (byte b : bytes) {
            builder.append(Integer.toHexString((short) (b & 0xff)))
                    .append(" ");
        }
        Log.d(TAG, name + " : " + builder.toString());
    }

    private void handleSuccess(final SocketCallback callback, final String content) {
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                callback.OnSuccess(content);
            }
        });
    }

    private void handleFailed(final SocketCallback callback, final Exception e) {
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                callback.OnFailed(e);
            }
        });
    }

    private boolean equalByteArray(byte[] a, byte[] b) {
        if (a.length != b.length) {
            return false;
        }
        for (int i = 0; i < a.length; i++) {
            if (a[i] != b[i]) {
                return false;
            }
        }
        return true;
    }

    private byte[] subArray(byte[] bytes, int start, int end) {
        int len = end - start;
        if (len <= 0 || bytes.length < end) {
            return bytes;
        }
        byte[] result = new byte[len];
        System.arraycopy(bytes, start, result, 0, len);
        return result;
    }

    /**
     * 咸鱼科技 socket 规范
     *
     * @param type 接口类型
     * @param data 数据
     * @return byte数组
     */
    private byte[] getBytesByXianYu(int type, String data) throws Exception {
        //报文类型(INT):   4 Byte(网络序or大端序) 详见报文接口设计
        byte[] headType = intToByteArray(type);
        //报文长度(INT): 4 Byte(网络序or大端序) 包体长 + 2Byte(包尾校验字)
        byte[] headLen = intToByteArray(data.getBytes(UTF_8).length + END_CHECK);
        //包头校验(SHORT): 2 Byte(网络序or大端序) [报文类型+报文长度] 单个字节累加之和
        byte[] headCheck = getCheckByteArray(headType, headLen);

//        printByte("headType", headType);
//        printByte("headLen", headLen);
//        printByte("headCheck", headCheck);
        //包体
        byte[] pkgData = data.getBytes(UTF_8);
        //包尾校验(SHORT): 2 Byte(网络序or大端序) [包体内容] 单个字节累加之和
        byte[] end = getEndCheckByteArray(pkgData);
        return mergeArray(headType, headLen, headCheck, pkgData, end);
    }


    private byte[] mergeArray(byte[]... array) {

        byte[] result = null;

        for (int i = 0; i < array.length; i++) {
            if (i == 0) {
                result = array[0];
            } else {
                result = merge(result, array[i]);
            }
        }
        return result;
    }

    private byte[] merge(byte[] a, byte[] b) {

        byte[] bytes = Arrays.copyOf(a, a.length + b.length);

        System.arraycopy(b, 0, bytes, a.length, b.length);

        return bytes;
    }

    private byte[] getCheckByteArray(byte[] headType, byte[] headLen) {
        short check = 0;

        for (byte b : headType) {
            short a = (short) (b & 0xff);
            check += a;
        }

        for (byte b : headLen) {
            short a = (short) ((b & 0xff));
            check += a;
        }
        return shortToByteArray(check);
    }

    private byte[] getEndCheckByteArray(byte[] pkgData) {
        short endCheck = 0;
        for (byte b : pkgData) {
            short a = (short) ((b & 0xff));
            endCheck += a;
        }
        return shortToByteArray(endCheck);
    }

    private byte[] intToByteArray(int a) {
        return new byte[]{
                (byte) ((a >> 24) & 0xFF),
                (byte) ((a >> 16) & 0xFF),
                (byte) ((a >> 8) & 0xFF),
                (byte) (a & 0xFF)
        };
    }

    private int byteArrayToInt(byte[] b) {
        return b[3] & 0xFF |
                (b[2] & 0xFF) << 8 |
                (b[1] & 0xFF) << 16 |
                (b[0] & 0xFF) << 24;
    }

    private byte[] shortToByteArray(short a) {
        return new byte[]{
                (byte) ((a >> 8) & 0xFF),
                (byte) (a & 0xFF)
        };
    }


    public interface SocketCallback {

        void OnSuccess(String content);

        void OnFailed(Exception e);
    }
}
