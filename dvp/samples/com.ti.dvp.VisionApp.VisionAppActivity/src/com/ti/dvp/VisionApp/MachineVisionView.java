/*
 *  Copyright (C) 2 Texas Instruments, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.ti.dvp.VisionApp;

import com.ti.dvp.ImageFilter;
import com.ti.dvp.VisionEngine;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicInteger;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.content.Context;
import android.graphics.SurfaceTexture;
import android.graphics.SurfaceTexture.OnFrameAvailableListener;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;


/** Implement a simple rotation control. */
class MachineVisionView extends GLSurfaceView implements VisionEngine.GraphCompletedListener {

    private static final String TAG = "MachineVisionView";
    private MachineVisionRenderer mRenderer;
    private int mTextureName;
    private SurfaceTexture mSurfaceTexture;

    AtomicInteger mReportedFrameCount = new AtomicInteger();
    AtomicBoolean mCameraEnabled = new AtomicBoolean();
    AtomicInteger mCameraFrameCount = new AtomicInteger();

    public double timeFPS;
    
    private ImageFilter m_filter;

    private byte[] m_input;
    private byte[] m_output;

    public interface onViewSetupListener {
        public void onViewSetupComplete(SurfaceTexture texture);
    }

    protected onViewSetupListener mListener;

    public static int loadShader(int shaderType, String source) {
        int shader = GLES20.glCreateShader(shaderType);
        if (shader != 0) {
            GLES20.glShaderSource(shader, source);
            GLES20.glCompileShader(shader);
            int[] compiled = new int[1];
            GLES20.glGetShaderiv(shader, GLES20.GL_COMPILE_STATUS, compiled, 0);
            if (compiled[0] == 0) {
                Log.e(TAG, "Could not compile shader " + shaderType + ":");
                Log.e(TAG, GLES20.glGetShaderInfoLog(shader));
                GLES20.glDeleteShader(shader);
                shader = 0;
            }
        }
        return shader;
    }

    public static void checkGlError(String op) {
        int error;
        while ((error = GLES20.glGetError()) != GLES20.GL_NO_ERROR) {
            Log.e(TAG, op + ": glError " + error);
            throw new RuntimeException(op + ": glError " + error);
        }
    }

    public static int createProgram(String vertexSource, String fragmentSource) {
        int vertexShader = loadShader(GLES20.GL_VERTEX_SHADER, vertexSource);
        if (vertexShader == 0) {
            return 0;
        }
        int pixelShader = loadShader(GLES20.GL_FRAGMENT_SHADER, fragmentSource);
        if (pixelShader == 0) {
            return 0;
        }

        int program = GLES20.glCreateProgram();
        if (program != 0) {
            GLES20.glAttachShader(program, vertexShader);
            checkGlError("glAttachShader");
            GLES20.glAttachShader(program, pixelShader);
            checkGlError("glAttachShader");
            GLES20.glLinkProgram(program);
            int[] linkStatus = new int[1];
            GLES20.glGetProgramiv(program, GLES20.GL_LINK_STATUS, linkStatus, 0);
            if (linkStatus[0] != GLES20.GL_TRUE) {
                Log.e(TAG, "Could not link program: ");
                Log.e(TAG, GLES20.glGetProgramInfoLog(program));
                GLES20.glDeleteProgram(program);
                program = 0;
            }
        }
        return program;
    }


    public MachineVisionView(Context context, onViewSetupListener listener) {
        super(context);
        setEGLContextClientVersion(2);
        mListener = listener;
        mRenderer = new MachineVisionRenderer();
        setRenderer(mRenderer);
        setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
        m_input = new byte[1280*800*4];
        m_output = new byte[1280*800*4];
        m_filter = new ImageFilter();
        m_filter.init();
    }
    
    public MachineVisionView(Context context, AttributeSet attrs, onViewSetupListener listener) {
        super(context, attrs);
        setEGLContextClientVersion(2);
        mListener = listener;
        mRenderer = new MachineVisionRenderer();
        setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
        setRenderer(new MachineVisionRenderer());
        m_input = new byte[1280*800*4];
        m_output = new byte[1280*800*4];
        m_filter = new ImageFilter();
        m_filter.init();
    }
    
