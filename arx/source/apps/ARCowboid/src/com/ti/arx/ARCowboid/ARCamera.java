/*
 *  Copyright (C) 2012 Texas Instruments, Inc.
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

package com.ti.arx.ARCowboid;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.view.Surface;

public class ARCamera implements GLSurfaceView.Renderer {

    public interface Callback {
        public void onARCameraDeath();
        public void onFPSChange(double fps);
    }
    
    public ARCamera(Context context, int width, int height) {
        mCamPoseMatrix = new float[12];
        mProjMatrix = new float[16];
        mCamViewMatrix = new float[16];
        mCube = new TexturedCube(context.getResources(), R.drawable.cowboid);
        
        mCamWidth = width;
        mCamHeight = height;
        
        mDraw = false;
        mTimestamp = 0;
    }
    
    public void setGLView(GLSurfaceView view) {
        mView = view;
    }

    public boolean start(Surface surf) {
        return create(mCamPoseMatrix, surf);
    }

    public void stop() {
        destroy();
    }    

    public void setCallback(Callback cb) {
        mListener = cb;        
    }

    @Override
    public void onDrawFrame(GL10 gl) {
        gl.glClear(GL10.GL_COLOR_BUFFER_BIT | GL10.GL_DEPTH_BUFFER_BIT);

        if (mDraw) {        
            gl.glMatrixMode(GL10.GL_PROJECTION);
            gl.glLoadIdentity();
            gl.glLoadMatrixf(mProjMatrix, 0);
    
            gl.glMatrixMode(GL10.GL_MODELVIEW);
            gl.glLoadIdentity();
            gl.glScalef(1f, -1f, -1f);
    
            gl.glMultMatrixf(mCamViewMatrix, 0);
            
            gl.glPushMatrix();
            gl.glScalef(0.5f, 0.5f, 0.5f);
            gl.glTranslatef(0.0f, 0.0f, 1.0f);
            mCube.draw(gl);
            gl.glPopMatrix(); 
        }
            
        renderPreview(mTimestamp);
        
        stopPerf();
        startPerf();
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        gl.glViewport(0, 0, width, height);
        gl.glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        gl.glEnable(GL10.GL_TEXTURE_2D);
        gl.glEnable(GL10.GL_CULL_FACE);
        gl.glFrontFace(GL10.GL_CW);
        
        mCube.setup(gl);

        //original k matrix is for 320x240 images.
        float k_scale = (float)width/320.0f;
        mProjMatrix[0] = k_scale*2*K[0]/width;
        mProjMatrix[4] = k_scale*K[1];
        mProjMatrix[8] = k_scale*2*(K[2]/width) - 1;
        mProjMatrix[12] = 0;
        mProjMatrix[1] = k_scale*K[3];
        mProjMatrix[5] = k_scale*2*K[4]/height;
        mProjMatrix[9] = k_scale*2*(K[5]/height) - 1;
        mProjMatrix[13] = 0;
        mProjMatrix[2] = k_scale*K[6];
        mProjMatrix[6] = k_scale*K[7];
        mProjMatrix[10] = -(FRUSTUM_NEAR_Z + FRUSTUM_FAR_Z)/(FRUSTUM_FAR_Z - FRUSTUM_NEAR_Z);
        mProjMatrix[14] = -(2*FRUSTUM_NEAR_Z*FRUSTUM_FAR_Z)/(FRUSTUM_FAR_Z - FRUSTUM_NEAR_Z);
        mProjMatrix[3] = 0;
        mProjMatrix[7] = 0;
        mProjMatrix[11] = -1;
        mProjMatrix[15] = 0;     
        
        resetPerf();
        startPerf();
    }

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {        
    }    
    
    private void onCamPoseUpdate(long timestamp, int status) {
        mCamViewMatrix[0] = mCamPoseMatrix[0];
        mCamViewMatrix[4] = mCamPoseMatrix[1];
        mCamViewMatrix[8] = mCamPoseMatrix[2];
        mCamViewMatrix[12] = mCamPoseMatrix[3];
        mCamViewMatrix[1] = mCamPoseMatrix[4];
        mCamViewMatrix[5] = mCamPoseMatrix[5];
        mCamViewMatrix[9] = mCamPoseMatrix[6];
        mCamViewMatrix[13] = mCamPoseMatrix[7];
        mCamViewMatrix[2] = mCamPoseMatrix[8];
        mCamViewMatrix[6] = mCamPoseMatrix[9];
        mCamViewMatrix[10] = mCamPoseMatrix[10];
        mCamViewMatrix[14] = mCamPoseMatrix[11];
        mCamViewMatrix[3] = 0;
        mCamViewMatrix[7] = 0;
        mCamViewMatrix[11] = 0;
        mCamViewMatrix[15] = 1;
        mTimestamp = timestamp;
        mDraw = status != 0; 
        mView.requestRender();
    }

    private void onARXDeath() {
        if (mListener != null) {
            mListener.onARCameraDeath();
        }
    }
    
    private void startPerf() {
        mStartTime = System.currentTimeMillis();
    }
    
    private void stopPerf() {        
        long duration = System.currentTimeMillis() - mStartTime;
        mDurationSum += duration;
        mFramesRendered++;
        
        if (mFramesRendered > MAXFRAMES) {
            double fps = (1000.0*(double)mFramesRendered)/mDurationSum;
            if (mListener != null) {
                mListener.onFPSChange(fps);
            }
            mFramesRendered = 0;
            mDurationSum = 0;
        }
    }
    
    private void resetPerf() {
        mDurationSum = 0;
        mFramesRendered = 0;
    }
    
    public native void reset();
    private native boolean create(float[] viewMatrix, Surface surface);
    private native void destroy();
    private native void renderPreview(long timestamp);
    
    private int mCamWidth;
    private int mCamHeight;
    private boolean mDraw;
    private long mTimestamp;
    
    private float[] mCamPoseMatrix;
    private float[] mProjMatrix;
    private float[] mCamViewMatrix;
    
    private GLSurfaceView mView;
    private TexturedCube mCube;
    private Callback mListener;
    
    private long mStartTime;
    private int mFramesRendered;
    private long mDurationSum;
    private static final int MAXFRAMES = 10; 

    //Camera Intrinsic parameter matrix (12MP SONY)
    private static final float[] K = { 267.27434f,        0.00f,   158.823f,
                                            0.00f, 269.2575423f, 122.68925f,
                                            0.00f,        0.00f,        1.00f};
    
    private static final float FRUSTUM_NEAR_Z = 0.01f;
    private static final float FRUSTUM_FAR_Z = 200.0f;
    
    private static final String TAG = "ARCamera";
    private int context;
    
    static {
        System.loadLibrary("arcamera_jni");
    }
}
