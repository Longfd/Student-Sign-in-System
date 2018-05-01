package com.xykj.studentsign.net;

import android.content.Context;
import android.os.Handler;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.util.Arrays;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

/**
 * 咸鱼科技 通信协议
 * <p>
 * 包头{
 * 请求包类型: 4 Bytes
 * 请求包长度: 4 Bytes	//INT型字节流约定 + 网络序
 * 请求包校验: 2 Bytes //
 * }
 */

public class SocketManager {
    private static final String TAG = "SocketManager";

    private static final int HEAD_LENGTH = 10;
    private static final int HEAD_TYPE_LENGTH = 4;
    private static final int HEAD_LEN_LENGTH = 4;
    private static final int HEAD_CHECK_LENGTH = 2;

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
            os.write(bytes);
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

            if (!equalByteArray(headCheck, getCheckByteArray(headType, headLen))) {
                throw new RuntimeException("Data validation failure!  ---by XianYu Tech.");
            }

            int dataTotalLen = byteArrayToInt(headLen);
            byte[] buffer = new byte[1024];
            int len;
            StringBuilder stringBuffer = new StringBuilder();
            while ((len = is.read(buffer)) > 0) {
                byte[] bytes = buffer;
                if (len < 1024) {
                    bytes = subArray(buffer, 0, len);
                }
                if (bytes != null) {
                    stringBuffer.append(new String(bytes, UTF_8));
                }
                dataTotalLen -= len;
                if (dataTotalLen <= 0) {
                    break;
                }
            }
            handleSuccess(callback, stringBuffer.toString());
            socket.close();
        } catch (Exception e) {
            handleFailed(callback, e);
        }
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
            return null;
        }
        byte[] result = new byte[len];
        System.arraycopy(bytes, start, result, 0, len);
        return result;
    }

    /**
     * 咸鱼科技 socket 规范
     * <p>
     * 包头{
     * 请求包类型: 4 Bytes
     * 请求包长度: 4 Bytes	//INT型字节流约定 + 网络序
     * 请求包校验: 2 Bytes //
     * }
     *
     * @param type 接口类型
     * @param data 数据
     * @return byte数组
     */
    private byte[] getBytesByXianYu(int type, String data) throws Exception {
        byte[] headType = intToByteArray(type);
        byte[] headLen = intToByteArray(data.getBytes().length);
        byte[] headCheck = getCheckByteArray(headType, headLen);
        byte[] pkgData = data.getBytes(UTF_8);
        return mergeArray(headType, headLen, headCheck, pkgData);
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
            check += b;
        }

        for (byte b : headLen) {
            check += b;
        }
        return shortToByteArray(check);
    }

    private byte[] intToByteArray(int a) {
        return new byte[]{
                (byte) ((a >> 24) & 0xFF),
                (byte) ((a >> 16) & 0xFF),
                (byte) ((a >> 8) & 0xFF),
                (byte) (a & 0xFF)
        };
    }

    public int byteArrayToInt(byte[] b) {
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
