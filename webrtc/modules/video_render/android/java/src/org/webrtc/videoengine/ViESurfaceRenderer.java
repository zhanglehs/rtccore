/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

package org.webrtc.videoengine;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;

import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Rect;
import android.graphics.Paint;
import android.opengl.GLES20;
import android.opengl.GLUtils;
import android.util.Log;
import android.view.Surface;

import com.laifeng.rtpmediasdk.player.GlCoordUtil;
import com.laifeng.rtpmediasdk.player.InputSurface;
import com.laifeng.rtpmediasdk.player.GlUtil;

public class ViESurfaceRenderer {

    private final static String TAG = "WEBRTC";

    private Bitmap bitmap = null;

    private InputSurface mInputSurface = null;
    private int mProgram = -1;
    private int maPositionHandle = -1;
    private int maTexCoordHandle = -1;
    private int muPosMtxHandle = -1;
    private int muSamplerHandle = -1;
    private int mTextureId = -1;
    private FloatBuffer mImageVertexBuffer = null;
    private float[] mNormalMtx = null;
    private FloatBuffer mNormalTexCoordBuf = null;

    private ByteBuffer byteBuffer = null;
    private Surface surface = null;
    private Rect srcRect = null;
    private Rect dstRect = null;
    private Paint paint = null;

    private boolean mUseGL = true;

    public ViESurfaceRenderer(Surface view, boolean useGL) {
        Log.d(TAG, "ViESurfaceRenderer create");
        surface = view;
        mUseGL = useGL;

        if (mUseGL) {
            mInputSurface = new InputSurface(surface);
            mNormalMtx = GlCoordUtil.createIdentityMtx();
            mNormalTexCoordBuf = GlCoordUtil.createTexCoordBuffer();
        }
        else {
            srcRect = new Rect();
            srcRect.left = 0;
            srcRect.top = 0;
            dstRect = new Rect();
            dstRect.left = 0;
            dstRect.top = 0;
            paint = new Paint();
            paint.setAntiAlias(true);
            paint.setFilterBitmap(true);
        }
    }

    public void Stop() {
        if (mInputSurface != null) {
            mInputSurface.release();
            mInputSurface = null;
        }
        if (mTextureId > 0) {
            destroyTexture(mTextureId);
            mTextureId = -1;
        }
    }

    public Bitmap CreateBitmap(int width, int height) {
        Log.d(TAG, "CreateByteBitmap " + width + ":" + height);
        if (bitmap == null) {
            try {
                android.os.Process.setThreadPriority(
                    android.os.Process.THREAD_PRIORITY_DISPLAY);
            }
            catch (Exception e) {
            }
        }
        bitmap = Bitmap.createBitmap(width, height, Bitmap.Config.RGB_565);
        return bitmap;
    }

    public void SetCoordinates(float left, float top, float right, float bottom) {
    }

    public ByteBuffer CreateByteBuffer(int width, int height) {
        Log.d(TAG, "CreateByteBuffer " + width + ":" + height);
        if (bitmap == null) {
            bitmap = CreateBitmap(width, height);
            byteBuffer = ByteBuffer.allocateDirect(width * height * 2);
        }
        return byteBuffer;
    }

    public void DrawByteBuffer() {
        if (byteBuffer == null || bitmap == null) {
            return;
        }
        byteBuffer.rewind();
        bitmap.copyPixelsFromBuffer(byteBuffer);  // GLES模式下占DrawByteBuffer()的4.9%的cpu消耗

        try {
            if (mUseGL) {
                RenderFrame_GLES(bitmap);
            }
            else {
                RenderFrame_software(bitmap);
            }
        }
        catch (Exception e) {
            Log.w(TAG, e.getMessage());
        }
    }

    private void RenderFrame_software(Bitmap bitmap) {
        Canvas canvas = surface.lockCanvas(null);
        if (canvas != null) {
            srcRect.right = bitmap.getWidth();
            srcRect.bottom = bitmap.getHeight();
            dstRect.right = canvas.getWidth();
            dstRect.bottom = canvas.getHeight();
            canvas.drawBitmap(bitmap, srcRect, dstRect, paint);
            surface.unlockCanvasAndPost(canvas);
        }
    }

