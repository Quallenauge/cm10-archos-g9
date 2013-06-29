/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.ti.omap4.android.camera;

import android.graphics.Matrix;
import android.graphics.Rect;
import android.graphics.RectF;
import android.hardware.Camera.Area;
import android.util.Log;
import android.view.View;
import android.view.MotionEvent;
import android.view.ViewGroup;
import android.widget.RelativeLayout;

import java.util.ArrayList;
import java.util.List;

// A class that handles the metering area
public class TouchManager {
    private static final String TAG = "TouchManager";

    private boolean mInitialized;
    private View mPreviewFrame;
    private List<Area> mMeteringArea; // metering area in driver format
    private int mTouchWidth;
    private int mTouchHeight;
    private Matrix mMatrix;
    Listener mListener;
    private String mPreviewLayout;

    public interface Listener {
        public void setTouchParameters();
    }

    public TouchManager() {
        mMatrix = new Matrix();
    }

    public void initialize(int touchWidth, int touchHeight, View previewFrame,
            Listener listener, boolean mirror, int displayOrientation) {

        mTouchWidth     = touchWidth    ;
        mTouchHeight    = touchHeight   ;
        mPreviewFrame   = previewFrame  ;
        mListener       = listener      ;

        Matrix matrix = new Matrix();

        Util.prepareMatrix(matrix, mirror, displayOrientation,
                previewFrame.getWidth(), previewFrame.getHeight());
        // In face detection, the matrix converts the driver coordinates to UI
        // coordinates. When touching, the inverted matrix converts the UI
        // coordinates to driver coordinates.
        matrix.invert(mMatrix);

        mInitialized = true;
    }

    public void calculateTapArea(int touchWidth, int touchHeight, float areaMultiple,
            int x, int y, int previewWidth, int previewHeight, Rect rect) {

        int areaWidth = (int)(touchWidth * areaMultiple);
        int areaHeight = (int)(touchHeight * areaMultiple);
        int left = Util.clamp(x - areaWidth / 2, 0, previewWidth - areaWidth);
        int top = Util.clamp(y - areaHeight / 2, 0, previewHeight - areaHeight);

        RectF rectF = new RectF(left, top, left + areaWidth, top + areaHeight);
        mMatrix.mapRect(rectF);
        Util.rectFToRect(rectF, rect);
    }

    public boolean onTouch(MotionEvent e) {

        // Initialize variables.
        int x = Math.round(e.getX());
        int y = Math.round(e.getY());
        int previewWidth = mPreviewFrame.getWidth();
        int previewHeight = mPreviewFrame.getHeight();
        if (mMeteringArea == null) {
            mMeteringArea = new ArrayList<Area>();
            mMeteringArea.add(new Area(new Rect(), 1));
        }

        // Convert the coordinates to driver format.
        if (CameraSettings.SS_FULL_S3D_LAYOUT.equals(mPreviewLayout)) {
            calculateTapArea(mTouchWidth, mTouchHeight * 2, 1, x, y, previewWidth, previewHeight,
                    mMeteringArea.get(0).rect);
        } else if (CameraSettings.TB_FULL_S3D_LAYOUT.equals(mPreviewLayout)) {
            calculateTapArea(mTouchWidth * 2, mTouchHeight, 1, x, y, previewWidth, previewHeight,
                    mMeteringArea.get(0).rect);
        } else {
            calculateTapArea(mTouchWidth, mTouchHeight, 1, x, y, previewWidth, previewHeight,
                    mMeteringArea.get(0).rect);
        }
        // Set the metering area.
        mListener.setTouchParameters();

        return true;
    }

    public boolean onTouch(MotionEvent e, String previewLayout) {
        float x = e.getX();
        float y = e.getY();
        mPreviewLayout = previewLayout;
        int previewWidth = mPreviewFrame.getWidth();
        int previewHeight = mPreviewFrame.getHeight();
        if (CameraSettings.SS_FULL_S3D_LAYOUT.equals(previewLayout)) {
            if (y < previewHeight / 2) {
                return onTouch(e);
            } else {
                e.setLocation(x, y - previewHeight / 2);
                return onTouch(e);
            }
        } else if (CameraSettings.TB_FULL_S3D_LAYOUT.equals(previewLayout)) {
            if (x < previewWidth / 2) {
                return onTouch(e);
            } else {
                e.setLocation(x - previewWidth / 2, y);
                return onTouch(e);
            }
        } else if (CameraSettings.TB_SUB_S3D_LAYOUT.equals(previewLayout)) {
            if (x < previewWidth / 2) {
                e.setLocation(x * 2, y);
                return onTouch(e);
            } else {
                e.setLocation((x - previewWidth / 2) * 2, y);
                return onTouch(e);
            }
        } else if (CameraSettings.SS_SUB_S3D_LAYOUT.equals(previewLayout)) {
            if (y < previewHeight / 2) {
                e.setLocation(x, y * 2);
                return onTouch(e);
            } else {
                e.setLocation(x, (y - previewHeight / 2) * 2);
                return onTouch(e);
            }
        } else {
            return onTouch(e);
        }
    }

    public List<Area> getMeteringAreas() {
        return mMeteringArea;
    }

}
