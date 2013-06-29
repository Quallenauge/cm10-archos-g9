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
import android.os.Bundle;
import android.content.Context;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.View;
import android.view.Window;
import android.widget.Button;
import android.widget.SeekBar;
import android.widget.TextView;


public class ManualConvergenceSettings extends Dialog {

    private View manualConvergencePanel;
    private SeekBar manualConvergenceControl;
    private TextView manualConvergenceCaption;
    private int mManualConvergence;
    private Handler cameraHandler;
    private int mProgressBarConvergenceValue;
    private int mMinConvergence;
    private int mConvergenceValue;

    public ManualConvergenceSettings (final Context context, final Handler handler,
            int convergenceValue,
            int minConvergence, int maxConvergence,
            int stepConvergence){
        super(context);
        requestWindowFeature(Window.FEATURE_NO_TITLE);

        // Given default values
        mManualConvergence = convergenceValue;
        mMinConvergence = minConvergence;
        mConvergenceValue = convergenceValue;

        cameraHandler = handler;
        setContentView(R.layout.manual_convergence_slider_control);

        // Views handlers
        manualConvergencePanel = findViewById(R.id.panel);
        manualConvergenceControl = (SeekBar) findViewById(R.id.convergence_seek);
        manualConvergenceControl.setMax(Math.abs(minConvergence) + maxConvergence);
        manualConvergenceControl.setKeyProgressIncrement(stepConvergence);
        manualConvergenceCaption = (TextView) findViewById(R.id.convergence_caption);

        String sCaptionConvergence = context.getString(R.string.settings_manual_convergence_caption);
        sCaptionConvergence = sCaptionConvergence + " " + Integer.toString(mManualConvergence);
        manualConvergenceCaption.setText(sCaptionConvergence);
        mProgressBarConvergenceValue = mManualConvergence + Math.abs(minConvergence);
        manualConvergenceControl.setProgress(mProgressBarConvergenceValue);

        manualConvergencePanel.setVisibility(View.VISIBLE);

        Button btn = (Button) findViewById(R.id.buttonOK);
        btn.setOnClickListener(new View.OnClickListener() {
                public void onClick(View v) {
                    if (mConvergenceValue != mManualConvergence) {
                        Message msg = new Message();
                        msg.obj = (Integer) (mManualConvergence);
                        msg.what = Camera.MANUAL_CONVERGENCE_CHANGED;
                        cameraHandler.sendMessage(msg);
                    }
                    dismiss();
                }
        });

        manualConvergenceControl.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
                public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                    manualConvergenceCaption = (TextView) findViewById(R.id.convergence_caption);
                    mManualConvergence = progress + mMinConvergence;
                    String sCaption = context.getString(R.string.settings_manual_convergence_caption);
                    sCaption = sCaption + " " + Integer.toString(mManualConvergence);
                    manualConvergenceCaption.setText(sCaption);
                }
            public void onStartTrackingTouch(SeekBar seekBar) {
            }
            public void onStopTrackingTouch(SeekBar seekBar) {
            }
        });
    }
}
