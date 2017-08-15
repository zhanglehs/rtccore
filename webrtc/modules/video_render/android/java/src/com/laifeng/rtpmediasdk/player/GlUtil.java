package com.laifeng.rtpmediasdk.player;

import android.annotation.TargetApi;
import android.graphics.Bitmap;
import android.opengl.EGL14;
import android.opengl.GLES20;
import android.util.Log;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

@TargetApi(18)
public class GlUtil {
	private static final String TAG = "GlUtil";

	public static int createProgram(String vertexSource, String fragmentSource) {
		// 编译链接opengl代码
		int vs = loadShader(GLES20.GL_VERTEX_SHADER, vertexSource);
		int fs = loadShader(GLES20.GL_FRAGMENT_SHADER, fragmentSource);
		int program = GLES20.glCreateProgram();
		GLES20.glAttachShader(program, vs);
		GLES20.glAttachShader(program, fs);
		GLES20.glLinkProgram(program);
		int[] linkStatus = new int[1];
		GLES20.glGetProgramiv(program, GLES20.GL_LINK_STATUS, linkStatus, 0);
		if (linkStatus[0] != GLES20.GL_TRUE) {
			Log.e(TAG, "Could not link program:");
			Log.e(TAG, GLES20.glGetProgramInfoLog(program));
			GLES20.glDeleteProgram(program);
			program = 0;
		}
		return program;
	}

	public static int loadShader(int shaderType, String source) {
		int shader = GLES20.glCreateShader(shaderType);
		GLES20.glShaderSource(shader, source);
		GLES20.glCompileShader(shader);
		//
		int[] compiled = new int[1];
		GLES20.glGetShaderiv(shader, GLES20.GL_COMPILE_STATUS, compiled, 0);
		if (compiled[0] == 0) {
			Log.e(TAG, "Could not compile shader(TYPE=" + shaderType + "):");
			Log.e(TAG, GLES20.glGetShaderInfoLog(shader));
			GLES20.glDeleteShader(shader);
			shader = 0;
		}
		//
		return shader;
	}

	public static void checkGlError(String op) {
		int error;
		while ((error = GLES20.glGetError()) != GLES20.GL_NO_ERROR) {
			Log.e(TAG, op + ": glGetError: 0x" + Integer.toHexString(error));
			throw new RuntimeException("glGetError encountered (see log)");
		}
	}

	public static void checkEglError(String op) {
		int error;
		while ((error = EGL14.eglGetError()) != EGL14.EGL_SUCCESS) {
			Log.e(TAG, op + ": eglGetError: 0x" + Integer.toHexString(error));
			throw new RuntimeException("eglGetError encountered (see log)");
		}
	}

	public static void saveFrameToBitmap(int width, int height, String path) {
		if(!path.endsWith(".jpg") && !path.endsWith(".png") && !path.endsWith(".webp")) {
			return;
		}
		ByteBuffer buf = ByteBuffer.allocateDirect(width * height * 4);
		buf.order(ByteOrder.LITTLE_ENDIAN);
		GLES20.glReadPixels(0, 0, width, height,
				GLES20.GL_RGBA, GLES20.GL_UNSIGNED_BYTE, buf);
		GlUtil.checkGlError("glReadPixels");
		buf.rewind();

		Bitmap bmp = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
		bmp.copyPixelsFromBuffer(buf);
		if(bmp != null) {
			saveBitmap(bmp, path);
		}
	}

	public static void saveBitmap(Bitmap bitmap, String path) {
		if (bitmap == null) {
			return;
		}
		File f = new File(path);
		if(f.exists()) {
			f.delete();
		}
		try {
			FileOutputStream out = new FileOutputStream(f);
			if(path.endsWith(".jpg")) {
				bitmap.compress(Bitmap.CompressFormat.JPEG, 100, out);
			} else if(path.endsWith(".png")) {
				bitmap.compress(Bitmap.CompressFormat.PNG, 100, out);
			} else if(path.endsWith(".webp")) {
				bitmap.compress(Bitmap.CompressFormat.WEBP, 100, out);
			}
			out.flush();
			out.close();
		} catch (FileNotFoundException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
}
