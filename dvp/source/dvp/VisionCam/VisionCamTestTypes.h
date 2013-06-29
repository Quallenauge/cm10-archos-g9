/*
 *  Copyright (C) 2009-2011 Texas Instruments, Inc.
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

#ifndef __VCAM_TEST_TYPES__

#define __VCAM_TEST_TYPES__

#include <unistd.h>
#include <time.h>
#include <sosal/sosal.h>
#include <dvp/VisionCam.h>

#if !defined(WIN32) && !defined(UNDER_CE)
  #include <linux/videodev2.h>
#endif

#define ARR_SIZE(arr) (sizeof(arr)/sizeof(arr[0]))

const int NUM_BUFFERS = 5;
const int NUM_BUFFERS_MAX = 8;

/** Menu entry used in menuEntries
  * value containing the index
  * for each menu option
  */
typedef int entryIndex;

/** entryKey is used to specify
  * which string must be entered
  * to invoke a proper command
  */
typedef const char * entryKey;

/** subMenuFn - pointer to a void function(void*)
  * This may be used for some menu functionalities
  * that need more complicated actions
  * than just sending a command and etc.
  */
typedef void (* subMenuFn)( void * );

typedef enum{
    neutralEntry,
    commandEntry,
    paramEntry_set,
    paramEntry_get,
    functionalEntry,

    entryMax
}entryType;

/** Structure defining the allocated buffers.
  *
  */
struct bufferProperties{
    int previewLength;
    unsigned int *previewBufs[ NUM_BUFFERS_MAX ];
    unsigned int * previewOffsets[ NUM_BUFFERS_MAX ];
};

/** Struct that will contain
  * values, meaningful for given setting
  * but outside of its defunition values
  * @param     array containing exeption values
  * @param  should equals size of the array
  * pointed by values
  */
struct exeptionValues{
    const int *values;
    const int numValues;
};
/*  Structure that will help validating values entered.
  *
  */
struct definitionValues{
    int min;
    int max;
    const exeptionValues *exeptions;
};

/*  Structure defining a submenu entry.
  * Only non scalar options ( enumerated )
  * will need this.
  */
struct subMenuEnrty {
    const int * keyValue;
    const char ** optString;
};

/*  Main menu entry type
  *
  */
struct menuEnrty {
    entryKey menuKey;               /// this is the user input
    entryType type;                 /// basicaly - what's this all about
    int ID;                         /// ID of the option, which will be send to GRE component
    const char * itemDescriptor;    /// string for the main menu
    definitionValues defVal;        /// gives the range of possible values
    void * localValue;              /// Saves the last value chosen by user
    void *subMenu;                  /// a sub menu structure, or a pointer to the function that will do the job
};

/**
 * Prototypes
*/
module_t initModule(VisionCam ** gCam );
status_e deinitModule( module_t handle, VisionCam ** gCam );

status_e allocPreviewPortBuffers(VisionCam *vCam, uint32_t width, uint32_t height);
void deallocPreviewPortBufers();

status_e allocVideoPortBuffers(VisionCam *vCam, uint32_t width, uint32_t height);
void deallocVideoPortBufers();

status_e startServices( VisionCam * gCam );
status_e stopServices( VisionCam * gCam );

void setInitialValues( VisionCam * gCam );
entryIndex Menu( const menuEnrty ** menu );
entryIndex getEntryIndex( const menuEnrty ** menu , entryKey desc );
status_e executeEntry( const menuEnrty *choice, VisionCam * gCam );
bool checkValue( const definitionValues * defVal, int value);
int getValue( const menuEnrty *choice );

void *notificationHandler(VisionCamClientNotifier::VisionCamClientNotificationMsg);
void sendBufferTo_V4L(VisionCamFrame *cameraFrame);
void receiveFramePackage(VisionCamFramePack *pack);

void drawFaceBox(VisionCamFrame *cameraFrame);
int getBytesPerPixel();
#endif /// __VCAM_TEST_TYPES__
