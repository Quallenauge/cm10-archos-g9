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

package com.ti.arx.ARCowboid;

import android.content.Context;
import android.util.Log;
import android.view.Gravity;
import android.view.View;
import android.widget.FrameLayout;

public class ARLayout extends FrameLayout {

    public ARLayout(Context context, double aspectRatio) {
        super(context);
        mAspectRatio = aspectRatio;
    }
    
    @Override
    protected void onLayout(boolean changed, int l, int t, int r, int b) {
        if (changed && getChildCount() > 0) {
            
            int layoutW = r - l;
            int layoutH = b - t;            
            int childWidth = layoutW;
            int childHeight = layoutH;
            int childLeft = l;
            int childTop = t;
            
            if (layoutW > layoutH * mAspectRatio) {
                childWidth = (int) Math.round(layoutH * mAspectRatio);
                childLeft = (layoutW - childWidth) / 2;
            } else {
                childHeight = (int) Math.round(layoutW / mAspectRatio);
                childTop = (layoutH - childHeight) / 2;
            }
            for (int i = 0; i < getChildCount(); i++) {
                View child = getChildAt(i);
                child.layout(childLeft, childTop, childLeft + childWidth, childTop + childHeight);
            }           
        }
    }

    private double mAspectRatio;

}
