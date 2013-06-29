/*
 * Copyright (C) 2009 The Android Open Source Project
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

import android.app.Activity;
import android.content.Context;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.hardware.Camera.CameraInfo;
import android.hardware.Camera.Parameters;
import android.hardware.Camera.Size;
import android.media.CamcorderProfile;
import android.util.Log;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.StringTokenizer;

/**
 *  Provides utilities and keys for Camera settings.
 */
public class CameraSettings {
    private static final int NOT_FOUND = -1;

    public static final String KEY_VERSION = "pref_version_key";
    public static final String KEY_LOCAL_VERSION = "pref_local_version_key";
    public static final String KEY_RECORD_LOCATION = RecordLocationPreference.KEY;
    public static final String KEY_VIDEO_QUALITY = "pref_video_quality_key";
    public static final String KEY_VIDEO_TIME_LAPSE_FRAME_INTERVAL = "pref_video_time_lapse_frame_interval_key";
    public static final String KEY_AUDIO_ENCODER = "pref_camera_audioencoder_key";
    public static final String KEY_VIDEO_ENCODER = "pref_camera_videoencoder_key";
    public static final String KEY_VIDEO_FRAMERATE_RANGE = "pref_camera_videoframerate_range_key";
    public static final String KEY_VIDEO_BITRATE = "pref_camera_videobitrate_key";
    public static final String KEY_VIDEO_FORMAT = "pref_camera_video_format_key";
    public static final String KEY_VIDEO_FILE_CONTAINER = "pref_camera_video_record_container_key";
    public static final String KEY_VIDEO_TIMER = "pref_camera_video_timer_key";
    public static final String KEY_VSTAB = "pref_camera_vstab_key";
    public static final String KEY_VNF = "pref_camera_vnf_key";
    public static final String KEY_PICTURE_SIZE = "pref_camera_picturesize_key";
    public static final String KEY_PICTURE_SIZE_2D = "pref_camera_picturesize2d_key";
    public static final String KEY_JPEG_QUALITY = "pref_camera_jpegquality_key";
    public static final String KEY_FOCUS_MODE = "pref_camera_focusmode_key";
    public static final String KEY_FLASH_MODE = "pref_camera_flashmode_key";
    public static final String KEY_VIDEOCAMERA_FLASH_MODE = "pref_camera_video_flashmode_key";
    public static final String KEY_WHITE_BALANCE = "pref_camera_whitebalance_key";
    public static final String KEY_SCENE_MODE = "pref_camera_scenemode_key";
    public static final String KEY_EXPOSURE_COMPENSATION_MENU = "pref_camera_exposure_key";
    public static final String KEY_VIDEO_EFFECT = "pref_video_effect_key";
    public static final String KEY_CAMERA_ID = "pref_camera_id_key";
    public static final String KEY_TAP_TO_FOCUS_PROMPT_SHOWN = "pref_tap_to_focus_prompt_shown_key";
    public static final String KEY_MODE = "mode";
    public static final String KEY_MODE_MENU = "pref_camera_mode_key";
    public static final String KEY_CAP_MODE_VALUES = "mode-values";
    public static final String KEY_GBCE = "pref_camera_gbce_key";
    public static final String KEY_BRACKET_RANGE = "pref_camera_bracketrange_key";
    public static final String KEY_BURST = "pref_camera_burst_key";
    public static final String KEY_ISO = "pref_camera_iso_key";
    public static final String KEY_COLOR_EFFECT = "pref_camera_coloreffect_key";
    public static final String KEY_CONTRAST = "pref_camera_contrast_key";
    public static final String KEY_BRIGHTNESS = "pref_camera_brightness_key";
    public static final String KEY_PREVIEW_SIZE = "pref_camera_previewsize_key";
    public static final String KEY_PREVIEW_SIZE_2D = "pref_camera_previewsize2d_key";
    public static final String KEY_ANTIBANDING = "pref_camera_antibanding_key";
    public static final String KEY_EXPOSURE_MODE_MENU = "pref_camera_exposuremode_key";
    public static final String KEY_PREVIEW_FRAMERATE = "pref_camera_previewframerate_key";
    public static final String KEY_SATURATION = "pref_camera_saturation_key";
    public static final String KEY_SHARPNESS = "pref_camera_sharpness_key";
    public static final String KEY_VIDEO_MODE = "pref_camera_video_mode_key";

    public static final String KEY_MECHANICAL_MISALIGNMENT_CORRECTION_SUPPORTED = "mechanical-misalignment-correction-supported";
    public static final String KEY_MECHANICAL_MISALIGNMENT_CORRECTION = "mechanical-misalignment-correction";
    public static final String KEY_MECHANICAL_MISALIGNMENT_CORRECTION_MENU="pref_camera_mechanical_misalignment_correction_key";

    public static final String KEY_AUTO_CONVERGENCE = "pref_camera_autoconvergence_key";
    public static final String KEY_AUTOCONVERGENCE_MODE = "auto-convergence-mode";
    public static final String KEY_AUTOCONVERGENCE_MODE_VALUES = "auto-convergence-mode-values";
    public static final String KEY_MANUAL_CONVERGENCE = "manual-convergence";
    public static final String KEY_SUPPORTED_MANUAL_CONVERGENCE_MIN = "supported-manual-convergence-min";
    public static final String KEY_SUPPORTED_MANUAL_CONVERGENCE_MAX = "supported-manual-convergence-max";
    public static final String KEY_SUPPORTED_MANUAL_CONVERGENCE_STEP = "supported-manual-convergence-step";
    public static final String KEY_CAPTURE_LAYOUT = "pref_camera_capture_layout_key";
    public static final String KEY_PREVIEW_LAYOUT = "pref_camera_preview_layout_key";
    public static final String KEY_CAPTURE_LAYOUT_VALUES = "s3d-cap-frame-layout-values";
    public static final String KEY_PREVIEW_LAYOUT_VALUES = "s3d-prv-frame-layout-values";
    public static final String KEY_VIDEO_PREVIEW_LAYOUT = "pref_video_preview_layout_key";
    public static final String KEY_PREVIEW_SIZES_SS = "pref_camera_ss_previewsize_key";
    public static final String KEY_PREVIEW_SIZES_TB = "pref_camera_tb_previewsize_key";
    public static final String KEY_PICTURE_SIZES_SS = "pref_camera_ss_picturesize_key";
    public static final String KEY_PICTURE_SIZES_TB = "pref_camera_tb_picturesize_key";
    public static final String KEY_SUPPORTED_PICTURE_TOPBOTTOM_SIZES = "supported-picture-topbottom-size-values";
    public static final String KEY_SUPPORTED_PICTURE_SIDEBYSIDE_SIZES = "supported-picture-sidebyside-size-values";
    public static final String KEY_SUPPORTED_PREVIEW_TOPBOTTOM_SIZES = "supported-preview-topbottom-size-values";
    public static final String KEY_SUPPORTED_PREVIEW_SIDEBYSIDE_SIZES = "supported-preview-sidebyside-size-values";
    public static final String KEY_SUPPORTED_PICTURE_SUBSAMPLED_SIZES = "supported-picture-subsampled-size-values";
    public static final String KEY_SUPPORTED_PREVIEW_SUBSAMPLED_SIZES = "supported-preview-subsampled-size-values";

