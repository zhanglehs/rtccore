package com.laifeng.rtpmediasdk.player;

import android.opengl.Matrix;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;

/**
 * @Title: GlCoordUtil
 * @Description:
 * @Author Jim
 * @Date 2017/1/17
 * @Time 下午4:26
 * @Version
 */

public class GlCoordUtil {
    public static FloatBuffer createSquareVtx() {
        final float vtx[] = {
                // XYZ, UV
                -1f,  1f, 0f, 0f, 1f,
                -1f, -1f, 0f, 0f, 0f,
                1f,   1f, 0f, 1f, 1f,
                1f,  -1f, 0f, 1f, 0f,
        };
        ByteBuffer bb = ByteBuffer.allocateDirect(4 * vtx.length);
        bb.order(ByteOrder.nativeOrder());
        FloatBuffer fb = bb.asFloatBuffer();
        fb.put(vtx);
        fb.position(0);
        return fb;
    }

    public static FloatBuffer createVertexBuffer() {
        final float vtx[] = {
                // XYZ
                -1f,  1f, 0f,
                -1f, -1f, 0f,
                1f,   1f, 0f,
                1f,  -1f, 0f,
        };
        ByteBuffer bb = ByteBuffer.allocateDirect(4 * vtx.length);
        bb.order(ByteOrder.nativeOrder());
        FloatBuffer fb = bb.asFloatBuffer();
        fb.put(vtx);
        fb.position(0);
        return fb;
    }

    public static FloatBuffer createTexCoordBuffer() {
        final float vtx[] = {
                // UV
                0f, 1f,
                0f, 0f,
                1f, 1f,
                1f, 0f,
        };
        ByteBuffer bb = ByteBuffer.allocateDirect(4 * vtx.length);
        bb.order(ByteOrder.nativeOrder());
        FloatBuffer fb = bb.asFloatBuffer();
        fb.put(vtx);
        fb.position(0);
        return fb;
    }

    public static float[] createIdentityMtx() {
        float[] m = new float[16];
        Matrix.setIdentityM(m, 0);
        return m;
    }

    public static FloatBuffer generateCameraTextureCoordinate(int cameraWidth2, int cameraHeight2, int screenWidth, int screenHeight) {
        if(cameraWidth2 <= 0 || cameraHeight2 <= 0|| screenHeight <= 0 || screenWidth <= 0) {
            return null;
        }

        FloatBuffer cameraTexCoordBuffer;
        int cameraWidth, cameraHeight;

        //if(CameraHolder.instance().isLandscape()) {
        if (true) {
            cameraWidth = Math.max(cameraWidth2, cameraHeight2);
            cameraHeight = Math.min(cameraWidth2, cameraHeight2);
        } else {
            cameraWidth = Math.min(cameraWidth2, cameraHeight2);
            cameraHeight = Math.max(cameraWidth2, cameraHeight2);
        }

        float hRatio = screenWidth / ((float)cameraWidth);
        float vRatio = screenHeight / ((float)cameraHeight);

        float ratio;
        if(hRatio > vRatio) {
            ratio = screenHeight / (cameraHeight * hRatio);
            final float vtx[] = {
                    //UV
                    0f, 0.5f + ratio/2,
                    0f, 0.5f - ratio/2,
                    1f, 0.5f + ratio/2,
                    1f, 0.5f - ratio/2,
            };
            ByteBuffer bb = ByteBuffer.allocateDirect(4 * vtx.length);
            bb.order(ByteOrder.nativeOrder());
            cameraTexCoordBuffer = bb.asFloatBuffer();
            cameraTexCoordBuffer.put(vtx);
            cameraTexCoordBuffer.position(0);
        } else {
            ratio = screenWidth/ (cameraWidth * vRatio);
            final float vtx[] = {
                    //UV
                    0.5f - ratio/2, 1f,
                    0.5f - ratio/2, 0f,
                    0.5f + ratio/2, 1f,
                    0.5f + ratio/2, 0f,
            };
            ByteBuffer bb = ByteBuffer.allocateDirect(4 * vtx.length);
            bb.order(ByteOrder.nativeOrder());
            cameraTexCoordBuffer = bb.asFloatBuffer();
            cameraTexCoordBuffer.put(vtx);
            cameraTexCoordBuffer.position(0);
        }
        return cameraTexCoordBuffer;
    }

    public static FloatBuffer generateImageVertexFullCoordinate(int imageWidth, int imageHeight, int screenWidth, int screenHeight) {
        if(imageWidth <= 0 || imageHeight <= 0 || screenHeight <= 0 || screenWidth <= 0) {
            return null;
        }

        FloatBuffer imageTexCoordBuffer;
        float hRatio = screenWidth / ((float)imageWidth);
        float vRatio = screenHeight / ((float)imageHeight);

        float ratio;
        if(hRatio > vRatio) {
            ratio = (imageWidth * vRatio) / screenWidth;
            final float vtx[] = {
                    //UV
                    -ratio, -1.0f, 0.0f,
                    -ratio, 1f, 0.0f,
                    ratio, -1.0f, 0.0f,
                    ratio, 1f, 0.0f,
            };
            ByteBuffer bb = ByteBuffer.allocateDirect(4 * vtx.length);
            bb.order(ByteOrder.nativeOrder());
            imageTexCoordBuffer = bb.asFloatBuffer();
            imageTexCoordBuffer.put(vtx);
            imageTexCoordBuffer.position(0);
        } else {
            ratio = (imageHeight * hRatio) / screenHeight;
            final float vtx[] = {
                    //UV
                    -1.0f, -ratio, 0.0f,
                    -1.0f, ratio, 0.0f,
                    1f, -ratio, 0.0f,
                    1f, ratio, 0.0f,
            };
            ByteBuffer bb = ByteBuffer.allocateDirect(4 * vtx.length);
            bb.order(ByteOrder.nativeOrder());
            imageTexCoordBuffer = bb.asFloatBuffer();
            imageTexCoordBuffer.put(vtx);
            imageTexCoordBuffer.position(0);
        }
        return imageTexCoordBuffer;
    }
}
