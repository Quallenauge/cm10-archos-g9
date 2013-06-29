/*
 * Copyright (C) 2008 The Android Open Source Project
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

import com.ti.omap4.android.camera.Camera;
import com.ti.omap4.android.camera.R;

import android.app.Dialog;
import android.opengl.Visibility;
import android.os.Bundle;
import android.content.Context;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.SeekBar;
import android.widget.TextView;

public class ManualGainExposureSettings extends Dialog {
    // 3D Variables
    private View manualGainExposurePanel;
    private SeekBar manualExposureControlLeft, manualExposureControlRight, manualGainControlLeft, manualGainControlRight;
    private TextView manualExposureCaptionLeft, manualExposureCaptionRight, manualGainCaptionLeft, manualGainCaptionRight;
    private int mManualExposureValueLeft, mManualExposureValueRight;
    private Handler cameraHandler;
    private int mProgressBarExposureValueLeft, mProgressBarExposureValueRight;
    private int mProgressBarGainValueLeft, mProgressBarGainValueRight;
    private int mManualGainValueLeft, mManualGainValueRight;
    private int mMinIso, mMaxIso;
    private int mExpRightValue,mExpLeftValue,mIsoRightValue,mIsoLeftValue;

    // 2D Variables
    private int mManualExposureValue,mManualGainValue;
    private int mExposureValue, mISOValue;
    private SeekBar manualExposureControl, manualGainControl;
    private TextView manualExposureCaption, manualGainCaption;
    private int mProgressBarExposureValue, mProgressBarGainValue;

    private Bundle data;

    // 3D Constructor
    public ManualGainExposureSettings (final Context context, final Handler handler,
            int expRightValue, int expLeftValue,
            int isoRightValue, int isoLeftValue,
            int minExposure, int maxExposure,
            int minIso, int maxIso,
            int stepExposure, int stepIso) {
        super(context);
        requestWindowFeature(Window.FEATURE_NO_TITLE);

        // Given default values
        mManualExposureValueLeft = expLeftValue;
        mManualExposureValueRight = expRightValue;
        mManualGainValueLeft = isoLeftValue;
        mManualGainValueRight = isoRightValue;
        mMinIso = minIso;
        mMaxIso = maxIso;
        mExpRightValue = expRightValue;
        mExpLeftValue = expLeftValue;
        mIsoRightValue = isoRightValue;
        mIsoLeftValue = isoLeftValue;
        cameraHandler = handler;
        setContentView(R.layout.manual_gain_exposure_slider_control);

        // Views handlers
        manualGainExposurePanel = findViewById(R.id.panel);

        manualExposureControlLeft = (SeekBar) findViewById(R.id.exposure_seek_left);
        manualExposureControlRight = (SeekBar) findViewById(R.id.exposure_seek_right);
        manualGainControlLeft = (SeekBar) findViewById(R.id.gain_seek_left);
        manualGainControlRight = (SeekBar) findViewById(R.id.gain_seek_right);

        manualExposureControlLeft.setMax(maxExposure-1);
        manualExposureControlRight.setMax(maxExposure-1);
        manualGainControlRight.setMax(maxIso-mMinIso);
        manualGainControlLeft.setMax(maxIso-mMinIso);

        manualExposureControlLeft.setKeyProgressIncrement(stepExposure);
        manualExposureControlRight.setKeyProgressIncrement(stepExposure);
        manualGainControlLeft.setKeyProgressIncrement(stepIso);
        manualGainControlRight.setKeyProgressIncrement(stepIso);

        manualExposureCaptionLeft = (TextView) findViewById(R.id.exposure_caption_left);
        manualExposureCaptionRight = (TextView) findViewById(R.id.exposure_caption_right);
        manualGainCaptionLeft = (TextView) findViewById(R.id.gain_caption_left);
        manualGainCaptionRight = (TextView) findViewById(R.id.gain_caption_right);

        String sCaptionExposureLeft = context.getString(R.string.settings_manual_3d_exposure_left_caption);
        sCaptionExposureLeft = sCaptionExposureLeft + " " + Integer.toString(mManualExposureValueLeft);
        manualExposureCaptionLeft.setText(sCaptionExposureLeft);
        mProgressBarExposureValueLeft = mManualExposureValueLeft;
        manualExposureControlLeft.setProgress(mProgressBarExposureValueLeft);

        String sCaptionExposureRight = context.getString(R.string.settings_manual_3d_exposure_right_caption);
        sCaptionExposureRight = sCaptionExposureRight + " " + Integer.toString(mManualExposureValueRight);
        manualExposureCaptionRight.setText(sCaptionExposureRight);
        mProgressBarExposureValueRight = mManualExposureValueRight;
        manualExposureControlRight.setProgress(mProgressBarExposureValueRight);

        String sCaptionGainLeft = context.getString(R.string.settings_manual_3d_gain_left_caption);
        sCaptionGainLeft = sCaptionGainLeft + " " + Integer.toString(mManualGainValueLeft);
        manualGainCaptionLeft.setText(sCaptionGainLeft);
        mProgressBarGainValueLeft = mManualGainValueLeft - mMinIso;
        manualGainControlLeft.setProgress(mProgressBarGainValueLeft);

        String sCaptionGainRight = context.getString(R.string.settings_manual_3d_gain_right_caption);
        sCaptionGainRight= sCaptionGainRight+ " " + Integer.toString(mManualGainValueRight);
        manualGainCaptionRight.setText(sCaptionGainRight);
        mProgressBarGainValueRight= mManualGainValueRight - mMinIso;
        manualGainControlRight.setProgress(mProgressBarGainValueRight);

        manualGainExposurePanel.setVisibility(View.VISIBLE);

        Button btn = (Button) findViewById(R.id.buttonOK);
        btn.setOnClickListener(new View.OnClickListener() {
                public void onClick(View v) {
                Message msg = new Message();

                data = new Bundle ();
                if (mManualExposureValueLeft != mExpLeftValue) {
                    data.putInt("EXPOSURE_LEFT", mManualExposureValueLeft);
                } else {
                    data.putInt("EXPOSURE_LEFT", mExpLeftValue);
                }
                if (mManualExposureValueRight != mExpRightValue) {
                    data.putInt("EXPOSURE_RIGHT", mManualExposureValueRight);
                } else {
                    data.putInt("EXPOSURE_RIGHT", mExpRightValue);
                }
                if (mManualGainValueLeft != mIsoLeftValue) {
                    data.putInt("ISO_LEFT", mManualGainValueLeft + mMinIso);
                } else {
                    data.putInt("ISO_LEFT", mIsoLeftValue);
                }
                if (mManualGainValueRight != mIsoRightValue) {
                    data.putInt("ISO_RIGHT", mManualGainValueRight +mMinIso);
                } else {
                    data.putInt("ISO_RIGHT", mIsoRightValue);
                }

                msg.what = Camera.MANUAL_GAIN_EXPOSURE_CHANGED;
                msg.setData(data);
                cameraHandler.sendMessage(msg);
                dismiss();
                }
        });

        manualExposureControlLeft.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
                public void onProgressChanged(SeekBar seekBar,
                    int progress, boolean fromUser) {
                        manualExposureCaptionLeft = (TextView) findViewById(R.id.exposure_caption_left);
                        mManualExposureValueLeft = progress+1;
                        String sCaptionLeft = context.getString(R.string.settings_manual_3d_exposure_left_caption);
                        sCaptionLeft = sCaptionLeft + " " + Integer.toString(mManualExposureValueLeft);
                        manualExposureCaptionLeft.setText(sCaptionLeft);
                    }
            public void onStartTrackingTouch(SeekBar seekBar) {
            }
            public void onStopTrackingTouch(SeekBar seekBar) {
            }
        });

        manualExposureControlRight.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
                public void onProgressChanged(SeekBar seekBar,
                    int progress, boolean fromUser) {
                        manualExposureCaptionRight = (TextView) findViewById(R.id.exposure_caption_right);
                        mManualExposureValueRight = progress+1;
                        String sCaptionRight = context.getString(R.string.settings_manual_3d_exposure_right_caption);
                        sCaptionRight = sCaptionRight + " " + Integer.toString(mManualExposureValueRight);
                        manualExposureCaptionRight.setText(sCaptionRight);
                    }
            public void onStartTrackingTouch(SeekBar seekBar) {
            }
            public void onStopTrackingTouch(SeekBar seekBar) {
            }
        });
        manualGainControlLeft.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
                public void onProgressChanged(SeekBar seekBar,
                    int progress, boolean fromUser) {
                manualGainCaptionLeft = (TextView) findViewById(R.id.gain_caption_left);
                mManualGainValueLeft = progress;
                String sCaptionLeft = context.getString(R.string.settings_manual_3d_gain_left_caption);
                sCaptionLeft = sCaptionLeft + " " + Integer.toString(mManualGainValueLeft + mMinIso);
                manualGainCaptionLeft.setText(sCaptionLeft);
            }
            public void onStartTrackingTouch(SeekBar seekBar) {
            }
            public void onStopTrackingTouch(SeekBar seekBar) {
            }
        });

        manualGainControlRight.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
                public void onProgressChanged(SeekBar seekBar,
                    int progress, boolean fromUser) {
                manualGainCaptionRight= (TextView) findViewById(R.id.gain_caption_right);
                mManualGainValueRight= progress;
                String sCaptionRight= context.getString(R.string.settings_manual_3d_gain_right_caption);
                sCaptionRight= sCaptionRight+ " " + Integer.toString(mManualGainValueRight + mMinIso);
                manualGainCaptionRight.setText(sCaptionRight);
            }
            public void onStartTrackingTouch(SeekBar seekBar) {
            }
            public void onStopTrackingTouch(SeekBar seekBar) {
            }
        });
        manualGainCaptionLeft.setVisibility(View.VISIBLE);
        manualGainCaptionRight.setVisibility(View.VISIBLE);
        manualGainControlRight.setVisibility(View.VISIBLE);
        manualGainControlLeft.setVisibility(View.VISIBLE);
        manualExposureControlLeft.setVisibility(View.VISIBLE);
        manualExposureControlRight.setVisibility(View.VISIBLE);
        manualExposureCaptionLeft.setVisibility(View.VISIBLE);
        manualExposureCaptionRight.setVisibility(View.VISIBLE);
    }

    // 2D Constructor
    public ManualGainExposureSettings (final Context context, final Handler handler,
            int expValue, int isoValue,
            int minExposure, int maxExposure,
            int minIso, int maxIso,
            int stepExposure, int stepIso) {
        super(context);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        // Given default values
        mManualExposureValue = expValue;
        mManualGainValue = isoValue;
        mMinIso = minIso;
        mMaxIso = maxIso;
        mExposureValue = expValue;
        mISOValue = isoValue;
        cameraHandler = handler;
        setContentView(R.layout.manual_gain_exposure_slider_control);

        // Views handlers
        manualGainExposurePanel = findViewById(R.id.panel);

        manualExposureControl = (SeekBar) findViewById(R.id.exposure_seek);
        manualGainControl = (SeekBar) findViewById(R.id.gain_seek);

        manualExposureControl.setMax(maxExposure-1);
        manualGainControl.setMax(maxIso-mMinIso);

        manualExposureControl.setKeyProgressIncrement(stepExposure);
        manualGainControl.setKeyProgressIncrement(stepIso);

        manualExposureCaption = (TextView) findViewById(R.id.exposure_caption);
        manualGainCaption = (TextView) findViewById(R.id.gain_caption);

        String sCaptionExposure = context.getString(R.string.settings_manual_2d_exposure_caption);
        sCaptionExposure = sCaptionExposure + " " + Integer.toString(mManualExposureValue);
        manualExposureCaption.setText(sCaptionExposure);
        mProgressBarExposureValue = mManualExposureValue;
        manualExposureControl.setProgress(mProgressBarExposureValue);

        String sCaptionGain = context.getString(R.string.settings_manual_2d_gain_caption);
        sCaptionGain = sCaptionGain + " " + Integer.toString(mManualGainValue);
        manualGainCaption.setText(sCaptionGain);
        mProgressBarGainValue = mManualGainValue - mMinIso;
        manualGainControl.setProgress(mProgressBarGainValue);

        manualGainExposurePanel.setVisibility(View.VISIBLE);

        Button btn = (Button) findViewById(R.id.buttonOK);
        btn.setOnClickListener(new View.OnClickListener() {
                public void onClick(View v) {
                Message msg = new Message();

                data = new Bundle ();
                if (mManualExposureValue != mExposureValue) {
                    data.putInt("EXPOSURE", mManualExposureValue);
                } else {
                    data.putInt("EXPOSURE", mExposureValue);
                }
                if (mManualGainValue != mISOValue) {
                    data.putInt("ISO", mManualGainValue + mMinIso);
                } else {
                    data.putInt("ISO", mISOValue);
                }

                msg.what = Camera.MANUAL_GAIN_EXPOSURE_CHANGED;
                msg.setData(data);
                cameraHandler.sendMessage(msg);
                dismiss();
                }
        });

        manualExposureControl.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
                public void onProgressChanged(SeekBar seekBar,
                    int progress, boolean fromUser) {
                        Message msg = new Message();
                        data = new Bundle ();
                        manualExposureCaption = (TextView) findViewById(R.id.exposure_caption);
                        mManualExposureValue = progress+1;
                        String sCaptionLeft = context.getString(R.string.settings_manual_2d_exposure_caption);
                        sCaptionLeft = sCaptionLeft + " " + Integer.toString(mManualExposureValue);
                        manualExposureCaption.setText(sCaptionLeft);
//                        if (mManualExposureValue != mExposureValue) {
//                            data.putInt("EXPOSURE", mManualExposureValue);
//                        } else {
//                            data.putInt("EXPOSURE", mExposureValue);
//                        }
//                        msg.what = Camera.MANUAL_GAIN_EXPOSURE_CHANGED;
//                        msg.setData(data);
//                        cameraHandler.sendMessage(msg);
                    }
            public void onStartTrackingTouch(SeekBar seekBar) {
            }
            public void onStopTrackingTouch(SeekBar seekBar) {
            }
        });

        manualGainControl.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
                public void onProgressChanged(SeekBar seekBar,
                    int progress, boolean fromUser) {
                Message msg = new Message();
                data = new Bundle ();
                manualGainCaption = (TextView) findViewById(R.id.gain_caption);
                mManualGainValue = progress;
                String sCaptionLeft = context.getString(R.string.settings_manual_2d_gain_caption);
                sCaptionLeft = sCaptionLeft + " " + Integer.toString(mManualGainValue + mMinIso);
                manualGainCaption.setText(sCaptionLeft);
//                if (mManualGainValue != mISOValue) {
//                    data.putInt("ISO", mManualGainValue + mMinIso);
//                } else {
//                    data.putInt("ISO", mISOValue);
//                }
//                msg.what = Camera.MANUAL_GAIN_EXPOSURE_CHANGED;
//                msg.setData(data);
//                cameraHandler.sendMessage(msg);
            }
            public void onStartTrackingTouch(SeekBar seekBar) {
            }
            public void onStopTrackingTouch(SeekBar seekBar) {
            }
        });
        manualGainCaption.setVisibility(View.VISIBLE);
        manualExposureCaption.setVisibility(View.VISIBLE);
        manualGainControl.setVisibility(View.VISIBLE);
        manualExposureControl.setVisibility(View.VISIBLE);
    }
}