    public static final String TB_FULL_S3D_LAYOUT = "tb-full";
    public static final String SS_FULL_S3D_LAYOUT = "ss-full";
    public static final String TB_SUB_S3D_LAYOUT = "tb-subsampled";
    public static final String SS_SUB_S3D_LAYOUT = "ss-subsampled";

    public static final String KEY_S3D_CAP_FRAME_LAYOUT = "s3d-cap-frame-layout";
    public static final String KEY_S3D_PRV_FRAME_LAYOUT = "s3d-prv-frame-layout";

    private static final boolean OMAP_ENHANCEMENT_S3D = android.os.SystemProperties.getBoolean("com.ti.omap_enhancement_s3d", false);
    public static final String KEY_S3D_MENU = "pref_camera_s3d_key";

    public static final String KEY_EXPOSURE_MODE = "exposure";
    // 3D exposure keys
    public static final String KEY_SUPPORTED_MANUAL_EXPOSURE_MIN = "supported-manual-exposure-min";
    public static final String KEY_SUPPORTED_MANUAL_EXPOSURE_MAX = "supported-manual-exposure-max";
    public static final String KEY_SUPPORTED_MANUAL_EXPOSURE_STEP = "supported-manual-exposure-step";
    public static final String KEY_SUPPORTED_MANUAL_GAIN_ISO_MIN = "supported-manual-gain-iso-min";
    public static final String KEY_SUPPORTED_MANUAL_GAIN_ISO_MAX = "supported-manual-gain-iso-max";
    public static final String KEY_SUPPORTED_MANUAL_GAIN_ISO_STEP = "supported-manual-gain-iso-step";
    public static final String KEY_MANUAL_EXPOSURE = "manual-exposure";
    public static final String KEY_MANUAL_EXPOSURE_RIGHT = "manual-exposure-right";
    public static final String KEY_MANUAL_GAIN_ISO = "manual-gain-iso";
    public static final String KEY_MANUAL_GAIN_ISO_RIGHT = "manual-gain-iso-right";

    public static final String KEY_TEMPORAL_BRACKETING = "temporal-bracketing";
    public static final String KEY_CAMERA_FIRST_USE_HINT_SHOWN = "pref_camera_first_use_hint_shown_key";
    public static final String KEY_VIDEO_FIRST_USE_HINT_SHOWN = "pref_video_first_use_hint_shown_key";

    public static final String KEY_PICTURE_FORMAT_MENU = "pref_camera_picture_format_key";
    public static final String KEY_PICTURE_FORMAT = "picture-format";
    public static final String KEY_SUPPORTED_PICTURE_FORMATS = "picture-format-values";

    public static final String KEY_SUPPORTED_PREVIEW_FRAMERATES_EXT = "preview-fps-ext-values";
    public static final String KEY_SUPPORTED_PREVIEW_FRAMERATE_RANGES_EXT = "preview-fps-range-ext-values";

    public static final String EXPOSURE_DEFAULT_VALUE = "0";

    //CPCAM shot parameres
    public static final String KEY_SHOTPARAMS_MANUAL_EXPOSURE_GAIN_POPUP_SLIDERS = "pref_camera_manual_exp_gain_key";
    public static final String KEY_SHOTPARAMS_BURST = "burst-capture";
    public static final String KEY_SHOTPARAMS_EXP_GAIN_PAIRS = "exp-gain-pairs";

    public static final int CURRENT_VERSION = 5;
    public static final int CURRENT_LOCAL_VERSION = 2;

    public static final int DEFAULT_VIDEO_DURATION = 0; // no limit
    public static final int DEFAULT_VIDEO_FORMAT_VALUE = 8; // 720p
    public static final int DEFAULT_AUDIO_ENCODER_VALUE = 3; // AAC
    public static final int DEFAULT_VIDEO_ENCODER_VALUE = 2; // H264
    public static final int DEFAULT_VIDEO_FRAMERATE_VALUE = 30;
    public static final int DEFAULT_VIDEO_BITRATE_VALUE = 12000000;
    private static final String TAG = "CameraSettings";

    private final Context mContext;
    private final Parameters mParameters;
    private final CameraInfo[] mCameraInfo;
    private final int mCameraId;

    //Maximum bitrates for h263 and MPEG4V in bits per second for various video resolutions.
    public static final int MAX_VIDEO_BITRATE_FOR_128x96 = 384*1024; //SQCIF
    public static final int MAX_VIDEO_BITRATE_FOR_176x144 = 384*1024; //QCIF
    public static final int MAX_VIDEO_BITRATE_FOR_352x288 = 2*1024*1024; //CIF
    public static final int MAX_VIDEO_BITRATE_FOR_320x240 = 2*1024*1024; //QVGA
    public static final int MAX_VIDEO_BITRATE_FOR_640x480 = 8*1024*1024; //VGA
    public static final int MAX_VIDEO_BITRATE_FOR_720x480 = 8*1024*1024; //NTSC
    public static final int MAX_VIDEO_BITRATE_FOR_720x576 = 8*1024*1024; //PAL
    public static final int MAX_VIDEO_BITRATE_FOR_800x480 = 8*1024*1024; //WVGA
    public static final int MAX_VIDEO_BITRATE_FOR_1280x720 = 12*1024*1024; //720p

    public static final Map<Integer,Integer> MAX_VIDEO_BITRATES;

    static {
        Map<Integer,Integer> bitrates = new HashMap<Integer,Integer>();
        bitrates.put(128*96, MAX_VIDEO_BITRATE_FOR_128x96);
        bitrates.put(176*144, MAX_VIDEO_BITRATE_FOR_176x144);
        bitrates.put(352*288, MAX_VIDEO_BITRATE_FOR_352x288);
        bitrates.put(320*240, MAX_VIDEO_BITRATE_FOR_320x240);
        bitrates.put(640*480, MAX_VIDEO_BITRATE_FOR_640x480);
        bitrates.put(720*480, MAX_VIDEO_BITRATE_FOR_720x480);
        bitrates.put(720*576, MAX_VIDEO_BITRATE_FOR_720x576);
        bitrates.put(800*480, MAX_VIDEO_BITRATE_FOR_800x480);
        bitrates.put(1280*720, MAX_VIDEO_BITRATE_FOR_1280x720);
        MAX_VIDEO_BITRATES = Collections.unmodifiableMap(bitrates);
    }

    public CameraSettings(Activity activity, Parameters parameters,
                          int cameraId, CameraInfo[] cameraInfo) {
        mContext = activity;
        mParameters = parameters;
        mCameraId = cameraId;
        mCameraInfo = cameraInfo;
    }

