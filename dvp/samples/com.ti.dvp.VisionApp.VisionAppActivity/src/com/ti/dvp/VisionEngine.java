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

package com.ti.dvp;

import java.io.IOException;
import java.util.List;

import android.graphics.ImageFormat;
//import android.graphics.ImageFormat;
import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.hardware.Camera.CameraInfo;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;

/**
 * A class for receiving the output of the camera
 * after the preview frames have been processed by a DVP graph.
 * The graph is constructed and executed by a JNI library. Access
 * to the camera is implemented either through Android's
 * android.hardware.Camera class or some implementation of VisionCam.
 *
 */
public class VisionEngine implements Camera.PreviewCallback {
    static {
        System.loadLibrary("dvp_jni"); // this will cause the JNI layer to be loaded and run-time linked.
    }

    private static String TAG = "VisionEngine";
    private Camera m_camera;
    private Camera.Parameters m_cparams;
    private int    m_numCameras;
    private int    m_camIdx;
    private int    m_graphEnum;

    private static final int GRAPH_MIN = -1;
    public static final int GRAPH_DEFAULT = 0;
    public static final int GRAPH_EDGE = 1;
    private static final int GRAPH_MAX = 2;

    private int m_width;
    private int m_height;
    private int m_format;

    private GraphCompletedThread m_thread;
    private GraphCompletedListener m_listener;

    /**
     * A dummy surface texture to tell the camera to use
     * for preview because we can't start preview otherwise.
     * The texture given to setPreviewSurfaceTexture
     * is used to preview the output of DVP graph.
     *
     * @see #setPreviewSurfaceTexture(SurfaceTexture)
     */
    private SurfaceTexture m_DummyTexture;

    private Surface m_DummySurface;

    /**
     * The surface the DVP uses to write the output of the DVP graph to.
     */
    private Surface m_DisplaySurface;

    /**
     * If true tells the DVP JNI that it has to use its own camera
     * for registering for preview frames.
     */
    private boolean m_useNativeCamera;

    /**
     * An interface for sending notifications to the users
     * of this class that the execution of the DVP graph has
     * completed.
     *
     */
    public interface GraphCompletedListener {
        public void onGraphCompleted();
    }

    /**
     * Class constructor.
     *
     * @param listener        the object that has to be notified of graph completion
     * @param useNativeCamera if true an implementation of VisionCam, otherwise
     *                        android.hardware.Camera is used.
     */
    public VisionEngine(GraphCompletedListener listener, boolean useNativeCamera)
    {
        m_camera = null;
        m_useNativeCamera = useNativeCamera;

        m_listener = listener;
        handle = InitNative(m_useNativeCamera);

        m_DummyTexture = new SurfaceTexture(1234);
        m_DummySurface = new Surface(m_DummyTexture);
        
    }

    public void destroy()
    {
        GraphTeardown();
        FinalizeNative(handle);
        if (m_camera != null) {
            m_camera.release();
        }
        m_DummyTexture.release();
        m_DummySurface.release();
        m_DisplaySurface.release();
    }

    protected void finalize()
    {
        destroy();
    }

    /**
     * Implementation of Camera.PreviewCallback method
     */
    public void onPreviewFrame(byte[] data, Camera camera) {
        Log.d(TAG, "Calling GraphUpdate!");
        GraphUpdate(data, m_width, m_height, m_format);
    }

    /**
     * Tells the camera where to render the preview to.
     *
     * @param surface the SurfaceHolder object used for the preview.
     */
    public void setPreviewSurface(SurfaceHolder surface) {
        if (m_camera != null) {
            try {
                m_camera.setPreviewDisplay(surface);
            } catch (IOException io) {}
        }
    }

    /**
     * Tells the camera where to render the preview to.
     *
     * @param texture the SurfaceTexture object used for preview.
     */
    public void setPreviewSurfaceTexture(SurfaceTexture texture) {
        if (m_camera != null) {
            try {
                m_camera.setPreviewTexture(m_DummyTexture);
            } catch (IOException io) {}
        }

        m_DisplaySurface = new Surface(texture);
        SetPreviewSurfaceNative(m_DisplaySurface, m_width, m_height, 0x100); //NV12 format

        if (m_useNativeCamera)
        {
            SelectCameraNative(0);
            DumpCameraParametersNative();

            m_DummyTexture.setDefaultBufferSize(m_width, m_height);
            StartPreviewNative(m_DummyTexture);
        }
    }

