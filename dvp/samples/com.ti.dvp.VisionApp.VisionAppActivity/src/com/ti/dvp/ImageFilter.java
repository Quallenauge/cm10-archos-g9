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

public class ImageFilter {

	static {
        System.loadLibrary("imagefilter"); // this will cause the JNI layer to be loaded and run-time linked.
    }
	
	public ImageFilter() {}
	
	public native void init();
	public native void deinit();
	
	/** \brief Computes a 3x1 filter based on [-k/64, (64 + 2*k)/64, -k/64]. */
	public native void filter3x1(byte[] input, int width, int height, byte[] output, int k);
	
}