    public void finalize()
    {
    	m_filter.deinit();
    }

    @Override
    public boolean onTrackballEvent(MotionEvent e) {
        requestRender();
        return true;
    }

    @Override
    public boolean onTouchEvent(MotionEvent e) {
        float x = e.getX();
        float y = e.getY();
        Log.d(TAG, "Touch Event at "+x+","+y+".");
        m_filter.filter3x1(m_input, 1280, 800, m_output, 8);
        
        switch (e.getAction()) {
	        case MotionEvent.ACTION_MOVE:
	        	requestRender();
        }
        return true;
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {

        Log.v(TAG, " onKeyDown[code:"+keyCode+", event:"+event.toString()+"]");

        switch(event.getAction()) {
            case KeyEvent.ACTION_DOWN:
                break;
        }
        return super.onKeyDown(keyCode, event);
    }

    @Override public boolean onKeyUp(int keyCode, KeyEvent event) {

        Log.v("MachineVisionView", " keyDown[code:"+keyCode+", event:"+event.toString()+"]");

        switch(event.getAction()) {
            case KeyEvent.ACTION_UP:
                break;
        }
        return super.onKeyUp(keyCode, event);
    }
    
    public void onGraphCompleted() {
    	requestRender();
    }

    /**
     * Render a cube.
     */
    private class MachineVisionRenderer implements GLSurfaceView.Renderer, OnFrameAvailableListener {

        private static final String TAG = "MachineVisionRenderer";
        private final static String VERTEX_SHADER =
            "attribute vec4 vPosition;\n" +
            "attribute vec2 a_texCoord;\n" +
            "varying vec2 v_texCoord;\n" +
            "uniform mat4 u_xform;\n" +
            "void main() {\n" +
            "  gl_Position = vPosition;\n" +
            "  v_texCoord = vec2(u_xform * vec4(a_texCoord, 1.0, 1.0));\n" +
            "}\n";

        private final static String FRAGMENT_SHADER =
            "#extension GL_OES_EGL_image_external : require\n" +
            "precision mediump float;\n" +
            "uniform samplerExternalOES s_texture;\n" +
            "varying vec2 v_texCoord;\n" +
            "void main() {\n" +
            "  gl_FragColor = texture2D(s_texture, v_texCoord);\n" +
            "}\n";

        private final float[] TEXTURE_VERTICES =
              { 1.0f, 1.0f,
                0.0f, 1.0f,
                0.0f, 0.0f,
                1.0f, 0.0f };

        private final float[] QUAD_VERTICES =
                { 1.0f, 1.0f,
                -1.0f,  1.0f,
                -1.0f,  -1.0f,
                 1.0f,  -1.0f };

        private final static int FLOAT_SIZE_BYTES = 4;

        private final FloatBuffer mTextureVertices;
        private final FloatBuffer mQuadVertices;

        private int mGLProgram;
        private int mTexHandle;
        private int mTexCoordHandle;
        private int mTriangleVerticesHandle;
        private int mTransformHandle;
        private int mViewWidth;
        private int mViewHeight;
        private float[] mTransformMatrix;
        private int mLastCameraFrameCount;
        
        public MachineVisionRenderer() {
            mTextureVertices = ByteBuffer.allocateDirect(TEXTURE_VERTICES.length *
                    FLOAT_SIZE_BYTES).order(ByteOrder.nativeOrder()).asFloatBuffer();
            mTextureVertices.put(TEXTURE_VERTICES).position(0);
            mQuadVertices = ByteBuffer.allocateDirect(QUAD_VERTICES.length *
                    FLOAT_SIZE_BYTES).order(ByteOrder.nativeOrder()).asFloatBuffer();
            mQuadVertices.put(QUAD_VERTICES).position(0);
            mTransformMatrix = new float[16];
            mLastCameraFrameCount = mCameraFrameCount.intValue();
        }

        public void onSurfaceCreated(GL10 gl, EGLConfig config) {
            mGLProgram = createProgram(VERTEX_SHADER, FRAGMENT_SHADER);

            mTexHandle = GLES20.glGetUniformLocation(mGLProgram, "s_texture");
            mTexCoordHandle = GLES20.glGetAttribLocation(mGLProgram, "a_texCoord");
            mTriangleVerticesHandle = GLES20.glGetAttribLocation(mGLProgram, "vPosition");
            mTransformHandle = GLES20.glGetUniformLocation(mGLProgram, "u_xform");
            int[] textures = new int[1];
            GLES20.glGenTextures(1, textures, 0);
            mTextureName = textures[0];
            GLES20.glUseProgram(mGLProgram);
            GLES20.glVertexAttribPointer(mTexCoordHandle, 2, GLES20.GL_FLOAT,
                    false, 0, mTextureVertices);
            GLES20.glVertexAttribPointer(mTriangleVerticesHandle, 2, GLES20.GL_FLOAT,
                    false, 0, mQuadVertices);
            checkGlError("initialization");
            mSurfaceTexture = new SurfaceTexture(mTextureName);
            Log.d(TAG, "Creating new SurfaceTexture "+mSurfaceTexture);
            mSurfaceTexture.setOnFrameAvailableListener(this);
            if (mListener != null)
                mListener.onViewSetupComplete(mSurfaceTexture);
        }

        public void onSurfaceChanged(GL10 gl, int width, int height) {
            Log.d(TAG, "### ### ### Surface Changed! ("+width+","+height+") ### ### ###\n");
            mViewWidth = width;
            mViewHeight = height;
        }

        public void onFrameAvailable(SurfaceTexture surfaceTexture) {
            mCameraFrameCount.incrementAndGet();
            // don't leave this enabled when not debugging, its printed frequently
            // Log.d(TAG, "Frame update! "+mCameraFrameCount.get()+"\n");
        }

        private static final int GL_TEXTURE_EXTERNAL_OES = 0x8D65;
        public double prev_time = 0;

        public void onDrawFrame(GL10 gl) {
        	Log.d(TAG, "onDrawFrame()");
            timeFPS = System.currentTimeMillis() - prev_time;
            prev_time = System.currentTimeMillis();
            if(timeFPS > 0)
            {
            }
            GLES20.glUseProgram(mGLProgram);
            GLES20.glViewport(0, 0, mViewWidth, mViewHeight);
            checkGlError("glViewport");
            GLES20.glClearColor(0,0,0,0);
            GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);
            if (mLastCameraFrameCount < mCameraFrameCount.intValue()) { // new frame has arrived
                mSurfaceTexture.updateTexImage();
                mLastCameraFrameCount = mCameraFrameCount.intValue();
            }
            else
            {
                // no vision update yet.
                //Log.d(TAG, "No texture update available, re-rendering old buffer!\n");
            }
            mSurfaceTexture.getTransformMatrix(mTransformMatrix);
            GLES20.glUniformMatrix4fv(mTransformHandle, 1, false, mTransformMatrix, 0);
            checkGlError("glUniformMatrix4fv");
            GLES20.glDisable(GLES20.GL_BLEND);
            checkGlError("setup");
            GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
            checkGlError("glActiveTexture");
            GLES20.glBindTexture(GL_TEXTURE_EXTERNAL_OES, mTextureName);
            checkGlError("glBindTexture");
            GLES20.glUniform1i(mTexHandle, 0);
            checkGlError("glUniform1i");
            GLES20.glEnableVertexAttribArray(mTexCoordHandle);
            checkGlError("glEnableVertexAttribArray");
            GLES20.glEnableVertexAttribArray(mTriangleVerticesHandle);
            checkGlError("glEnableVertexAttribArray");

            mQuadVertices.put(QUAD_VERTICES).position(0);
            mTextureVertices.put(TEXTURE_VERTICES).position(0);

            GLES20.glDrawArrays(GLES20.GL_TRIANGLE_FAN, 0, 4);
            checkGlError("glDrawArrays");
        }
    }
}