    /**
     * Initializes Android's camera.
     *
     * @return true if camera has been initialized successfully or <br>
     *         false if no camera is found or can't be initialized
     */
    public boolean CameraInit()
    {
        Log.d(TAG, "CameraInit()");
        try {
            CameraInfo info = new Camera.CameraInfo();

            m_camera = null;
            for (m_camIdx = 0; m_camIdx < Camera.getNumberOfCameras(); m_camIdx++)
            {
                Camera.getCameraInfo(m_camIdx, info);
                Log.d(TAG, "Camera: "+m_camIdx+" is a "+info.facing+" facing camera");
                if (CameraInfo.CAMERA_FACING_FRONT == info.facing)
                {
                    m_camera = Camera.open(m_camIdx);
                    break;
                }
            }
        } catch (RuntimeException re) {
            Log.e(TAG, "Failed to connect to camera!");
            m_camera = null;
        }
        if (m_camera == null)
        {
            Log.e(TAG, "Could not find a front camera to open!");
            return false;
        }
        Log.d(TAG, "Opened Camera!");

        // get the buffers from the lower layers
        m_camera.setPreviewCallback(this); // @TODO add with Buffers later
//      m_camera.setPreviewCallbackWithBuffer(this);
//      m_camera.addCallbackBuffer(GetBuffersNative(0));

        m_cparams = m_camera.getParameters();
        if (m_cparams != null)
        {
//          m_cparams.setExposureCompensation(m_cparams.getMinExposureCompensation());
            m_cparams.setFocusMode("auto");
            m_cparams.setPreviewSize(m_width, m_height);
            m_cparams.setPreviewFormat(m_format);
            List<int[]> range = m_cparams.getSupportedPreviewFpsRange();
            for (int i = 0; i < range.size(); i++)
            {
                int[] subrange = range.get(i);
                for (int j = 0; j < subrange.length; j++)
                    Log.d(TAG, "FPS["+i+"]["+j+"]: "+subrange[j]+"");
            }
            m_cparams.setPreviewFpsRange(24*1000, 24*1000);
            m_camera.setParameters(m_cparams);
            return true;
        }
        else
            return false;
    }

    public boolean GraphSetup(int gEnum, int width, int height, int format)
    {
        if (GRAPH_MIN < gEnum && gEnum < GRAPH_MAX)
        {
            boolean ret = false;
            m_width = width;
            m_height = height;
            m_format = format;
            m_graphEnum = gEnum;

            ret = GraphSetupNative(handle, m_graphEnum, m_width, m_height, m_format);
            if (ret == true && !m_useNativeCamera)
                ret = CameraInit();
            return ret;
        }
        else
            return false;
    }

    public boolean GraphUpdate(byte[] buffer, int width, int height, int format)
    {
        Log.d(TAG, "Calling GraphUpdateNative for "+width+","+height+","+format);
        return GraphUpdateNative(handle, graphEnum, buffer, width, height, format);
    }

    public boolean GraphTeardown()
    {
        return GraphTeardownNative(handle, m_graphEnum);
    }

    public void WaitForComplete()
    {
        WaitForCompleteNative();
    }

    public void CancelWaitForComplete()
    {
        CancelWaitForCompleteNative();
    }

    public void Start() {
        if (m_camera != null) {
            m_camera.startPreview();
        }
//          if (m_useNativeCamera)
//          {
//          m_DummyTexture.setDefaultBufferSize(m_width, m_height);
//          StartPreviewNative(m_DummyTexture);
//          }
        StartNotification();
    }

    public void Stop() {
        StopNotification();
        if (m_useNativeCamera) {
            StopPreviewNative();
        }
        if (m_camera != null) {
            m_camera.stopPreview();
        }
    }

    /**
     * Creates an object of GraphCompletedThread
     * and starts its thread.
     */
    private void StartNotification() {
        m_thread = new GraphCompletedThread();
        m_thread.setRunning(true);
        m_thread.start();
    }

    /**
     * Stops the thread running in GraphCompletedThread
     * object.
     */
    private void StopNotification() {
        m_thread.setRunning(false);
        CancelWaitForComplete();
        try {
            m_thread.join();
        } catch (InterruptedException e) {
            // we will try it again and again...
        }
    }

    /**
     * This class runs a thread that waits the DVP
     * graph to completion and notifies the subscribers
     * to GraphCompletedListener.
     *
     */
    private class GraphCompletedThread extends Thread {
        private boolean m_running = false;

        public GraphCompletedThread() {
        }

        public void setRunning(boolean running) {
            m_running = running;
        }

        @Override
        public void run() {
            while (m_running) {
                WaitForComplete();
                if (m_running) {
//                  m_camera.addCallbackBuffer(GetBuffersNative(0));
                    m_listener.onGraphCompleted();
                }
            }
        }
    }

    // PRIVATE NATIVE INTERFACES