    public PreferenceGroup getPreferenceGroup(int preferenceRes) {
        PreferenceInflater inflater = new PreferenceInflater(mContext);
        PreferenceGroup group =
                (PreferenceGroup) inflater.inflate(preferenceRes);
        initPreference(group);
        return group;
    }

    public static String getDefaultVideoQuality(int cameraId,
            String defaultQuality) {
        int quality = Integer.valueOf(defaultQuality);
        if (CamcorderProfile.hasProfile(cameraId, quality)) {
            return defaultQuality;
        }
        return Integer.toString(CamcorderProfile.QUALITY_HIGH);
    }

    public static void initialCameraPictureSize(
            Context context, Parameters parameters) {
        // When launching the camera app first time, we will set the picture
        // size to the first one in the list defined in "arrays.xml" and is also
        // supported by the driver.
        List<String> supported = new ArrayList<String>();
        int pictureEntryValues = R.array.pref_camera_picturesize_entryvalues;
        String supp = null;
        String captureLayout = parameters.get(KEY_S3D_CAP_FRAME_LAYOUT);
        if (captureLayout !=null && !captureLayout.equals("")) {
            if (captureLayout.equals(TB_FULL_S3D_LAYOUT)) {
                supp = parameters.get(KEY_SUPPORTED_PICTURE_TOPBOTTOM_SIZES);
                if (supp !=null && !supp.equals("")) {
                    for (String item : supp.split(",")) {
                        supported.add(item);
                    }
                }
                pictureEntryValues = R.array.pref_camera_tb_picturesize_entryvalues;
            } else if (captureLayout.equals(SS_FULL_S3D_LAYOUT)) {
                supp = parameters.get(KEY_SUPPORTED_PICTURE_SIDEBYSIDE_SIZES);
                if (supp !=null && !supp.equals("")) {
                    for (String item : supp.split(",")) {
                        supported.add(item);
                    }
                }
                pictureEntryValues = R.array.pref_camera_ss_picturesize_entryvalues;
            } else if(captureLayout.equals(SS_SUB_S3D_LAYOUT)|| captureLayout.equals(TB_SUB_S3D_LAYOUT)) {
                supp = parameters.get(KEY_SUPPORTED_PICTURE_SUBSAMPLED_SIZES);
                if (supp !=null && !supp.equals("")) {
                    for (String item : supp.split(",")) {
                        supported.add(item);
                    }
                }
            } else {
                List<Size> suppSizes = parameters.getSupportedPictureSizes();
                if (suppSizes != null) {
                    supported = sizeListToStringList(suppSizes);
                }
            }
        }else{
            List<Size> suppSizes = parameters.getSupportedPictureSizes();
            if (suppSizes != null) {
                supported = sizeListToStringList(suppSizes);
            }
        }

        if (supported == null) return;
        for (String candidate : context.getResources().getStringArray(pictureEntryValues)) {
            if (setCameraPictureSize(candidate, supported, parameters)) {
                SharedPreferences.Editor editor = ComboPreferences
                        .get(context).edit();
                editor.putString(KEY_PICTURE_SIZE, candidate);
                editor.apply();
                return;
            }
        }
        Log.e(TAG, "No supported picture size found");
    }

    public static void initialFrameRate(Context context, Parameters parameters) {
        List<Integer> supported = parameters.getSupportedPreviewFrameRates();
        for (String candidate : context.getResources().getStringArray(R.array.pref_camera_previewframerate_entryvalues)) {
            if (setCameraFrameRate(candidate, supported, parameters)) {
                SharedPreferences.Editor editor = ComboPreferences
                        .get(context).edit();
                editor.putString(KEY_PREVIEW_FRAMERATE, candidate);
                editor.apply();
                return;
            }
        }
    }

    public static void removePreferenceFromScreen(
            PreferenceGroup group, String key) {
        removePreference(group, key);
    }

    public static boolean setCameraFrameRate(String candidate,List<Integer> supported, Parameters parameters) {
        int intCandidate = Integer.parseInt(candidate);
        if (supported.indexOf(intCandidate) >= 0) {
            List<int[]> supportedRanges = parameters.getSupportedPreviewFpsRange();
            boolean isRangeSupported = false;
            for (int[] arr : supportedRanges) {
                if (((intCandidate * 1000) == arr[0]) && ((intCandidate * 1000) == arr[1])) {
                    isRangeSupported = true;
                    break;
                }
            }
            if (isRangeSupported) {
                parameters.setPreviewFpsRange(intCandidate * 1000, intCandidate * 1000);
                return true;
            }
        }
        return false;
    }

    public static boolean getSupportedFramerateRange(int[] range, final List<int[]> supportedFpsRanges) {

        if (supportedFpsRanges.isEmpty()) {
            return false;
        }

        int currentBest = Integer.MAX_VALUE;
        int bestMatch = range[Parameters.PREVIEW_FPS_MAX_INDEX];
        List<int[]> matchedResults = new ArrayList<int[]>();
        matchedResults.add(range);

        //Find fpsRange with minimum difference in maximum fps value
        for (int[] fpsRange: supportedFpsRanges) {
            final int diff = Math.abs(fpsRange[Parameters.PREVIEW_FPS_MAX_INDEX] - range[Parameters.PREVIEW_FPS_MAX_INDEX]);
            if (diff < currentBest) {
                currentBest = diff;
                bestMatch = fpsRange[Parameters.PREVIEW_FPS_MAX_INDEX];
                matchedResults.clear();
                matchedResults.add(fpsRange);
            }
            else if (diff == currentBest) {
                matchedResults.add(fpsRange);
            }
        }

        if (matchedResults.size() == 1) {
            range[Parameters.PREVIEW_FPS_MIN_INDEX] = matchedResults.get(0)[Parameters.PREVIEW_FPS_MIN_INDEX];
            range[Parameters.PREVIEW_FPS_MAX_INDEX] = matchedResults.get(0)[Parameters.PREVIEW_FPS_MAX_INDEX];
            return true;
        }

        //If we got more than one result - find range with minimum difference in
        //minimum fps value
        currentBest = Integer.MAX_VALUE;
        int[] bestMatchRange = {range[Parameters.PREVIEW_FPS_MIN_INDEX], bestMatch};
        for (int[] fpsRange: matchedResults) {
            final int diff = Math.abs(fpsRange[Parameters.PREVIEW_FPS_MIN_INDEX] - range[Parameters.PREVIEW_FPS_MIN_INDEX]);
            if (diff < currentBest) {
                currentBest = diff;
                bestMatchRange[Parameters.PREVIEW_FPS_MIN_INDEX] = fpsRange[Parameters.PREVIEW_FPS_MIN_INDEX];
            }
        }
        range[Parameters.PREVIEW_FPS_MIN_INDEX] = bestMatchRange[Parameters.PREVIEW_FPS_MIN_INDEX];
        range[Parameters.PREVIEW_FPS_MAX_INDEX] = bestMatchRange[Parameters.PREVIEW_FPS_MAX_INDEX];
        return true;
    }

