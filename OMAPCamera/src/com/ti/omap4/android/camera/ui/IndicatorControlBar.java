/*
 * Copyright (C) 2010 The Android Open Source Project
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

package com.ti.omap4.android.camera.ui;

import com.ti.omap4.android.camera.PreferenceGroup;
import com.ti.omap4.android.camera.R;
import com.ti.omap4.android.camera.Util;

import android.content.Context;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.View;
import android.widget.ImageView;
import android.util.Log;
/**
 * A view that contains the top-level indicator control.
 */
public class IndicatorControlBar extends IndicatorControl implements
        View.OnClickListener {
    private static final String TAG = "IndicatorControlBar";

    // Space between indicator icons.
    public static final int ICON_SPACING = Util.dpToPixel(16);

    private ImageView mZoomIcon;
    private ImageView mSecondLevelIcon;
    private ZoomControlBar mZoomControl;
    private CPcamGainControlBar mGainControl;
    private CPcamExposureControlBar mExposureControl;

    public IndicatorControlBar(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    @Override
    protected void onFinishInflate() {
        mSecondLevelIcon = (ImageView)
                findViewById(R.id.second_level_indicator_bar_icon);
        mSecondLevelIcon.setOnClickListener(this);
    }

    public void initialize(Context context, PreferenceGroup group,
            boolean zoomSupported, boolean CPcamSlidersSupported) {
        setPreferenceGroup(group);

        // Add the ZoomControl if supported.
        if (zoomSupported) {
            mZoomControl = (ZoomControlBar) findViewById(R.id.zoom_control);
            mZoomControl.setVisibility(View.VISIBLE);
        }

        if (CPcamSlidersSupported) {
            mGainControl = (CPcamGainControlBar) findViewById(R.id.gain_control);
            mGainControl.setVisibility(View.INVISIBLE);
            mExposureControl = (CPcamExposureControlBar) findViewById(R.id.exposure_control);
            mExposureControl.setVisibility(View.INVISIBLE);
            mSecondLevelIcon.setVisibility(View.INVISIBLE);
        }
        else
        {
            // Add CameraPicker control if not in CPCam mode.
            initializeCameraPicker();
            if (mCameraPicker != null) {
                mCameraPicker.setBackgroundResource(R.drawable.bg_pressed);
            }
        }

        requestLayout();

    }

    public void showCPCamSliders (boolean enabled)
    {
        if (enabled) {
            mGainControl = (CPcamGainControlBar) findViewById(R.id.gain_control);
            mGainControl.setVisibility(View.VISIBLE);
            mExposureControl = (CPcamExposureControlBar) findViewById(R.id.exposure_control);
            mExposureControl.setVisibility(View.VISIBLE);
            requestLayout();
        } else {
            mGainControl = (CPcamGainControlBar) findViewById(R.id.gain_control);
            mGainControl.setVisibility(View.INVISIBLE);
            mExposureControl = (CPcamExposureControlBar) findViewById(R.id.exposure_control);
            mExposureControl.setVisibility(View.INVISIBLE);
            requestLayout();
        }
    }


    @Override
    public boolean dispatchTouchEvent(MotionEvent event) {
        super.dispatchTouchEvent(event);
        // We need to consume the event, or it will trigger tap-to-focus.
        return true;
    }

    public void onClick(View view) {
        dismissSettingPopup();
        // Only for the click on mSecondLevelIcon.
        mOnIndicatorEventListener.onIndicatorEvent(
                OnIndicatorEventListener.EVENT_ENTER_SECOND_LEVEL_INDICATOR_BAR);
    }

    @Override
    protected void onLayout(
            boolean changed, int left, int top, int right, int bottom) {
        int count = getChildCount();
        if (count == 0) return;

        // We have (equal) paddings at left and right, but no padding at top or
        // bottom.
        int padding = getPaddingLeft();
        int width = right - left;
        int height = bottom - top;

        // We want the icons to be square (size x size)
        int size = height;

        mSecondLevelIcon.layout(padding, 0, padding + size, size);

        // Layout the zoom control if required.
        if (mZoomControl != null)  {
            mZoomControl.layout(padding + size, 0, width - padding - size, size);
        }

        if (mGainControl != null)  {
            mGainControl.layout(padding + size, 0, width - padding - size, size);
        }

        if (mExposureControl != null)  {
            mExposureControl.layout(padding + size, (size / 3)*2, width - padding - size, size);
        }

        if (mCameraPicker != null) {
            mCameraPicker.layout(width - padding - size, 0, width - padding, size);
        }
    }

    @Override
    public void setEnabled(boolean enabled) {
        super.setEnabled(enabled);
        if (mGainControl != null || mExposureControl != null) {
            mSecondLevelIcon.setVisibility(View.INVISIBLE);
        } else {
            mSecondLevelIcon.setVisibility(enabled ? View.VISIBLE : View.INVISIBLE);
        }
        if (mCurrentMode != MODE_VIDEO) {
            // We also disable the zoom button during snapshot.
            enableZoom(enabled);
        }
        mSecondLevelIcon.setEnabled(enabled);
    }

    public void enableZoom(boolean enabled) {
        if (mZoomControl != null)  mZoomControl.setEnabled(enabled);
        if (mGainControl != null)  mGainControl.setEnabled(enabled);
        if (mExposureControl != null)  mExposureControl.setEnabled(enabled);
    }
}
