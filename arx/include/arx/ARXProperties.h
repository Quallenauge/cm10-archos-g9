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

#ifndef _ARXPROPERTIES_H_
#define _ARXPROPERTIES_H_

namespace tiarx {

enum Properties_e {
    /*!
     * The state of the ARX engine
     * @see EngineState_e
     */
    PROP_ENGINE_STATE,
    /*!
     * Property to configure/query camera id (0 or 1)
     */
    PROP_CAM_ID,
    /*!
     * Property to configure/query camera fps
     */
    PROP_CAM_FPS,
    /*!
     * Property to configure/query the mirroring type used for the camera main output
     * This property only takes effect during a PROP_ENGINE_STATE change to ENGINE_STATE_START.
     * NOTE: Face detect locations are always calculated with no mirroring
     * @see CamMirrorType_e
     */
    PROP_CAM_MAIN_MIRROR,
    /*!
     * Property to configure/query the mirroring type used for the camera secondary output
     * This property only takes effect during a PROP_ENGINE_STATE change to ENGINE_STATE_START.
     * NOTE: Face detect locations are always calculated with no mirroring
     * @see CamMirrorType_e
     */
    PROP_CAM_SEC_MIRROR,


    PROP_CUSTOM_ENGINE_START = 0x10000,
    /*!
     * Property to reset the camera pose engine tracking
     * Value is ignored
     */
    PROP_ENGINE_CAMPOSE_RESET = PROP_CUSTOM_ENGINE_START,

    PROP_ENGINE_FPD_FILTER,
    PROP_ENGINE_FPD_MINMOVE,
    PROP_ENGINE_FPD_MAXMOVE,
    /*!
     * Property to set the confidence level threshold (range: [1 - 1000], where 1000 is the highest
     * confidence).  FPD only returns results when the confidence of all FPD features are higher
     * than the threshold.  The default threshold value is 1.
     */
    PROP_ENGINE_FPD_CONFTHRESH,
};

/** @see Properties_e */
enum EngineState_e {
    /*!
     * Starts the ARX engine (starts camera and performs computations)
     */
    ENGINE_STATE_START,
    /*!
     * Stops the ARX engine (stops camera and stops computations)
     */
    ENGINE_STATE_STOP,
    /*!
     * The ARX engine died (some fatal error)
     * The client will receive a callback when this happens, at which point ARX must be destroyed.
     */
    ENGINE_STATE_DEAD,
};

enum CamMirrorType_e {
    /*!
     * No mirroring applied.
     */
    CAM_MIRROR_NONE,
    /*!
     * Mirroring around horizontal axis
     */
    CAM_MIRROR_VERTICAL,
    /*!
     * Mirroring around vertical axis.
     */
    CAM_MIRROR_HORIZONTAL,
    /*!
     * Mirror around both axes.
     */
    CAM_MIRROR_BOTH,
};

}
#endif //_ARXPROPERTIES_H_