    public static boolean setCameraPictureSize(
            String candidate, List<String> supported, Parameters parameters) {
        int index = candidate.indexOf('x');
        if (index == NOT_FOUND) return false;
        int width = Integer.parseInt(candidate.substring(0, index));
        int height = Integer.parseInt(candidate.substring(index + 1));
        for (String item : supported) {
            int w = Integer.parseInt(item.substring(0, item.indexOf("x")));
            int h = Integer.parseInt(item.substring(item.indexOf("x") + 1));
            if (w == width && h == height) {
                parameters.setPictureSize(width, height);
                return true;
            }
        }
        return false;
    }

    public static boolean setCameraPreviewSize(
          String candidate, List<String> supported, Parameters parameters) {
       int index = candidate.indexOf('x');
       if (index == NOT_FOUND) return false;
       int width = Integer.parseInt(candidate.substring(0, index));
       int height = Integer.parseInt(candidate.substring(index + 1));
       for (String item: supported) {
           int w = Integer.parseInt(item.substring(0, item.indexOf("x")));
           int h = Integer.parseInt(item.substring(item.indexOf("x") + 1));
           if (w == width && h == height) {
               parameters.setPreviewSize(w, h);
               return true;
           }
       }
       return false;
    }

    private void checkPreferenceSizeMatches(ListPreference a, ListPreference b) {
        if (a != null && b != null) {
            if (a.getEntryValues().length != b.getEntryValues().length) {
                throw new RuntimeException("Preference list sizes do not match for keys: " +
                        a.getKey() + " and " + b.getKey() + ". Check the arrays.xml");
            }
        }
    }

