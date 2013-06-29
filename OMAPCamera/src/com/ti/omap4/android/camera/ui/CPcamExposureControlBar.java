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

package com.ti.omap4.android.camera.ui;

import com.ti.omap4.android.camera.R;
import com.ti.omap4.android.camera.Util;

import android.content.Context;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.View;

/**
 * A view that contains camera CPCam exposure control and its layout.
 */
public class CPcamExposureControlBar extends CPcamExposureControl {
    private static final String TAG = "CPcamExposureControlBar";
    private static final int THRESHOLD_FIRST_MOVE = Util.dpToPixel(10); // pixels
    private static final int ICON_SPACING = Util.dpToPixel(12);

    private View mBar;
    private boolean mStartChanging;
    private int mSliderPosition = 0;
    private int mSliderLength;
    private int mWidth;
    private int mIconWidth;
    private int mTotalIconWidth;

    public CPcamExposureControlBar(Context context, AttributeSet attrs) {
        super(context, attrs);
        mBar = new View(context);
        mBar.setBackgroundResource(R.drawable.cpcam_exposure_slider_bar);
        addView(mBar);
    }

    @Override
    public void setActivated(boolean activated) {
        super.setActivated(activated);
        mBar.setActivated(activated);
    }

    private int getSliderPosition(int x) {
        // Calculate the absolute offset of the slider in the exposure control bar.
        // For left-hand users, as the device is rotated for 180 degree for
        // landscape mode, the exposure-in bottom should be on the top, so the
        // position should be reversed.
        int pos; // the relative position in the exposure slider bar
        if (mOrientation == 90) {
            pos = mWidth - mTotalIconWidth - x;
        } else {
            pos = x - mTotalIconWidth;
        }
        if (pos < 0) pos = 0;
        if (pos > mSliderLength) pos = mSliderLength;
        return pos;
    }

    @Override
    protected void onSizeChanged(int w, int h, int oldw, int oldh) {
        mWidth = w;
        mIconWidth = mExposureIn.getMeasuredWidth();
        mTotalIconWidth = mIconWidth + ICON_SPACING;
        mSliderLength = mWidth  - (2 * mTotalIconWidth);
    }

    @Override
    public boolean dispatchTouchEvent(MotionEvent event) {
        if (!isEnabled() || (mWidth == 0)) return false;
        int action = event.getAction();

        switch (action) {
            case MotionEvent.ACTION_OUTSIDE:
            case MotionEvent.ACTION_CANCEL:
            case MotionEvent.ACTION_UP:
                setActivated(false);
                closeExposureControl();
                break;

            case MotionEvent.ACTION_DOWN:
                setActivated(true);
                mStartChanging = false;
            case MotionEvent.ACTION_MOVE:
                int pos = getSliderPosition((int) event.getX());
                if (!mStartChanging) {
                    // Make sure the movement is large enough before we start
                    // changing the exposure.
                    int delta = mSliderPosition - pos;
                    if ((delta > THRESHOLD_FIRST_MOVE) ||
                            (delta < -THRESHOLD_FIRST_MOVE)) {
                        mStartChanging = true;
                    }
                }
                if (mStartChanging) {
                    performExposure(1.0d * pos / mSliderLength);
                    mSliderPosition = pos;
                }
                requestLayout();
        }
        return true;
    }

    @Override
    public void setOrientation(int orientation) {
        // layout for the left-hand camera control
        if ((orientation == 90) || (mOrientation == 90)) requestLayout();
        super.setOrientation(orientation);
    }

    @Override
    protected void onLayout(
            boolean changed, int left, int top, int right, int bottom) {
        if (mExposureMax == 0) return;
        int height = bottom - top;
        mBar.layout(mTotalIconWidth, 0, mWidth - mTotalIconWidth, height);
        // For left-hand users, as the device is rotated for 180 degree,
        // the exposure-in button should be on the top.
        int pos; // slider position
        int sliderPosition;
        if (mSliderPosition != -1) { // -1 means invalid
            sliderPosition = mSliderPosition;
        } else {
            sliderPosition = (int) ((double) mSliderLength * mExposureIndex / mExposureMax);
        }
        if (mOrientation == 90) {
            mExposureIn.layout(0, 0, mIconWidth, height);
            mExposureOut.layout(mWidth - mIconWidth, 0, mWidth, height);
            pos = mBar.getRight() - sliderPosition;
        } else {
            mExposureOut.layout(0, 0, mIconWidth, height);
            mExposureIn.layout(mWidth - mIconWidth, 0, mWidth, height);
            pos = mBar.getLeft() + sliderPosition;
        }
        int sliderWidth = mExposureSlider.getMeasuredWidth();
        mExposureSlider.layout((pos - sliderWidth / 2), 0,
                (pos + sliderWidth / 2), height);
    }

    @Override
    public void setExposureIndex(int index) {
        super.setExposureIndex(index);
        mSliderPosition = -1; // -1 means invalid
        requestLayout();
    }
}