    private void RenderFrame_GLES(Bitmap bitmap) {
        mInputSurface.makeCurrent();

        initGLES();

        int windowWidth = mInputSurface.getWidth();
        int windowHeight = mInputSurface.getHeight();
        if (windowWidth <= 0 || windowHeight <= 0) {
            return;
        }

        GLES20.glViewport(0, 0, windowWidth, windowHeight);
        GLES20.glClearColor(0f, 0f, 0f, 1f);
        GLES20.glClear(GLES20.GL_DEPTH_BUFFER_BIT | GLES20.GL_COLOR_BUFFER_BIT); // 占RenderFrame_GLES的33.5%的cpu消耗
        GLES20.glUseProgram(mProgram);

        if (mTextureId < 0) {
            mTextureId = createTexture();
            if (mTextureId <= 0) {
                return;
            }
        }
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, mTextureId);

        GLUtils.texImage2D(GLES20.GL_TEXTURE_2D, 0, bitmap, 0); // 占RenderFrame_GLES的14.4%的cpu消耗
        GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MIN_FILTER,
                GLES20.GL_LINEAR);
        GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MAG_FILTER,
                GLES20.GL_LINEAR);
        GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_S,
                GLES20.GL_CLAMP_TO_EDGE);
        GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_T,
                GLES20.GL_CLAMP_TO_EDGE);

        if (muPosMtxHandle>= 0) {
            GLES20.glUniformMatrix4fv(muPosMtxHandle, 1, false, mNormalMtx, 0);
        }

        if (mImageVertexBuffer == null) {
			// TODO: zhangle, surface宽高变化时，mImageVertexBuffer应该重新赋值
            mImageVertexBuffer = GlCoordUtil.generateImageVertexFullCoordinate(bitmap.getWidth(), bitmap.getHeight(), windowWidth, windowHeight);
            if (mImageVertexBuffer == null) {
                return;
            }
        }
        mImageVertexBuffer.position(0);
        GLES20.glVertexAttribPointer(maPositionHandle,
                3, GLES20.GL_FLOAT, false, 4 * 3, mImageVertexBuffer);
        GLES20.glEnableVertexAttribArray(maPositionHandle);

        mNormalTexCoordBuf.position(0);
        GLES20.glVertexAttribPointer(maTexCoordHandle,
                2, GLES20.GL_FLOAT, false, 4 * 2, mNormalTexCoordBuf);
        GLES20.glEnableVertexAttribArray(maTexCoordHandle);

        GLES20.glBlendFunc(GLES20.GL_SRC_ALPHA, GLES20.GL_ONE_MINUS_SRC_ALPHA);
        GLES20.glEnable(GLES20.GL_BLEND);

        GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 0, 4); // 占RenderFrame_GLES的7.5%的cpu消耗
        GLES20.glDisable(GLES20.GL_BLEND);

        mInputSurface.swapBuffers();   // 占RenderFrame_GLES的35.2%的cpu消耗
        mInputSurface.setPresentationTime(System.nanoTime());
    }

    private int createTexture() {
        int[] textures = new int[1];
        textures[0] = -1;
        GLES20.glGenTextures(1, textures, 0);
        return textures[0];
    }

    private void destroyTexture(int texture_id) {
        if (texture_id > 0) {
            int[] textures = new int[1];
            textures[0] = texture_id;
            GLES20.glDeleteTextures(1, textures, 0);
        }
    }

    private void initGLES() {
        if (mProgram >= 0) {
            return;
        }

        GlUtil.checkGlError("initGL_S");

        final String vertexShader =
           "attribute vec4 position;\n" +
           "attribute vec4 inputTextureCoordinate;\n" +
           "uniform   mat4 uPosMtx;\n" +
           "varying   vec2 textureCoordinate;\n" +
           "void main() {\n" +
           "  gl_Position = uPosMtx * position;\n" +
           "  textureCoordinate   = inputTextureCoordinate.xy;\n" +
           "}\n";
        final String fragmentShader =
           "precision mediump float;\n" +
           "uniform sampler2D uSampler;\n" +
           "varying vec2  textureCoordinate;\n" +
           "void main() {\n" +
           "  gl_FragColor = texture2D(uSampler, textureCoordinate);\n" +
           "}\n";
        mProgram = GlUtil.createProgram(vertexShader, fragmentShader);

        // 缓存上面opengl代码的部分变量
        maPositionHandle = GLES20.glGetAttribLocation(mProgram, "position");
        maTexCoordHandle = GLES20.glGetAttribLocation(mProgram, "inputTextureCoordinate");
        muPosMtxHandle = GLES20.glGetUniformLocation(mProgram, "uPosMtx");
        muSamplerHandle = GLES20.glGetUniformLocation(mProgram, "uSampler");

        GlUtil.checkGlError("initGL_E");
    }

}