    private void initPreference(PreferenceGroup group) {
        ListPreference videoQuality = group.findPreference(KEY_VIDEO_QUALITY);
        ListPreference videoFormat = group.findPreference(KEY_VIDEO_FORMAT);
        ListPreference timeLapseInterval = group.findPreference(KEY_VIDEO_TIME_LAPSE_FRAME_INTERVAL);
        ListPreference whiteBalance =  group.findPreference(KEY_WHITE_BALANCE);
        ListPreference sceneMode = group.findPreference(KEY_SCENE_MODE);
        ListPreference flashMode = group.findPreference(KEY_FLASH_MODE);
        ListPreference focusMode = group.findPreference(KEY_FOCUS_MODE);
        ListPreference exposure = group.findPreference(KEY_EXPOSURE_COMPENSATION_MENU);
        ListPreference cpcam_manual_exp_gain = group.findPreference(KEY_SHOTPARAMS_MANUAL_EXPOSURE_GAIN_POPUP_SLIDERS);
        ListPreference colorEffect = group.findPreference(KEY_COLOR_EFFECT);
        ListPreference autoConvergence = group.findPreference(KEY_AUTO_CONVERGENCE);
        ListPreference iso = group.findPreference(KEY_ISO);
        ListPreference captureLayout = group.findPreference(KEY_CAPTURE_LAYOUT);
        ListPreference previewLayout = group.findPreference(KEY_PREVIEW_LAYOUT);
        IconListPreference cameraIdPref =
                (IconListPreference) group.findPreference(KEY_CAMERA_ID);
        ListPreference videoFlashMode =
                group.findPreference(KEY_VIDEOCAMERA_FLASH_MODE);
        ListPreference videoEffect = group.findPreference(KEY_VIDEO_EFFECT);
        ListPreference antibanding = group.findPreference(KEY_ANTIBANDING);
        ListPreference vstab = group.findPreference(KEY_VSTAB);
        ListPreference vnf = group.findPreference(KEY_VNF);
        ListPreference exposureMode = group.findPreference(KEY_EXPOSURE_MODE_MENU);
        ListPreference previewFramerate = group.findPreference(KEY_PREVIEW_FRAMERATE);
        ListPreference mechanicalMisalignmentCorrection = group.findPreference(KEY_MECHANICAL_MISALIGNMENT_CORRECTION_MENU);
        ListPreference mode = group.findPreference(KEY_MODE_MENU);
        ListPreference pictureFormat = group.findPreference(KEY_PICTURE_FORMAT_MENU);
        ListPreference videoPreviewLayout = group.findPreference(KEY_VIDEO_PREVIEW_LAYOUT);
        ListPreference pictureSize = group.findPreference(KEY_PICTURE_SIZE);
        ListPreference pictureSize2d = group.findPreference(KEY_PICTURE_SIZE_2D);
        ListPreference previewSize = group.findPreference(KEY_PREVIEW_SIZE);
        ListPreference previewSize2d = group.findPreference(KEY_PREVIEW_SIZE_2D);
        ListPreference pictureSizeTB = group.findPreference(KEY_PICTURE_SIZES_TB);
        ListPreference pictureSizeSS = group.findPreference(KEY_PICTURE_SIZES_SS);
        ListPreference previewSizeTB = group.findPreference(KEY_PREVIEW_SIZES_TB);
        ListPreference previewSizeSS = group.findPreference(KEY_PREVIEW_SIZES_SS);
        ListPreference s3d = group.findPreference(KEY_S3D_MENU);
        ListPreference videoMode = group.findPreference(KEY_VIDEO_MODE);

        checkPreferenceSizeMatches(previewSize, previewSizeTB);
        checkPreferenceSizeMatches(previewSize, previewSizeSS);
        checkPreferenceSizeMatches(pictureSize, pictureSizeTB);
        checkPreferenceSizeMatches(pictureSize, pictureSizeSS);

        if(s3d != null){
            List<String> supp = new ArrayList<String>();
            boolean mode2d = false;
            String suppLayouts = mParameters.get(KEY_PREVIEW_LAYOUT_VALUES);
            // if there aren't supported layouts  -> 2d mode
            if(suppLayouts == null ||
                    suppLayouts.equals("") ||
                    suppLayouts.equals("none")){
                mode2d = true;
            }else{
                mode2d = false;
            }
            if(OMAP_ENHANCEMENT_S3D && !mode2d){
                supp.add("on");
                supp.add("off");
            }else{
                supp.add("none");
            }
            filterUnsupportedOptions(group, s3d, supp);
        }

        if (cpcam_manual_exp_gain != null) {
            List<String> supp = new ArrayList<String>();
            supp.add("on");
            supp.add("off");
            filterUnsupportedOptions(group, cpcam_manual_exp_gain, supp);
        }

        if (previewSize2d != null) {
            List<String> supp = new ArrayList<String>();
            String suppSizes  = mParameters.get(KEY_SUPPORTED_PREVIEW_SUBSAMPLED_SIZES);
            if (suppSizes !=null && !suppSizes.equals("")) {
                for (String item : suppSizes.split(",")) {
                    supp.add(item);
                }
            } else {
                supp = sizeListToStringList(mParameters.getSupportedPreviewSizes());
            }
            filterUnsupportedOptions(group, previewSize2d, supp);
        }

        if (pictureSize2d != null) {
            List<String> supp = new ArrayList<String>();
            String suppSizes  = mParameters.get(KEY_SUPPORTED_PICTURE_SUBSAMPLED_SIZES);
            if (suppSizes !=null && !suppSizes.equals("")) {
                for (String item : suppSizes.split(",")) {
                    supp.add(item);
                }
            } else {
                supp = sizeListToStringList(mParameters.getSupportedPictureSizes());
            }
            filterUnsupportedOptions(group, pictureSize2d, supp);
        }

        if (pictureSizeTB !=null) {
            List<String> supp = new ArrayList<String>();
            String suppSizes = mParameters.get(KEY_SUPPORTED_PICTURE_TOPBOTTOM_SIZES);
            if (suppSizes !=null && !suppSizes.equals("")) {
                for (String item : suppSizes.split(",")) {
                    supp.add(item);
                }
            }
            filterUnsupportedOptions(group, pictureSizeTB, supp);
        }

        if (pictureSizeSS !=null) {
            List<String> supp = new ArrayList<String>();
            String suppSizes = mParameters.get(KEY_SUPPORTED_PICTURE_SIDEBYSIDE_SIZES);
            if (suppSizes !=null && !suppSizes.equals("")) {
                for (String item : suppSizes.split(",")) {
                    supp.add(item);
                }
            }
            filterUnsupportedOptions(group, pictureSizeSS, supp);
        }
        ListPreference contrastEnhancement = group.findPreference(KEY_GBCE);

        if (previewSizeSS !=null) {
            List<String> supp = new ArrayList<String>();
            String suppSizes = mParameters.get(KEY_SUPPORTED_PREVIEW_SIDEBYSIDE_SIZES);
            if (suppSizes !=null && !suppSizes.equals("")) {
                for (String item : suppSizes.split(",")) {
                    supp.add(item);
                }
            }
            filterUnsupportedOptions(group, previewSizeSS, supp);
        }

        if(previewSizeTB !=null){
            List<String> supp = new ArrayList<String>();
            String suppSizes = mParameters.get(KEY_SUPPORTED_PREVIEW_TOPBOTTOM_SIZES);
            if (suppSizes !=null && !suppSizes.equals("")) {
                for (String item : suppSizes.split(",")) {
                    supp.add(item);
                }
            }
            filterUnsupportedOptions(group, previewSizeTB, supp);
        }

        String key_picture_sizes = null;
        String selectedPictureLayout = mParameters.get(KEY_S3D_CAP_FRAME_LAYOUT);
        if (selectedPictureLayout !=null && !selectedPictureLayout.equals("")) {
            if (selectedPictureLayout.equals(TB_FULL_S3D_LAYOUT)) {
                key_picture_sizes = KEY_PICTURE_SIZES_TB;
            } else if (selectedPictureLayout.equals(SS_FULL_S3D_LAYOUT)) {
                key_picture_sizes = KEY_PICTURE_SIZES_SS;
            } else {
                key_picture_sizes = KEY_PICTURE_SIZE_2D;
            }
        } else {
            key_picture_sizes = KEY_PICTURE_SIZE_2D;
        }

        String key_preview_sizes = null;
        String selectedPreviewLayout = mParameters.get(KEY_S3D_PRV_FRAME_LAYOUT);
        if (selectedPreviewLayout !=null && !selectedPreviewLayout.equals("")) {
            if (selectedPreviewLayout.equals(TB_FULL_S3D_LAYOUT)) {
                key_preview_sizes = KEY_PREVIEW_SIZES_TB;
            } else if (selectedPreviewLayout.equals(SS_FULL_S3D_LAYOUT)) {
                key_preview_sizes = KEY_PREVIEW_SIZES_SS;
            } else {
                key_preview_sizes = KEY_PREVIEW_SIZE_2D;
            }
        } else {
            key_preview_sizes = KEY_PREVIEW_SIZE_2D;
        }
        // Since the screen could be loaded from different resources, we need
        // to check if the preference is available here
        if ( vstab != null ) {
            filterVSTAB(group, vstab);
        }
        if ( vnf != null ) {
            filterVNF(group, vnf);
        }
        if (videoQuality != null) {
            filterUnsupportedOptions(group, videoQuality, getSupportedVideoQuality());
        }
        if (videoFormat != null) {
            List<String> supp = new ArrayList<String>();
            String suppPreview  = mParameters.get(KEY_SUPPORTED_PREVIEW_SUBSAMPLED_SIZES);
            if (suppPreview != null && !suppPreview.equals("")) {
                for (String previewItem : suppPreview.split(",")) {
                    int index = 0;
                    for (String candidate : mContext.getResources().getStringArray(R.array.pref_camera_video_format_sizevalues)) {
                        if (previewItem.equals(candidate)) {
                            supp.add(mContext.getResources().getStringArray(R.array.pref_camera_video_format_entryvalues)[index].toString());
                        }
                        index++;
                    }
                }
            } else {
                List<String> supported = sizeListToStringList(mParameters.getSupportedPreviewSizes());
                for (String previewItem : supported) {
                    int index = 0;
                    for (String candidate : mContext.getResources().getStringArray(R.array.pref_camera_video_format_sizevalues)) {
                        if (previewItem.equals(candidate)) {
                            supp.add(mContext.getResources().getStringArray(R.array.pref_camera_video_format_entryvalues)[index].toString());
                        }
                        index++;
                    }
                }
            }
            filterUnsupportedOptions(group, videoFormat, supp);
        }
        if (videoPreviewLayout != null) {
            ArrayList<String> suppLayout = new ArrayList<String>();
            String videoLayouts = mParameters.get(KEY_PREVIEW_LAYOUT_VALUES);
            if (videoLayouts != null && !videoLayouts.equals("") && !videoLayouts.equals("none")) {
                for (String item : videoLayouts.split(",")) {
                    suppLayout.add(item);
                }
            }
            filterUnsupportedOptions(group,videoPreviewLayout,suppLayout);
        }

        if (captureLayout != null) {
            ArrayList<String> suppLayout = new ArrayList<String>();
            String captureLayouts = mParameters.get(KEY_CAPTURE_LAYOUT_VALUES);
            if (captureLayouts !=null && !captureLayouts.equals("")) {
                for (String item : captureLayouts.split(",")) {
                    suppLayout.add(item);
                }
                filterUnsupportedOptions(group,captureLayout,suppLayout);
            }
        }

        if (previewLayout != null) {
            ArrayList<String> suppLayout = new ArrayList<String>();
            String previewLayouts = mParameters.get(KEY_PREVIEW_LAYOUT_VALUES);
            if (previewLayouts !=null && !previewLayouts.equals("")) {
                for (String item : previewLayouts.split(",")) {
                    suppLayout.add(item);
                }
                filterUnsupportedOptions(group,previewLayout,suppLayout);
            }
        }

        ArrayList<CharSequence[]> allPreviewEntries = new ArrayList<CharSequence[]>();
        ArrayList<CharSequence[]> allPreviewEntryValues = new ArrayList<CharSequence[]>();
        allPreviewEntries.add(previewSize2d.getEntries());
        allPreviewEntries.add(previewSizeTB.getEntries());
        allPreviewEntries.add(previewSizeSS.getEntries());
        allPreviewEntryValues.add(previewSize2d.getEntryValues());
        allPreviewEntryValues.add(previewSizeTB.getEntryValues());
        allPreviewEntryValues.add(previewSizeSS.getEntryValues());

        ArrayList<CharSequence[]> allPictureEntries = new ArrayList<CharSequence[]>();
        ArrayList<CharSequence[]> allPictureEntryValues = new ArrayList<CharSequence[]>();

        if(pictureSize2d != null  && pictureSizeTB != null && pictureSizeSS != null){
            allPictureEntries.add(pictureSize2d.getEntries());
            allPictureEntries.add(pictureSizeTB.getEntries());
            allPictureEntries.add(pictureSizeSS.getEntries());
            allPictureEntryValues.add(pictureSize2d.getEntryValues());
            allPictureEntryValues.add(pictureSizeTB.getEntryValues());
            allPictureEntryValues.add(pictureSizeSS.getEntryValues());
        }

        ListPreference filteredSizes = group.findPreference(key_picture_sizes);
        if (filteredSizes != null) {
            ListPreference pictureSizes = group.findPreference(KEY_PICTURE_SIZE);
            if (pictureSizes !=null) {
                pictureSizes.clearAndSetEntries(allPictureEntries, allPictureEntryValues,
                        filteredSizes.getEntries(), filteredSizes.getEntryValues());
            }
        }
        filteredSizes = group.findPreference(key_preview_sizes);
        if (filteredSizes != null) {
            ListPreference previewSizes = group.findPreference(KEY_PREVIEW_SIZE);
            if (previewSizes !=null) {
                previewSizes.clearAndSetEntries(allPreviewEntries, allPreviewEntryValues,
                        filteredSizes.getEntries(), filteredSizes.getEntryValues());
            }
        }
        if (whiteBalance != null) {
            filterUnsupportedOptions(group,
                    whiteBalance, mParameters.getSupportedWhiteBalance());
        }
        if (previewFramerate != null) {
            filterUnsupportedOptionsInt(group,
                    previewFramerate, mParameters.getSupportedPreviewFrameRates());
        }

        List<Integer> fpsList = getSupportedFramerates(mParameters);

        if(videoMode != null){
            ArrayList<String> suppMode = new ArrayList<String>();
            String modeValues = mParameters.get(KEY_CAP_MODE_VALUES);
            if(modeValues != null && !modeValues.equals("")){
                for(String item : modeValues.split(",")){
                    suppMode.add(item);
                }
            }
            filterUnsupportedOptions(group, videoMode, suppMode);
        }

        if (sceneMode != null) {
            filterUnsupportedOptions(group,
                    sceneMode, mParameters.getSupportedSceneModes());
        }
        if (flashMode != null) {
            filterUnsupportedOptions(group,
                    flashMode, mParameters.getSupportedFlashModes());
        }
        if (colorEffect != null) {
            filterUnsupportedOptions(group,
                    colorEffect, mParameters.getSupportedColorEffects());
        }
        if (antibanding != null) {
            filterUnsupportedOptions(group,
                    antibanding, mParameters.getSupportedAntibanding());
        }
        if ( iso  != null) {
            filterUnsupportedOptions(group, iso,
                    parseToList(mParameters.get(Camera.PARM_SUPPORTED_ISO_MODES)));
        }
        if ( exposureMode  != null) {
            filterUnsupportedOptions(group, exposureMode,
                    parseToList(mParameters.get(Camera.PARM_SUPPORTED_EXPOSURE_MODES)));
        }
        if (focusMode != null) {
            filterUnsupportedOptions(group,
                    focusMode, mParameters.getSupportedFocusModes());
        }
        if (videoFlashMode != null) {
            filterUnsupportedOptions(group,
                    videoFlashMode, mParameters.getSupportedFlashModes());
        }
        if (exposure != null) buildExposureCompensation(group, exposure);
        if (cameraIdPref != null) buildCameraId(group, cameraIdPref);

        if (timeLapseInterval != null) resetIfInvalid(timeLapseInterval);
        if (videoEffect != null) {
            initVideoEffect(group, videoEffect);
            resetIfInvalid(videoEffect);
        }

        if (autoConvergence != null) {
            ArrayList<String> suppConvergence = new ArrayList<String>();
            String convergence = mParameters.get(CameraSettings.KEY_AUTOCONVERGENCE_MODE_VALUES);
            if (convergence != null && !convergence.equals("")) {
                for(String item : convergence.split(",")){
                    suppConvergence.add(item);
                }
            }
            filterUnsupportedOptions(group,
                    autoConvergence, suppConvergence);
        }

        if(mechanicalMisalignmentCorrection !=null){
            final String mechanicalMisalignmentSupported =
                    mParameters.get(KEY_MECHANICAL_MISALIGNMENT_CORRECTION_SUPPORTED);
            ArrayList<String> suppMechanicalMisalignmentCorrection = new ArrayList<String>();
            if ( mechanicalMisalignmentSupported != null && mechanicalMisalignmentSupported.equals(Camera.TRUE) ) {
                final String enable = mContext.getString(R.string.pref_camera_mechanical_misalignment_correction_enable);
                final String disable = mContext.getString(R.string.pref_camera_mechanical_misalignment_correction_disable);
                suppMechanicalMisalignmentCorrection.add(mechanicalMisalignmentCorrection.findEntryValueByEntry(enable));
                suppMechanicalMisalignmentCorrection.add(mechanicalMisalignmentCorrection.findEntryValueByEntry(disable));
            }
            filterUnsupportedOptions(group, mechanicalMisalignmentCorrection, suppMechanicalMisalignmentCorrection);
        }

        if(mode !=null){
            ArrayList<String> suppMode = new ArrayList<String>();
            String modeValues = mParameters.get(KEY_CAP_MODE_VALUES);
            if(modeValues != null && !modeValues.equals("")){
                for(String item : modeValues.split(",")){
                    suppMode.add(item);
                }
            }
            filterUnsupportedOptions(group, mode, suppMode);
        }

        if (pictureFormat != null) {
            ArrayList<String> suppPictureFormat = new ArrayList<String>();
            String pictureFormatValues = mParameters.get(KEY_SUPPORTED_PICTURE_FORMATS);
            if (pictureFormatValues != null && !pictureFormatValues.equals("")) {
                for (String item : pictureFormatValues.split(",")) {
                    suppPictureFormat.add(item);
                }
            }
            filterUnsupportedOptions(group, pictureFormat, suppPictureFormat);
        }

        if ( contrastEnhancement != null ) {
            filterContrastEnhancement(group, contrastEnhancement);
        }
    }

