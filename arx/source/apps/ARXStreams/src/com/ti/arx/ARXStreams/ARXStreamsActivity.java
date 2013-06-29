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

package com.ti.arx.ARXStreams;

import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.SurfaceTexture;
import android.os.Bundle;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.TextureView;
import android.widget.ImageView;
import android.widget.LinearLayout;

/*!
 * Renders three buffer streams from the TI Augmented Reality Accelerator
 * to an Android window.
 */
public class ARXStreamsActivity 
    extends Activity 
    implements SurfaceHolder.Callback, TextureView.SurfaceTextureListener
{
    /**
     * Here we initialize the 3 views that will display our ARX buffer streams.
     * 1. We use a SurfaceView to show how an ARX buffer stream can be bound to
     * an Android Surface.
     * 2. We use a TextureView to show how an ARX buffer stream can be bound to
     * an Android SurfaceTexture.
     * 3. We use an ImageView to show how an ARX buffer stream can be manually
     * rendered, for example, to a Bitmap. 
     */
    @Override
    public void onCreate(Bundle savedInstanceState) 
    {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.main);        
        createLayout();
    }
    
    private void createLayout() {
        LinearLayout ll = (LinearLayout)findViewById(R.id.topLayout);
        if (ll != null) {                
            // 1. This view will display the primary preview 
            // stream from ARX.
            camMainView = new SurfaceView(this);
            camMainView.getHolder().addCallback(this);
            ll.addView(camMainView, 640, 480);
            
            // 2. This view will display the secondary preview 
            // stream from ARX (lower resolution, useful
            // for computations).
            camSecView = new TextureView(this);
            camSecView.setSurfaceTextureListener(this);
            ll.addView(camSecView, 320, 240);
                        
            // 3. This view will display the Sobel image stream
            // from ARX.
            sobelView = new ImageView(this);            
            sobelBmp = Bitmap.createBitmap(320, 240, Bitmap.Config.ARGB_8888);
            sobelView.setImageBitmap(sobelBmp);
            ll.addView(sobelView, 320, 240);
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        startARX();
    }

    /**
     * When the activity is paused, we make sure to stop ARX and release
     * resources.
     */
    @Override
    protected void onPause() {
        super.onPause();
        destroyARX(); // Stop ARX and release resources.
    }

    /*************************************************************
     * SurfaceHolder.Callback interface methods
     *************************************************************/
    /*!
     * (non-Javadoc)
     * Store a reference to the Surface into which we will draw the primary
     * preview stream and start ARX if all views are ready.
     * @see android.view.SurfaceHolder.Callback#surfaceCreated(android.view.SurfaceHolder)
     */
    public void surfaceCreated(SurfaceHolder holder) {
        mSurface = holder.getSurface();
        startARX();
    }

    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
    }

    public void surfaceDestroyed(SurfaceHolder holder) {
        mSurface = null;
    }
    
    /*************************************************************
     * TextureView.SurfaceTextureListener interface methods
     *************************************************************/
    /*!
     * (non-Javadoc)
     * Store a reference to the SurfaceTexture into which we will draw the secondary
     * preview stream and start ARX if all views are ready.
     * @see android.view.TextureView.SurfaceTextureListener#onSurfaceTextureAvailable(android.graphics.SurfaceTexture, int, int)
     */
    public void onSurfaceTextureAvailable(SurfaceTexture surface, int width, int height) {
        mSurfaceTex = surface;
        startARX();
    }

    public void onSurfaceTextureSizeChanged(SurfaceTexture surface, int width, int height) {
    }

    public boolean onSurfaceTextureDestroyed(SurfaceTexture surface) {
        mSurfaceTex = null;
        return true;
    }

    public void onSurfaceTextureUpdated(SurfaceTexture surface) {
    }   

    /*!
     * Open and start ARX after configuring the buffer streams we are interested in.
     * @param s a reference to the Surface where we will render the main camera stream.
     * @param st a reference to the SurfaceTexture where we will render the secondary camera stream.
     * @param bitmap a reference to a Bitmap where the sobel data will be copied to.
     */
    public native boolean createARX(Surface s, SurfaceTexture st, Bitmap bitmap);
    
    /*!
     * Stop ARX and release the resources it is using.
     */
    public native void destroyARX();
    
    /*!
     * Start ARX (only if all of our surfaces have been created). 
     */
    private void startARX() {
        if (mSurface != null && mSurfaceTex != null && sobelBmp != null) {
            createARX(mSurface, mSurfaceTex, sobelBmp);
        }
    }
    
    private void onARXDeath() {
        //Restart ARX
        destroyARX();
        Runnable r = new Runnable() {
            public void run() {
                LinearLayout ll = (LinearLayout)findViewById(R.id.topLayout);
                if (ll != null) { 
                    ll.removeAllViews();
                }
                createLayout();
            }
        };
        runOnUiThread(r);       
    }

    private void onBufferChanged() {
        sobelView.postInvalidate();
    }

    private SurfaceView camMainView; // displays main camera stream
    private TextureView camSecView; // displays secondary camera stream
    private ImageView sobelView; // displays Sobel stream
    private Bitmap sobelBmp;
    private Surface mSurface;
    private SurfaceTexture mSurfaceTex;
    
    /* Holds the native context; for JNI use only */
    private int context;

    // Load our JNI code when this class is first referenced.
    static {
        System.loadLibrary("arxstreams_jni");
    }
}
