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

// The following four imports are needed saveBitmapToJPEG which
// is for debug only
import java.io.ByteArrayOutputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;

import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Rect;
import android.graphics.Paint;
import android.util.Log;
import android.view.Surface;

public class ViESurfaceRenderer {

    private final static String TAG = "WEBRTC";

    // the bitmap used for drawing.
    private Bitmap bitmap = null;
    private ByteBuffer byteBuffer = null;
    private Surface surface;
    // Rect of the source bitmap to draw
    private Rect srcRect = new Rect();
    // Rect of the destination canvas to draw to
    private Rect dstRect = new Rect();
    private float dstTopScale = 0;
    private float dstBottomScale = 1;
    private float dstLeftScale = 0;
    private float dstRightScale = 1;
    private Paint paint = new Paint();

    public ViESurfaceRenderer(Surface view) {
        Log.d(TAG, "ViESurfaceRenderer create");
        surface = view;
        paint.setAntiAlias(true);
        paint.setFilterBitmap(true);
    }

    // surfaceChanged and surfaceCreated share this function
    private void changeDestRect(int dstWidth, int dstHeight) {
        dstRect.right = (int)(dstRect.left + dstRightScale * dstWidth);
        dstRect.bottom = (int)(dstRect.top + dstBottomScale * dstHeight);
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
        srcRect.left = 0;
        srcRect.top = 0;
        srcRect.bottom = height;
        srcRect.right = width;
        return bitmap;
    }

    public ByteBuffer CreateByteBuffer(int width, int height) {
        Log.d(TAG, "CreateByteBuffer " + width + ":" + height);
        if (bitmap == null) {
            bitmap = CreateBitmap(width, height);
            byteBuffer = ByteBuffer.allocateDirect(width * height * 2);
        }
        return byteBuffer;
    }

    public void SetCoordinates(float left, float top,
            float right, float bottom) {
        Log.d(TAG, "SetCoordinates " + left + "," + top + ":" +
                right + "," + bottom);
        dstLeftScale = left;
        dstTopScale = top;
        dstRightScale = right;
        dstBottomScale = bottom;
    }

    // It saves bitmap data to a JPEG picture, this function is for debug only.
    private void saveBitmapToJPEG(int width, int height) {
        ByteArrayOutputStream byteOutStream = new ByteArrayOutputStream();
        bitmap.compress(Bitmap.CompressFormat.JPEG, 100, byteOutStream);

        try{
            FileOutputStream output = new FileOutputStream(String.format(
                "/sdcard/render_%d.jpg", System.currentTimeMillis()));
            output.write(byteOutStream.toByteArray());
            output.flush();
            output.close();
        }
        catch (FileNotFoundException e) {
        }
        catch (IOException e) {
        }
    }

    public void DrawByteBuffer() {
        if (byteBuffer == null) {
            return;
        }
        byteBuffer.rewind();
        bitmap.copyPixelsFromBuffer(byteBuffer);
        DrawBitmap();
    }

    public void DrawBitmap() {
        if (bitmap == null) {
            return;
        }
        if (!surface.isValid()) {
            return;
        }

        try {
            Canvas canvas = surface.lockCanvas(null);
            if (canvas != null) {
                // The follow line is for debug only
                // saveBitmapToJPEG(srcRect.right - srcRect.left,
                //                  srcRect.bottom - srcRect.top);
                dstRect.left = 0;
                dstRect.top = 0;
                dstRect.right = canvas.getWidth();
                dstRect.bottom = canvas.getHeight();
                canvas.drawBitmap(bitmap, srcRect, dstRect, paint);
                surface.unlockCanvasAndPost(canvas);
            }
        }
        catch (Exception e) {
            // Log.d(TAG, e.getMessage());
        }
    }

}
