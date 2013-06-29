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

import android.app.Activity;
import android.graphics.PixelFormat;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.LinearLayout;
import android.widget.TextView;

/**
 * @brief
 */
public class ARCowboidActivity extends Activity implements SurfaceHolder.Callback, ARCamera.Callback {
    /**
     * @brief Overridden to create and display an instance of our custom view.
     */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        int w = 1280;
        int h = 960;
        mARCam = new ARCamera(this, w, h);
        mARCam.setCallback(this);

        mCamView = new SurfaceView(this);
        mCamView.getHolder().addCallback(this);
        
        mARView = new GLSurfaceView(this);
        mARView.setZOrderOnTop(true);        
        mARView.setEGLConfigChooser(8, 8, 8, 8, 16, 0);
        mARView.getHolder().setFormat(PixelFormat.TRANSLUCENT);
                        
        mARCam.setGLView(mARView);
        mARView.setRenderer(mARCam);
        mARView.setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
        
        mArLayout = new ARLayout(this, (double)w/(double)h);
        mArLayout.addView(mCamView);
        mArLayout.addView(mARView);
        
        TrackerResetOnClick trackerReset = new TrackerResetOnClick();
        mArLayout.setOnClickListener(trackerReset);

        setContentView(R.layout.main);
        
        mFPSView = (TextView)findViewById(R.id.fpsCounter);
        if (mFPSView != null) {
            mFPSView.setText(FPSLABEL);
        }
        
        LinearLayout root = (LinearLayout)findViewById(R.id.root);
        if (root != null) {
            root.addView(mArLayout);
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (mSurface != null) {
            boolean started = mARCam.start(mSurface);
            if (!started) {
                finish();
            }
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        mARCam.stop();
    }

    public void surfaceCreated(SurfaceHolder holder) {
        mSurface = holder.getSurface();
        boolean started = mARCam.start(mSurface);
        if (!started) {
            finish();
        }
    }

    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {

    }

    public void surfaceDestroyed(SurfaceHolder holder) {
        mSurface = null;
    }

    public void onARCameraDeath() {
        Log.d(TAG, "onARCamDeath");
        mARCam.stop();
        Runnable r = new Runnable() {
            public void run() {
                mCamView = new SurfaceView(ARCowboidActivity.this);
                mCamView.getHolder().addCallback(ARCowboidActivity.this);
                mArLayout.removeAllViews();
                mArLayout.addView(mCamView);
                mArLayout.addView(mARView);
            }
        };
        runOnUiThread(r);
    }
    
    public void onFPSChange(final double fps) {
        Runnable r = new Runnable() {
            public void run() {
                mFPSView.setText(FPSLABEL + String.format("%.1f", fps));
            }
        };
        runOnUiThread(r);
    }

    private class TrackerResetOnClick implements OnClickListener {

        @Override
        public void onClick(View v) {
            ARCamera cam = ARCowboidActivity.this.mARCam;
            if (cam != null) {
                cam.reset();
            }
        }
    }

    private static final String TAG = "ARCowboid";
    private static final String FPSLABEL = "FPS: ";
    private ARLayout mArLayout;
    private SurfaceView mCamView;
    private Surface mSurface;
    private GLSurfaceView mARView;
    private ARCamera mARCam;
    private TextView mFPSView;
}
