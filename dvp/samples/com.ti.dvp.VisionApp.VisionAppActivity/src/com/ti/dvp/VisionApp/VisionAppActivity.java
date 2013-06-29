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

package com.ti.dvp.VisionApp;

import com.ti.dvp.VisionEngine;
import com.ti.dvp.VisionEngine.GraphCompletedListener;

import android.app.Activity;
//import android.content.Context;
import android.graphics.ImageFormat;
import android.graphics.SurfaceTexture;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.util.Log;
//import android.view.KeyEvent;
//import android.view.MotionEvent;
import android.view.Window;
import android.view.WindowManager;

public class VisionAppActivity extends Activity implements MachineVisionView.onViewSetupListener{
    static {
        System.loadLibrary("dvp_jni"); // this will cause the JNI layer to be loaded and run-time linked.
    }

    private VisionEngine m_engine;

    private static final String TAG = "VisionAppActivity";
    private GLSurfaceView m_mvView;

    public VisionAppActivity() {
    	Log.d(TAG, "VisionAppActivity()");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // Hide window title.
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);

        m_mvView = new MachineVisionView(this, this);
        setContentView(m_mvView);
        m_mvView.requestFocus();
        m_mvView.setFocusableInTouchMode(true);
        
        m_engine = new VisionEngine((GraphCompletedListener)m_mvView, true);
    }

    @Override
    protected void onStart() {
        super.onStart();
        
    }

    @Override
    protected void onResume() {
        super.onResume();
        m_mvView.onResume();
        m_engine.GraphSetup(VisionEngine.GRAPH_DEFAULT, 320, 240, ImageFormat.YUY2);
        m_engine.Start();
    }

    @Override
    protected void onPause() {
    	m_engine.Stop();
    	m_engine.GraphTeardown();
        m_mvView.onPause();
        super.onPause();
    }

    @Override
    protected void onStop() {
        super.onStop();
    }

    @Override
    protected void onRestart() {
        super.onRestart();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        m_engine.destroy();
    }

	public void onViewSetupComplete(SurfaceTexture texture) {
		m_engine.setPreviewSurfaceTexture(texture);		
	}
}



