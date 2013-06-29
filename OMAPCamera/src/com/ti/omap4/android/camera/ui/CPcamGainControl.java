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
import android.view.View;
import android.widget.ImageView;
import android.widget.RelativeLayout;
import android.util.Log;
/**
 * A view that contains camera gain control which could adjust the gain in/out
 * if the camera supports gaining.
 */
public abstract class CPcamGainControl extends RelativeLayout implements Rotatable {
    // The states of gain button.
    public static final int GAIN_IN = 0;
    public static final int GAIN_OUT = 1;
    public static final int GAIN_STOP = 2;

    private static final String TAG = "CPcamGainControl";
    private static final int SCALING_INTERVAL = 1000; // milliseconds

    protected ImageView mGainIn;
    protected ImageView mGainOut;
    protected ImageView mGainSlider;
    protected int mOrientation;
    private Handler mHandler;

    public interface OnGainChangedListener {
        void onGainValueChanged(int index);
    }

    // The interface OnGainIndexChangedListener is used to inform the
    // GainIndexBar about the gain index change. The index position is between
    // 0 (the index is zero) and 1.0 (the index is mGainMax).
    public interface OnGainIndexChangedListener {
        void onGainIndexChanged(double indexPosition);
    }

    protected int mGainMax, mGainMin,mGainIndex;
    private OnGainChangedListener mListener;
    private OnGainIndexChangedListener mIndexListener;

    protected OnIndicatorEventListener mOnIndicatorEventListener;
    private int mState;
    private int mStep;

    protected final Runnable mRunnable = new Runnable() {
        public void run() {
            performGain(mState, false);
        }
    };

    public CPcamGainControl(Context context, AttributeSet attrs) {
        super(context, attrs);
        mGainIn = addImageView(context, R.drawable.ic_cpcam_gain_in);
        mGainSlider = addImageView(context, R.drawable.ic_cpcam_gain_slider);
        mGainOut = addImageView(context, R.drawable.ic_cpcam_gain_out);
        mHandler = new Handler();
    }

    public void startGainControl() {
        mGainSlider.setPressed(true);
        setGainIndex(mGainIndex); // Update the gain index bar.
    }

    protected ImageView addImageView(Context context, int iconResourceId) {
        ImageView image = new RotateImageView(context);
        image.setImageResource(iconResourceId);
        if (iconResourceId == R.drawable.ic_cpcam_gain_slider) {
            image.setContentDescription(getResources().getString(
                    R.string.accessibility_cpcam_gain_control));
        } else {
            image.setContentDescription(getResources().getString(
                    R.string.empty));
        }
        addView(image);
        return image;
    }

    public void closeGainControl() {
        mGainSlider.setPressed(false);
        if (mOnIndicatorEventListener != null) {
            mOnIndicatorEventListener.onIndicatorEvent(
                    OnIndicatorEventListener.EVENT_LEAVE_GAIN_CONTROL);
        }
    }

    public void setGainMinMax(int gainMin, int gainMax) {
        mGainMax = gainMax;
        mGainMin = gainMin;
        // Layout should be requested as the maximum gain level is the key to
        // show the correct gain slider position.
        requestLayout();
    }

    public void setOnGainChangeListener(OnGainChangedListener listener) {
        mListener = listener;
    }

    public void setOnIndicatorEventListener(OnIndicatorEventListener listener) {
        mOnIndicatorEventListener = listener;
    }

    public void setGainIndex(int index) {
        if (index < mGainMin || index > mGainMax) {
            throw new IllegalArgumentException("Invalid gain value:" + index);
        }
        mGainIndex = index;
        invalidate();
    }

    private boolean gainIn() {
        return (mGainIndex == mGainMax) ? false : changeGainIndex(mGainIndex + mStep);
    }

    private boolean gainOut() {
        return (mGainIndex == mGainMin) ? false : changeGainIndex(mGainIndex - mStep);
    }

    public void setGainStep(int step) {
        mStep = step;
    }

    // Called from GainControlWheel to change the gain level.
    // TODO: merge the gain control for both platforms.
    protected void performGain(int state) {
        performGain(state, true);
    }

    private void performGain(int state, boolean fromUser) {
        if ((mState == state) && fromUser) return;
        if (fromUser) mHandler.removeCallbacks(mRunnable);
        mState = state;
        switch (state) {
            case GAIN_IN:
                gainIn();
                break;
            case GAIN_OUT:
                gainOut();
                break;
            case GAIN_STOP:
                break;
        }
    }

    // Called from CPCamGainControlBar to change the gain level.
    protected void performGain(double gainPercentage) {
        int index = (int) (mGainMax * gainPercentage);
        if (mGainIndex == index) return;
        changeGainIndex(index);
   }

    private boolean changeGainIndex(int index) {
        if (mListener != null) {
            if (index > mGainMax) index = mGainMax;
            if (index < mGainMin) index = mGainMin;
            mListener.onGainValueChanged(index);
            mGainIndex = index;
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
        mGainIn.setActivated(activated);
        mGainOut.setActivated(activated);
    }
}