    private void buildExposureCompensation(
            PreferenceGroup group, ListPreference exposure) {
        int max = mParameters.getMaxExposureCompensation();
        int min = mParameters.getMinExposureCompensation();
        if (max == 0 && min == 0) {
            removePreference(group, exposure.getKey());
            return;
        }
        float step = mParameters.getExposureCompensationStep();

        // show only integer values for exposure compensation
        int maxValue = (int) Math.floor(max * step);
        int minValue = (int) Math.ceil(min * step);
        CharSequence entries[] = new CharSequence[maxValue - minValue + 1];
        CharSequence entryValues[] = new CharSequence[maxValue - minValue + 1];
        for (int i = minValue; i <= maxValue; ++i) {
            entryValues[maxValue - i] = Integer.toString(Math.round(i / step));
            StringBuilder builder = new StringBuilder();
            if (i > 0) builder.append('+');
            entries[maxValue - i] = builder.append(i).toString();
        }
        exposure.setEntries(entries);
        exposure.setEntryValues(entryValues);
    }

    private void buildCameraId(
            PreferenceGroup group, IconListPreference preference) {
        int numOfCameras = mCameraInfo.length;
        if (numOfCameras < 2) {
            removePreference(group, preference.getKey());
            return;
        }

        CharSequence[] entryValues = new CharSequence[numOfCameras];
        for (int i = 0; i < mCameraInfo.length; ++i) {
            entryValues[i] = "" + i;
        }
        preference.setEntryValues(entryValues);
    }

