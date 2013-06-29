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

import android.content.Context;
import android.os.Handler;
import android.util.AttributeSet;
import android.util.Log;
import android.view.View;
import android.widget.ImageView;
import android.widget.RelativeLayout;

/**
 * A view that contains camera exposure control which could adjust the exposure in/out
 * if the camera supports exposureing.
 */
public abstract class CPcamExposureControl extends RelativeLayout implements Rotatable {
    // The states of exposure button.
    public static final int EXPOSURE_IN = 0;
    public static final int EXPOSURE_OUT = 1;
    public static final int EXPOSURE_STOP = 2;

    private static final String TAG = "CPcamExposureControl";
    private static final int SCALING_INTERVAL = 1000; // milliseconds

    protected ImageView mExposureIn;
    protected ImageView mExposureOut;
    protected ImageView mExposureSlider;
    protected int mOrientation;
    private Handler mHandler;

    public interface OnExposureChangedListener {
        void onExposureValueChanged(int index);
    }

    // The interface OnExposureIndexChangedListener is used to inform the
    // ExposureIndexBar about the exposure index change. The index position is between
    // 0 (the index is zero) and 1.0 (the index is mExposureMax).
    public interface OnExposureIndexChangedListener {
        void onExposureIndexChanged(double indexPosition);
    }

    protected int mExposureMax,mExposureMin, mExposureIndex;
    private OnExposureChangedListener mListener;
    private OnExposureIndexChangedListener mIndexListener;

    protected OnIndicatorEventListener mOnIndicatorEventListener;
    private int mState;
    private int mStep;

    protected final Runnable mRunnable = new Runnable() {
        public void run() {
            performExposure(mState, false);
        }
    };

    public CPcamExposureControl(Context context, AttributeSet attrs) {
        super(context, attrs);
        mExposureIn = addImageView(context, R.drawable.ic_cpcam_exposure_in);
        mExposureSlider = addImageView(context, R.drawable.ic_cpcam_exposure_slider);
        mExposureOut = addImageView(context, R.drawable.ic_cpcam_exposure_out);
        mHandler = new Handler();
    }

    public void startExposureControl() {
        mExposureSlider.setPressed(true);
        setExposureIndex(mExposureIndex); // Update the exposure index bar.
    }

    protected ImageView addImageView(Context context, int iconResourceId) {
        ImageView image = new RotateImageView(context);
        image.setImageResource(iconResourceId);
        if (iconResourceId == R.drawable.ic_cpcam_exposure_slider) {
            image.setContentDescription(getResources().getString(
                    R.string.accessibility_cpcam_exposure_control));
        } else {
            image.setContentDescription(getResources().getString(
                    R.string.empty));
        }
        addView(image);
        return image;
    }

    public void closeExposureControl() {
        mExposureSlider.setPressed(false);
        if (mOnIndicatorEventListener != null) {
            mOnIndicatorEventListener.onIndicatorEvent(
                    OnIndicatorEventListener.EVENT_LEAVE_EXPOSURE_CONTROL);
        }
    }

    public void setExposureMinMax(int exposureMin,int exposureMax) {
        mExposureMax = exposureMax;
        mExposureMin = exposureMin;

        // Layout should be requested as the maximum exposure level is the key to
        // show the correct exposure slider position.
        requestLayout();
    }

    public void setOnExposureChangeListener(OnExposureChangedListener listener) {
        mListener = listener;
    }

    public void setOnIndicatorEventListener(OnIndicatorEventListener listener) {
        mOnIndicatorEventListener = listener;
    }

    public void setExposureIndex(int index) {
        if (index < 0 || index > mExposureMax) {
            throw new IllegalArgumentException("Invalid exposure value:" + index);
        }
        mExposureIndex = index;
        invalidate();
    }

    private boolean exposureIn() {
        return (mExposureIndex == mExposureMax) ? false : changeExposureIndex(mExposureIndex + mStep);
    }

    private boolean exposureOut() {
        return (mExposureIndex == mExposureMin) ? false : changeExposureIndex(mExposureIndex - mStep);
    }

    public void setExposureStep(int step) {
        mStep = step;
    }

    // Called from ExposureControlWheel to change the exposure level.
    // TODO: merge the exposure control for both platforms.
    protected void performExposure(int state) {
        performExposure(state, true);
    }

    private void performExposure(int state, boolean fromUser) {
        if ((mState == state) && fromUser) return;
        if (fromUser) mHandler.removeCallbacks(mRunnable);
        mState = state;
        switch (state) {
            case EXPOSURE_IN:
                exposureIn();
                break;
            case EXPOSURE_OUT:
                exposureOut();
                break;
            case EXPOSURE_STOP:
                break;
        }
    }

    // Called from ExposureControlBar to change the exposure level.
    protected void performExposure(double exposurePercentage) {
        int index = (int) (mExposureMax * exposurePercentage);
        if (mExposureIndex == index) return;
        changeExposureIndex(index);
   }

    private boolean changeExposureIndex(int index) {
        if (mListener != null) {
            if (index > mExposureMax) index = mExposureMax;
            if (index < 0) index = 0;
            mListener.onExposureValueChanged(index);
            mExposureIndex = index;
        }
        return true;
    }

    public void setOrientation(int orientation) {
        mOrientation = orientation;
        int count = getChildCount();
        for (int i = 0 ; i < count ; ++i) {
            View view = getChildAt(i);
            if (view instanceof RotateImageView) {
                ((RotateImageView) view).setOrientation(orientation);
            }
        }
    }

    @Override
    public void setActivated(boolean activated) {
        super.setActivated(activated);
        mExposureIn.setActivated(activated);
        mExposureOut.setActivated(activated);
    }
}