    private int handle;
    private int graphEnum;

    /**
     * Initializes the JNI library and VisionCam if necessary.
     * Postcondition of this method is {@link #FinalizeNative(int)}.
     *
     * @param  initCamera true if VisionCam should be used as source or <br>
     *                    false if Android's camera is used
     * @return an integer type handle to the created DVP graph
     */
    private native int InitNative(boolean initCamera);

    /**
     * Releases resources held by the JNI library.
     * Precondition to this method is {@link #InitNative(boolean)}.
     * Precondition to this method is {@link #GraphTeardownNative(int, int)}.
     *
     * @param handle handle specifying the graph.
     */
    private native void FinalizeNative(int handle);

    /**
     * Constructs a DVP graph.
     * Precondition to this method is {@link #InitNative(boolean)}.
     * Postcondition of this method is {@link #GraphTeardownNative(int, int)}.
     *
     * @param handle    handle identifying an already allocated graph.
     * @param graphEnum what type of graph. Currently not used.
     * @param width     width of the image buffers used by the graph.
     * @param height    height of the image buffers used by the graph.
     * @param format    image format of the preview buffers returned by camera.
     *                  For example ImageFormat.YUY2 or ImageFormat.NV12.
     * @return          true if the graph is constructed successfully or <br>
     *                  false if errors are found.
     */
    private native boolean GraphSetupNative(int handle, int graphEnum, int width, int height, int format);

    /**
     * Updates the graph with last received image. Used only with Android's camera.
     * Precondition to this method is {@link #InitNative(boolean)}.
     * Precondition to this method is {@link #GraphSetupNative(int, int, int, int, int)}.
     *
     * @param handle    handle identifying an already allocated graph.
     * @param graphEnum what type of graph. Currently not used.
     * @param buffer    the image preview data
     * @param width     width of the preview image.
     * @param height    height of the preview image.
     * @param bpp       how many bytes per pixel has the buffer
     * @return          true
     */
    private native boolean GraphUpdateNative(int handle, int graphEnum, byte[] buffer, int width, int height, int bpp);

    /**
     * Deconstructs a DVP graph.
     * Precondition to this method is {@link #InitNative(boolean)}.
     * Precondition to this method is {@link #GraphSetupNative(int, int, int, int, int)}.
     *
     * @param handle    handle identifying an already allocated graph.
     * @param graphEnum what type of graph. Currently not used.
     * @return          true
     */
    private native boolean GraphTeardownNative(int handle, int graphEnum);

    /**
     * Waits for notification from DVP that graph execution
     * has been completed.
     * Precondition to this method is {@link #InitNative(boolean)}.
     */
    private native void WaitForCompleteNative();

    /**
     * Cancels a call to WaitForCompleteNative.
     * Precondition to this method is {@link #InitNative(boolean)}.
     */
    private native void CancelWaitForCompleteNative();

    /**
     * Prints VisionCam parameters.
     * Precondition to this method is {@link #InitNative(boolean)}.
     */
    private native void DumpCameraParametersNative();

    /**
     * Tells the VisionCam to connect to the specified camera.
     * Precondition to this method is {@link #InitNative(boolean)}.
     *
     * @param id number of the camera to connect to
     * @return   true if connection is established or <br>
     *           false if unable to connect
     */
    private native boolean SelectCameraNative(int id);

    /**
     * Tells the JNI library what Surface (ANativeWindow) to use
     * to write the output of the DVP graph to.
     * Precondition to this method is {@link #InitNative(boolean)}.
     *
     * @param surface a Surface object
     * @param width   with of the surface window
     * @param height  height of the surface window
     * @param format  format of the surface window
     * @return        true if preview is set successfully or <br>
     *                false if not set.
     */
    private native boolean SetPreviewSurfaceNative(Surface surface, int width, int height, int format);

    /**
     * Tells the VisionCam to start preview.
     * Precondition to this method is {@link #InitNative(boolean)}.
     * Precondition to this method is {@link #SelectCameraNative(int)}.
     * Precondition to this method is {@link #SetPreviewSurfaceNative(Surface, int, int, int)}.
     *
     * @param surface a SurfaceTexture object required by CSVisionCam to start preview.
     * @return        true if preview is started successfully or <br>
     *                false if unable to start preview
     */
    private native boolean StartPreviewNative(SurfaceTexture surface);

    /**
     * Tells the VisionCam to stop preview.
     * Precondition to this method is {@link #InitNative(boolean)}.
     *
     * @return true if preview is stopped or <br>
     *         false if errors are found
     */
    private native boolean StopPreviewNative();

    private native byte[] GetBuffersNative(int buffID);
}