    private static boolean removePreference(PreferenceGroup group, String key) {
        for (int i = 0, n = group.size(); i < n; i++) {
            CameraPreference child = group.get(i);
            if (child instanceof PreferenceGroup) {
                if (removePreference((PreferenceGroup) child, key)) {
                    return true;
                }
            }
            if (child instanceof ListPreference &&
                    ((ListPreference) child).getKey().equals(key)) {
                group.removePreference(i);
                return true;
            }
        }
        return false;
    }

    private void filterVNF(PreferenceGroup group, ListPreference pref) {
        String vnfSupported = mParameters.get(VideoCamera.PARM_VNF_SUPPORTED);
        String enable =  mContext.getString(R.string.pref_camera_vnf_entry_on);
        String disable = mContext.getString(R.string.pref_camera_vnf_entry_off);
        List<String> supported = new ArrayList<String>();

        if ( "true".equals(vnfSupported) ) {
            supported.add(pref.findEntryValueByEntry(enable));
            supported.add(pref.findEntryValueByEntry(disable));
        }

        filterUnsupportedOptions(group, pref, supported);
    }

    private void filterVSTAB(PreferenceGroup group, ListPreference pref) {
        boolean vstabSupported = mParameters.isVideoStabilizationSupported();
        String enable =  mContext.getString(R.string.pref_camera_vstab_entry_on);
        String disable = mContext.getString(R.string.pref_camera_vstab_entry_off);
        List<String> supported = new ArrayList<String>();

        if ( vstabSupported ) {
            supported.add(pref.findEntryValueByEntry(enable));
            supported.add(pref.findEntryValueByEntry(disable));
        }

        filterUnsupportedOptions(group, pref, supported);
    }

    private void filterContrastEnhancement(PreferenceGroup group, ListPreference pref) {
        String gbceSupported = mParameters.get(Camera.PARM_SUPPORTED_GBCE);
        String glbceSupported = mParameters.get(Camera.PARM_SUPPORTED_GLBCE);
        String offEntry = mContext.getString(R.string.pref_camera_gbce_entry_off);
        String gbceEntry = mContext.getString(R.string.pref_camera_gbce_entry_gbce);
        String glbceEntry = mContext.getString(R.string.pref_camera_gbce_entry_glbce);
        List<String> supported = new ArrayList<String>();

        // Off is there by default
        supported.add(pref.findEntryValueByEntry(offEntry));

        if ( ( null == gbceSupported ) ||
             ( Camera.TRUE.equals(gbceSupported)) ) {
            supported.add(pref.findEntryValueByEntry(gbceEntry));
        }

        if ( ( null == glbceSupported ) ||
             ( Camera.TRUE.equals(glbceSupported)) ) {
            supported.add(pref.findEntryValueByEntry(glbceEntry));
           }

        filterUnsupportedOptions(group, pref, supported);
    }

    public static void filterUnsupportedOptions(PreferenceGroup group,
            ListPreference pref, List<String> supported) {

        // Remove the preference if the parameter is not supported or there is
        // only one options for the settings.
        if (supported == null || supported.size() <= 1) {
            removePreference(group, pref.getKey());
            return;
        }

        pref.filterUnsupported(supported);
        if (pref.getEntries().length <= 1) {
            removePreference(group, pref.getKey());
            return;
        }

        resetIfInvalid(pref);
    }

    private void filterUnsupportedOptionsInt(PreferenceGroup group,
            ListPreference pref, List<Integer> supported) {
        // Remove the preference if the parameter is not supported or there is
        // only one options for the settings.
        if (supported == null || supported.size() <= 1) {
            removePreference(group, pref.getKey());
            return;
        }

        pref.filterUnsupportedInt(supported);

        // Set the value to the first entry if it is invalid.
        String value = pref.getValue();
        if (pref.findIndexOfValue(value) == NOT_FOUND) {
            pref.setValueIndex(0);
        }
    }

    private static void resetIfInvalid(ListPreference pref) {
        // Set the value to the first entry if it is invalid.
        String value = pref.getValue();
        if (pref.findIndexOfValue(value) == NOT_FOUND) {
            pref.setValueIndex(0);
        }
    }

    public static List<String> sizeListToStringList(List<Size> sizes) {
        ArrayList<String> list = new ArrayList<String>();

        if (sizes != null) {
            for (Size size : sizes) {
                list.add(String.format("%dx%d", size.width, size.height));
            }
        }

        return list;
    }

    public static void upgradeLocalPreferences(SharedPreferences pref) {
        int version;
        try {
            version = pref.getInt(KEY_LOCAL_VERSION, 0);
        } catch (Exception ex) {
            version = 0;
        }
        if (version == CURRENT_LOCAL_VERSION) return;

        SharedPreferences.Editor editor = pref.edit();
        if (version == 1) {
            // We use numbers to represent the quality now. The quality definition is identical to
            // that of CamcorderProfile.java.
            editor.remove("pref_video_quality_key");
        }
        editor.putInt(KEY_LOCAL_VERSION, CURRENT_LOCAL_VERSION);
        editor.apply();
    }

    public static void upgradeGlobalPreferences(SharedPreferences pref) {
        int version;
        try {
            version = pref.getInt(KEY_VERSION, 0);
        } catch (Exception ex) {
            version = 0;
        }
        if (version == CURRENT_VERSION) return;

        SharedPreferences.Editor editor = pref.edit();
        if (version == 0) {
            // We won't use the preference which change in version 1.
            // So, just upgrade to version 1 directly
            version = 1;
        }
        if (version == 2) {
            editor.putString(KEY_RECORD_LOCATION,
                    pref.getBoolean(KEY_RECORD_LOCATION, false)
                    ? RecordLocationPreference.VALUE_ON
                    : RecordLocationPreference.VALUE_NONE);
            version = 3;
        }
        if (version == 3) {
            // Just use video quality to replace it and
            // ignore the current settings.
            editor.remove("pref_camera_videoquality_key");
            editor.remove("pref_camera_video_duration_key");
        }

        editor.putInt(KEY_VERSION, CURRENT_VERSION);
        editor.apply();
    }

