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

import android.os.SystemProperties;
import android.view.SurfaceHolder;
import android.util.Log;

import java.lang.reflect.Constructor;
import java.lang.reflect.Method;
import java.lang.reflect.Field;

/**
 * A Wrapper for the S3DView class, which uses reflection to avoid specifying the
 * a dependency on S3DView library at compile time which may or may not be available.
 * If the S3DView class is not found, this class essentially becomes a stub implementation
 * (i.e. it does nothing).
 */
public class S3DViewWrapper {

    private SurfaceHolder holder;
    private Object s3dView;
    private Method setLayoutMethod;
    private Object mono;
    private Object topBottom;
    private Object sideBySide;

    private static final boolean OMAP_ENHANCEMENT_S3D = SystemProperties.getBoolean("com.ti.omap_enhancement_s3d", false);
    private static final String TAG = "S3DViewWrapper";

    public S3DViewWrapper(SurfaceHolder holder) {
        this.holder = holder;

        if (!OMAP_ENHANCEMENT_S3D)
            return;

        try {
            Class s3dViewClazz = Class.forName("com.ti.s3d.S3DView");
            Class layoutClazz = Class.forName("com.ti.s3d.S3DView$Layout");

            Class[] params = new Class[]{SurfaceHolder.class};
            Constructor cons = s3dViewClazz.getConstructor(params);
            s3dView = cons.newInstance(holder);

            Class[] layoutParams = new Class[] {layoutClazz};
            setLayoutMethod = s3dViewClazz.getDeclaredMethod("setLayout", layoutParams);

            mono = Enum.valueOf(layoutClazz, "MONO");
            sideBySide = Enum.valueOf(layoutClazz, "SIDE_BY_SIDE_LR");
            topBottom = Enum.valueOf(layoutClazz, "TOPBOTTOM_L");
        } catch(Exception e) {
            s3dView = null;
            setLayoutMethod = null;
            mono = null;
            topBottom = null;
            sideBySide = null;
        }
    }

    public void setMonoLayout() {
        if (s3dView != null) {
            try {
                setLayoutMethod.invoke(s3dView, mono);
            } catch (Exception e) {
                Log.e(TAG, "Error setting Mono stereo layout");
            }
        }
    }

    public void setSideBySideLayout() {
        if (s3dView != null) {
            // If display orientation is portrait then side-by-side layout
            // should be transformed to top-bottom beacuse of the rotated
            // preview. This is the case for Blaze
            if (Util.isTabletUI()) {
                try {
                    setLayoutMethod.invoke(s3dView, sideBySide);
                } catch (Exception e) {
                    Log.e(TAG, "Error setting Side-by-Side stereo layout");
                }
            } else {
                try {
                    setLayoutMethod.invoke(s3dView, topBottom);
                } catch (Exception e) {
                    Log.e(TAG, "Error setting Top-Bottom stereo layout");
                }
            }
        }
    }

    public void setTopBottomLayout() {
        if (s3dView != null) {
            // If display orientation is portrait then top-bottom layout
            // should be transformed to side-by-side beacuse of the rotated
            // preview. This is the case for Blaze
            if (Util.isTabletUI()) {
                try {
                    setLayoutMethod.invoke(s3dView, topBottom);
                } catch (Exception e) {
                    Log.e(TAG, "Error setting Top-Bottom stereo layout");
                }
            } else {
                try {
                    setLayoutMethod.invoke(s3dView, sideBySide);
                } catch (Exception e) {
                    Log.e(TAG, "Error setting Side-by-Side stereo layout");
                }
            }
        }
    }
}