    public static int readPreferredCameraId(SharedPreferences pref) {
        return Integer.parseInt(pref.getString(KEY_CAMERA_ID, "0"));
    }

    public static void writePreferredCameraId(SharedPreferences pref,
            int cameraId) {
        Editor editor = pref.edit();
        editor.putString(KEY_CAMERA_ID, Integer.toString(cameraId));
        editor.apply();
    }

    public static int readExposure(ComboPreferences preferences) {
        String exposure = preferences.getString(
                CameraSettings.KEY_EXPOSURE_COMPENSATION_MENU,
                EXPOSURE_DEFAULT_VALUE);
        try {
            return Integer.parseInt(exposure);
        } catch (Exception ex) {
            Log.e(TAG, "Invalid exposure: " + exposure);
        }
        return 0;
    }

    public static int readEffectType(SharedPreferences pref) {
        String effectSelection = pref.getString(KEY_VIDEO_EFFECT, "none");
        if (effectSelection.equals("none")) {
            return EffectsRecorder.EFFECT_NONE;
        } else if (effectSelection.startsWith("goofy_face")) {
            return EffectsRecorder.EFFECT_GOOFY_FACE;
        } else if (effectSelection.startsWith("backdropper")) {
            return EffectsRecorder.EFFECT_BACKDROPPER;
        }
        Log.e(TAG, "Invalid effect selection: " + effectSelection);
        return EffectsRecorder.EFFECT_NONE;
    }

    public static Object readEffectParameter(SharedPreferences pref) {
        String effectSelection = pref.getString(KEY_VIDEO_EFFECT, "none");
        if (effectSelection.equals("none")) {
            return null;
        }
        int separatorIndex = effectSelection.indexOf('/');
        String effectParameter =
                effectSelection.substring(separatorIndex + 1);
        if (effectSelection.startsWith("goofy_face")) {
            if (effectParameter.equals("squeeze")) {
                return EffectsRecorder.EFFECT_GF_SQUEEZE;
            } else if (effectParameter.equals("big_eyes")) {
                return EffectsRecorder.EFFECT_GF_BIG_EYES;
            } else if (effectParameter.equals("big_mouth")) {
                return EffectsRecorder.EFFECT_GF_BIG_MOUTH;
            } else if (effectParameter.equals("small_mouth")) {
                return EffectsRecorder.EFFECT_GF_SMALL_MOUTH;
            } else if (effectParameter.equals("big_nose")) {
                return EffectsRecorder.EFFECT_GF_BIG_NOSE;
            } else if (effectParameter.equals("small_eyes")) {
                return EffectsRecorder.EFFECT_GF_SMALL_EYES;
            }
        } else if (effectSelection.startsWith("backdropper")) {
            // Parameter is a string that either encodes the URI to use,
            // or specifies 'gallery'.
            return effectParameter;
        }

        Log.e(TAG, "Invalid effect selection: " + effectSelection);
        return null;
    }


    public static void restorePreferences(Context context,
            ComboPreferences preferences, Parameters parameters) {
        int currentCameraId = readPreferredCameraId(preferences);

        for (int i=0; i<CameraHolder.instance().getNumberOfCameras(); i++)
        {
            preferences.setLocalId(context, i);
            Editor editor = preferences.edit();
            editor.clear();
            editor.apply();
        }

        // Switch back to the preferences of the current camera. Otherwise,
        // we may write the preference to wrong camera later.
        preferences.setLocalId(context, currentCameraId);

        upgradeGlobalPreferences(preferences.getGlobal());
        upgradeLocalPreferences(preferences.getLocal());

        // Write back the current camera id because parameters are related to
        // the camera. Otherwise, we may switch to the front camera but the
        // initial picture size is that of the back camera.
        initialCameraPictureSize(context, parameters);
        writePreferredCameraId(preferences, currentCameraId);
    }

    private static List<Integer> getSupportedFramerates(Parameters params) {
        final List<Integer> extendedFrameRates = parseToIntList(
                params.get(KEY_SUPPORTED_PREVIEW_FRAMERATES_EXT));
        return extendedFrameRates == null || extendedFrameRates.isEmpty() ?
                params.getSupportedPreviewFrameRates() : // fallback to regular frame rates
                extendedFrameRates;
    }

    private static List<Integer> parseToIntList(String str) {
        if ( str == null ) {
            return null;
        }

        StringTokenizer tokenizer = new StringTokenizer(str, ",");
        ArrayList<Integer> intList = new ArrayList<Integer>();

        while (tokenizer.hasMoreElements()) {
            intList.add(Integer.parseInt(tokenizer.nextToken()));
        }

        return intList;
    }

    public List<String> parseToList(String str) {
        if (null == str)
            return null;

        StringTokenizer tokenizer = new StringTokenizer(str, ",");
        ArrayList<String> substrings = new ArrayList<String>();

        while (tokenizer.hasMoreElements())
            substrings.add(tokenizer.nextToken());

        return substrings;
    }

    private ArrayList<String> getSupportedVideoQuality() {
        ArrayList<String> supported = new ArrayList<String>();
        // Check for supported quality
        if (CamcorderProfile.hasProfile(mCameraId, CamcorderProfile.QUALITY_1080P)) {
            supported.add(Integer.toString(CamcorderProfile.QUALITY_1080P));
        }
        if (CamcorderProfile.hasProfile(mCameraId, CamcorderProfile.QUALITY_720P)) {
            supported.add(Integer.toString(CamcorderProfile.QUALITY_720P));
        }
        if (CamcorderProfile.hasProfile(mCameraId, CamcorderProfile.QUALITY_480P)) {
            supported.add(Integer.toString(CamcorderProfile.QUALITY_480P));
        }

        return supported;
    }

    private void initVideoEffect(PreferenceGroup group, ListPreference videoEffect) {
        CharSequence[] values = videoEffect.getEntryValues();

        boolean goofyFaceSupported =
                EffectsRecorder.isEffectSupported(EffectsRecorder.EFFECT_GOOFY_FACE);
        boolean backdropperSupported =
                EffectsRecorder.isEffectSupported(EffectsRecorder.EFFECT_BACKDROPPER) &&
                mParameters.isAutoExposureLockSupported() &&
                mParameters.isAutoWhiteBalanceLockSupported();

        ArrayList<String> supported = new ArrayList<String>();
        for (CharSequence value : values) {
            String effectSelection = value.toString();
            if (!goofyFaceSupported && effectSelection.startsWith("goofy_face")) continue;
            if (!backdropperSupported && effectSelection.startsWith("backdropper")) continue;
            supported.add(effectSelection);
        }

        filterUnsupportedOptions(group, videoEffect, supported);
    }
}
