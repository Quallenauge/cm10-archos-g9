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

#include "VisionCamTestData.h"

#if defined(WIN32) || defined(UNDER_CE)
  #include <windows.h>
#else
  #include <stdio.h>
  #include <stdlib.h>
  #include <math.h>
  #include <string.h>
  #include <sys/types.h>
  #include <sys/stat.h>
  #include <fcntl.h>
  #include <sys/mman.h>
#endif

#include <dvp/dvp.h>
#include <dvp/dvp_display.h>

#define here {printf("\n======> %s <======> %d <======\n", __FILE__, __LINE__);fflush(stdout);}

// #define EXTRA_DATA_CTL_BYTE     15
// #define EXTRA_DATA_CTL_FLAG     0x7F
#define EXTRA_DATA_FILE_LENGHT  12332

#define MODULE_TO_LOAD MODULE_NAME("vcam")
#define SYMBOL_TO_LOAD "VisionCamFactory"
#define VIDEO_DEVICE "/dev/video1"
#include <stdint.h>

const int32_t max_video_port_resolution = VCAM_RES_VGA;
static DVP_Image_t *dvpBuffsDisp = NULL, *dvpBuffsImg = NULL;

#if !defined(ICS) && !defined(JELLYBEAN)
static DVP_Handle hDVP = 0;
#endif

/// Display stuff
static dvp_display_t *dvpd[VCAM_PORT_MAX] = { NULL };
static queue_t *frameQ[VCAM_PORT_MAX] = { NULL };
//thread_ret_t retFrameThread( void *arg );

int main()
{
    VisionCam * gCam = NULL;
    entryIndex index = 0;
    module_t handle = NULL;
    status_e ret = STATUS_SUCCESS;

#if defined(SOSAL_RUNTIME_DEBUG)
    debug_get_zone_mask("SOSAL_ZONE_MASK", &sosal_zone_mask);
#endif

#if defined(DVP_RUNTIME_DEBUG)
    debug_get_zone_mask("DVP_ZONE_MASK", &dvp_zone_mask);
#endif

    handle =  initModule( &gCam );
#ifdef VCAM_AS_SHARED
    if( handle == NULL )
        return -1;
#endif

    if( gCam )
    {
        setInitialValues( gCam );
        ret = startServices( gCam );
    }
    else
    {
        ret = STATUS_CATASTROPHIC;
    }

    if( ret !=  STATUS_SUCCESS )
    {
        ret = deinitModule( handle, &gCam );
    }
    else
    {
        while( 1 )
        {
            index = Menu( menu );
            if( index == -1 )
            {
                continue;
            }

            /// 'q' button (quit) is pressed
            if( menu[ index ]->ID == VCAM_CMD_QUIT )
            {
                index = getEntryIndex( menu , KEY_PREVIEW_STOP );
                ret = executeEntry( menu[ index ], gCam );
                break;
            }
            ret = executeEntry( menu[ index ] , gCam );
        }
        ret = stopServices( gCam );
        ret = deinitModule( handle, &gCam );
    }

    printf("\tvcam_test exiting with %d.\n", ret);

    if( STATUS_SUCCESS != ret )
        puts("\tTerminating application.");

    return ret;
}

#ifdef CAP_FRAME
void capFrame(void *input __attribute__ ((unused)))
{
    recNextFrame = !recNextFrame;

    if( recNextFrame )
    {
        puts("Next frame will be recorded.");
    }
}
#endif // CAP_FRAME

status_e allocPreviewPortBuffers(VisionCam *vCam, uint32_t width, uint32_t height)
{
    status_e ret = STATUS_SUCCESS;

    VisionCamResType res;
    res.mHeight = VisionCamResolutions[prevResIdx].mHeight;
    res.mWidth = VisionCamResolutions[prevResIdx].mWidth;
    res.mResIdx = (VisionCamResolIdex)prevResIdx;

    uint32_t *disp_width, *disp_height;
    if( true_e == show_whole_buffer )
    {
        disp_width = &res.mWidth;
        disp_height = &res.mHeight;
    }
    else
    {
        disp_width = &width;
        disp_height = &height;
    }

    vCam->getParameter(VCAM_PARAM_2DBUFFER_DIM, &res, sizeof(res), VCAM_PORT_PREVIEW);

    if( !dvpd[VCAM_PORT_PREVIEW] )
    {
        dvpd[VCAM_PORT_PREVIEW] = DVP_Display_Create(*disp_width, *disp_height,
                                  res.mWidth, res.mHeight,
                                  DVP_DISPLAY_WIDTH, DVP_DISPLAY_HEIGHT,
                                  res.mWidth, res.mHeight,
                                  0 , 0,
                                  colorSpacePeview, 0, NUM_BUFFERS);
    }

    if (dvpd[VCAM_PORT_PREVIEW])
    {
        dvpBuffsDisp = (DVP_Image_t *)calloc(NUM_BUFFERS, sizeof(DVP_Image_t));

        for ( int i = 0; i < NUM_BUFFERS; i++)
        {
            DVP_Image_Init( &dvpBuffsDisp[i], res.mWidth, res.mHeight, colorSpacePeview );

            if( !DVP_Display_Alloc(dvpd[VCAM_PORT_PREVIEW], &dvpBuffsDisp[i]) )
            {
                printf("Error allocating preview port buffers.\n");
                deallocPreviewPortBufers();
                ret = STATUS_NOT_ENOUGH_MEMORY;
                break;
            }
        }
    }
    else
    {
        printf("Error creating display. No buffers were allocated.\n");
        ret = STATUS_FAILURE;
    }

    if( STATUS_SUCCESS == ret )
        frameQ[VCAM_PORT_PREVIEW] = queue_create(NUM_BUFFERS, sizeof(DVP_Image_t *));

    return ret;
}

void deallocPreviewPortBufers()
{
    if( dvpBuffsDisp )
    {
        for (int i = 0; i < NUM_BUFFERS; i++)
        {
            DVP_Display_Free(dvpd[VCAM_PORT_PREVIEW], &dvpBuffsDisp[i]);
            DVP_Image_Deinit(&dvpBuffsDisp[i]);
        }

        free( dvpBuffsDisp );
        dvpBuffsDisp = NULL;
    }

    if( dvpd[VCAM_PORT_PREVIEW] )
    {
        DVP_Display_Destroy(&dvpd[VCAM_PORT_PREVIEW]);
        dvpd[VCAM_PORT_PREVIEW] = 0;
    }

    if( frameQ[VCAM_PORT_PREVIEW] )
    {
        queue_destroy(frameQ[VCAM_PORT_PREVIEW]);
        frameQ[VCAM_PORT_PREVIEW] = NULL;
    }
}
#if defined(ICS) || defined(JELLYBEAN)
status_e allocVideoPortBuffers(VisionCam *vCam, uint32_t width, uint32_t height)
{
    status_e ret = STATUS_SUCCESS;

    VisionCamResType res;
//    if( VisionCamResolutions[max_video_port_resolution].mHeight < height
//            || VisionCamResolutions[max_video_port_resolution].mWidth < width)
//    {
//        res.mHeight = VisionCamResolutions[max_video_port_resolution].mHeight;
//        res.mWidth = VisionCamResolutions[max_video_port_resolution].mWidth;
//        res.mResIdx = (VisionCamResolIdex)max_video_port_resolution;
//    }
//    else
    {
        res.mHeight = height;
        res.mWidth = width;
        res.mResIdx = (VisionCamResolIdex)prevResIdx;
    }
    uint32_t *disp_width, *disp_height;
    if( true_e == show_whole_buffer )
    {
        disp_width = &res.mWidth;
        disp_height = &res.mHeight;
    }
    else
    {
        disp_width = &width;
        disp_height = &height;
    }

    vCam->getParameter(VCAM_PARAM_2DBUFFER_DIM, &res, sizeof(res), VCAM_PORT_VIDEO);

    if( !dvpd[VCAM_PORT_VIDEO] )
    {
        dvpd[VCAM_PORT_VIDEO] = DVP_Display_Create(*disp_width, *disp_height,
                                  res.mWidth, res.mHeight,
                                  DVP_DISPLAY_WIDTH, DVP_DISPLAY_HEIGHT,
                                  res.mWidth, res.mHeight,
                                  0 , 0,
                                  colorSpaceVideo, 0, NUM_BUFFERS);
    }

    if( dvpd[VCAM_PORT_VIDEO] )
    {
        dvpBuffsImg = (DVP_Image_t *)calloc(NUM_BUFFERS, sizeof(DVP_Image_t));

        for ( int i = 0; i < NUM_BUFFERS; i++)
        {
            DVP_Image_Init( &dvpBuffsImg[i], res.mWidth, res.mHeight, colorSpaceVideo );

            if( !DVP_Display_Alloc(dvpd[VCAM_PORT_VIDEO], &dvpBuffsImg[i]) )
            {
                printf("Error allocating video display buffers.\n");
                deallocVideoPortBufers();
                ret = STATUS_NOT_ENOUGH_MEMORY;
                break;
            }
            DVP_PrintImage(DVP_ZONE_ALWAYS, &dvpBuffsImg[i]);
        }
    }
    else
    {
        printf("Error creating display. No buffers were allocated.\n");
        ret = STATUS_FAILURE;
    }

    if( STATUS_SUCCESS == ret )
        frameQ[VCAM_PORT_VIDEO] = queue_create(NUM_BUFFERS, sizeof(DVP_Image_t *));

    return ret;
}

void deallocVideoPortBufers()
{
    if( dvpBuffsImg )
    {
        for (int i = 0; i < NUM_BUFFERS; i++)
        {

            if( DVP_Display_Free( dvpd[VCAM_PORT_VIDEO], &dvpBuffsImg[i]) )
            {
                DVP_Image_Deinit(&dvpBuffsImg[i]);
            }
        }

        free( dvpBuffsImg );
        dvpBuffsImg = NULL;
    }

    if( dvpd[VCAM_PORT_VIDEO] )
    {
        DVP_Display_Destroy(&dvpd[VCAM_PORT_VIDEO]);
        dvpd[VCAM_PORT_VIDEO] = 0;
    }

    if( frameQ[VCAM_PORT_PREVIEW] )
    {
        queue_destroy(frameQ[VCAM_PORT_VIDEO]);
        frameQ[VCAM_PORT_VIDEO] = NULL;
    }
}
#else
status_e allocVideoPortBuffers(VisionCam *vCam, uint32_t width, uint32_t height)
{
    status_e ret = STATUS_SUCCESS;

    hDVP = DVP_KernelGraph_Init();

    if ( hDVP )
    {
        dvpBuffsImg = (DVP_Image_t *)calloc(NUM_BUFFERS, sizeof(DVP_Image_t));
        for ( int i = 0; i < NUM_BUFFERS; i++)
        {
            DVP_Image_Init( &dvpBuffsImg[i],
                            width,
                            height,
                            colorSpaceVideo);

            if( !DVP_Image_Alloc( hDVP, &dvpBuffsImg[i], DVP_MTYPE_MPUNONCACHED_2DTILED ) )
            {
                printf("Error allocating image buffers.\n");
                deallocVideoPortBufers();

                ret = STATUS_NOT_ENOUGH_MEMORY;
                break;
            }
            printf("vid biffs: w x h ; %d x %d\n", dvpBuffsImg[0].height, dvpBuffsImg[0].width);
        }
    }
    else
    {
        printf("Error getting DVP handle No buffers were allocated.\n");
        ret = STATUS_FAILURE;
    }

    if( STATUS_SUCCESS == ret )
        frameqImg = queue_create( NUM_BUFFERS, sizeof(DVP_Image_t *) );

    return ret;
}

void deallocVideoPortBufers()
{
    if( dvpBuffsImg )
    {
        for (int i = 0; i < NUM_BUFFERS; i++)
        {
            DVP_Image_Free( hDVP, &dvpBuffsImg[i]);
            DVP_Image_Deinit(&dvpBuffsImg[i]);
        }

        free( dvpBuffsImg );
        dvpBuffsImg = NULL;
    }

    if( hDVP )
    {
        DVP_KernelGraph_Deinit(hDVP);
        hDVP = 0;
    }

    if( frameqImg )
    {
        queue_destroy(frameqImg);
        frameqImg = NULL;
    }
}

#endif // ICS

status_e startServices( VisionCam * vCam )
{
    status_e ret = STATUS_SUCCESS;

    if( vCam != NULL)
    {
        VisionCamResType resPreview, resVideo;

        resPreview.mHeight = VisionCamResolutions[prevResIdx].mHeight;
        resPreview.mWidth = VisionCamResolutions[prevResIdx].mWidth;
        resPreview.mResIdx = (VisionCamResolIdex)prevResIdx;

        // Check required resolution and change the video port resolution to its maxumum, in case it is higher
        if( VisionCamResolutions[prevResIdx].mHeight > VisionCamResolutions[VCAM_RES_VGA].mHeight ||
            VisionCamResolutions[prevResIdx].mWidth > VisionCamResolutions[VCAM_RES_VGA].mWidth )
        {
            resVideo.mHeight = VisionCamResolutions[VCAM_RES_VGA].mHeight;
            resVideo.mWidth = VisionCamResolutions[VCAM_RES_VGA].mWidth;
            resVideo.mResIdx = VCAM_RES_VGA;
        }
        else
        {
            resVideo.mHeight = VisionCamResolutions[prevResIdx].mHeight;
            resVideo.mWidth = VisionCamResolutions[prevResIdx].mWidth;
            resVideo.mResIdx = (VisionCamResolIdex)prevResIdx;
        }
        // allocate and register Preview Port buffers

        if( !dvpBuffsDisp && ret == STATUS_SUCCESS )
        {
            ret = allocPreviewPortBuffers( vCam, resPreview.mWidth, resPreview.mHeight);
            if(ret == STATUS_SUCCESS )
                ret = vCam->useBuffers( dvpBuffsDisp, NUM_BUFFERS, VCAM_PORT_PREVIEW);
            else
                printf("allocPreviewPortBuffers() returned %d (ox%x)\n", ret, ret);
        }

        // allocate and register Video Port buffers
        if( !dvpBuffsImg && ret == STATUS_SUCCESS )
        {
            ret = allocVideoPortBuffers( vCam, resVideo.mWidth, resVideo.mHeight );
            if( ret == STATUS_SUCCESS )
                ret = vCam->useBuffers( dvpBuffsImg, NUM_BUFFERS, VCAM_PORT_VIDEO );
            else
                printf("allocVideoPortBuffers() returned %d (ox%x)\n", ret, ret);
        }

        if( ret ==  STATUS_SUCCESS )
        {
            bool_e use = ( usePackaging ? true_e : false_e);
            ret = vCam->sendCommand(VCAM_CMD_PACK_FRAMES, &use, sizeof(bool_e), VCAM_PORT_ALL);
        }

        if( ret ==  STATUS_SUCCESS )
            ret = vCam->enablePackedFramesCbk(receiveFramePackage);

        if( ret ==  STATUS_SUCCESS )
            ret = vCam->enablePreviewCbk( sendBufferTo_V4L );

        // enable all notifications
        if( ret ==  STATUS_SUCCESS )
            ret = vCam->sendCommand(VCAM_CMD_SET_CLIENT_NOTIFICATION_CALLBACK, (void*)notificationHandler );
    }
    else
        ret = STATUS_FAILURE;

    return ret;
}

status_e stopServices( VisionCam * vCam )
{
    status_e result = STATUS_SUCCESS;
    result = vCam->releaseBuffers(VCAM_PORT_ALL);

    if( dvpBuffsDisp )
    {
        deallocPreviewPortBufers();
    }

    if( dvpBuffsImg )
        deallocVideoPortBufers();

    if( STATUS_SUCCESS == result )
        result = vCam->disablePreviewCbk(sendBufferTo_V4L);

    if( STATUS_SUCCESS == result )
        result = vCam->disablePackedFramesCbk(receiveFramePackage);

    if( STATUS_SUCCESS == result ) /// disable all notifications
        result = vCam->sendCommand(VCAM_CMD_SET_CLIENT_NOTIFICATION_CALLBACK, NULL );

    return result;
}

void *notificationHandler(VisionCamClientNotifier::VisionCamClientNotificationMsg msg)
{
    void *ret = NULL;

    switch( msg )
    {
        case VisionCamClientNotifier::VCAM_MESSAGE_ALLOCATE_V4L2_BUFFERS:
        {
            break;
        }
        case VisionCamClientNotifier::VCAM_MESSAGE_PREEMPT_SUSPEND_ACTIVITY:
        {
            break;
        }

        case VisionCamClientNotifier::VCAM_MESSAGE_PREEMPT_RESUME_ACTIVITY:
        {
            break;
        }

        case VisionCamClientNotifier::VCAM_MESSAGE_PREEMPT_WAIT_RESOURCES:
        {
            break;
        }

        case VisionCamClientNotifier::VCAM_MESSAGE_PREAMPT_RESOURCES_READY:
        {
            break;
        }

        case VisionCamClientNotifier::VCAM_MESSAGE_MAX:
        case VisionCamClientNotifier::VCAM_MESSAGE_EMPTY:
        {
            break;
        }
        case VisionCamClientNotifier::VCAM_MESSAGE_STOP_STREAMING:
        {
            break;
        }

        default:
        {
            ret = NULL;
            break;
        }
    }

    return ret;
}

#ifdef VCAM_AS_SHARED
module_t initModule(VisionCam ** gCam )
{
    module_t handle;
    VisionCamFactory_f factory;

    handle = module_load( (char*)MODULE_TO_LOAD );

    if (handle != NULL)
    {
        module_error();
        factory = (VisionCamFactory_f)module_symbol( handle, (char*)SYMBOL_TO_LOAD );

        *gCam = factory();

        if (gCam == NULL)
        {
            printf("Error getting handle to Vision Camera.\n");
            handle = NULL;
        }
        else
        {
            (*gCam)->init(frameq);

        }
    }
    else
    {
        printf("error while opening gesture cam library.\n");
        handle = NULL;
    }
    return handle;
}

status_e deinitModule( module_t handle, VisionCam ** gCam)
{
    status_e ret = STATUS_SUCCESS;

    if( gCam != NULL )
    {
        ret = (*gCam)->deinit();
    }

    if( ret == STATUS_SUCCESS )
    {
        delete (*gCam);
    }
    else
    {
        printf("gesture cam not deinitialized !!!\n");
    }

    if( handle )
    {
        module_unload( handle );
    }

    return ret;
}
#else
module_t initModule(VisionCam ** gCam )
{
    VisionCam *pCam = NULL;
    status_e initErr = STATUS_SUCCESS;
    *gCam = NULL;
    pCam = VisionCamFactory(VISIONCAM_OMX);
    if (pCam)
    {
        initErr = pCam->init(frameQ[VCAM_PORT_PREVIEW]);

        if( initErr != STATUS_SUCCESS )
            *gCam = NULL;
        else
            *gCam = pCam;
    }
    return NULL;
}
status_e deinitModule(module_t handle __attribute__ ((unused)) ,
                 VisionCam * * gCam)
{
    if (*gCam)
    {
        (*gCam)->deinit();
        delete *gCam;
        *gCam = NULL;
    }
    return STATUS_SUCCESS;
}
#endif

status_e executeEntry( const menuEnrty *choice, VisionCam * gCam )
{
    status_e result = STATUS_SUCCESS;
    switch( choice->type )
    {
        case commandEntry:
        {
            int val = 0;
            printf("Command: \"%s\" ( ID = %d )\n", choice->itemDescriptor, choice->ID);
            val = getValue( choice );
            result = gCam->sendCommand( (VisionCamCmd_e)choice->ID, (void *)&val, sizeof(int) , currentPort);
            break;
        }
        case paramEntry_set:
        {
            int val = getValue( choice );
            result = gCam->setParameter( (VisionCamParam_e)choice->ID, (void *)&val, sizeof(int) , currentPort);
            break;
        }
        case paramEntry_get:
        {
            int32_t index = 0;
            int32_t paramVal = (uint32_t)(-1);
            int32_t paramID = getValue( choice );

            result = gCam->getParameter( (VisionCamParam_e)paramID, (void *)&paramVal, sizeof(int32_t) , currentPort);

            if( STATUS_SUCCESS == result )
            {
                while( ((subMenuEnrty*)choice->subMenu)->keyValue[index] != paramID )
                    index++;

                if( index <= choice->defVal.max )
                    printf("\n%s is set to %d (0x%x) on preview port\n", ((subMenuEnrty*)choice->subMenu)->optString[index], paramVal, paramVal);
            }
            else
                printf("Couldn't get this param.");

            break;
        }
        case functionalEntry:
        {
            subMenuFn executor = (subMenuFn)(choice->subMenu);
            if( executor )
                executor( gCam );
            else
            {
                printf("VisionCamTest error: NULL function pointer passed.\n");
                printf("Request cancelled.\n");
            }
            break;
        }
        case neutralEntry:
        {
            getValue( choice );
            break;
        }
        default:
        {
            printf("executeEntry: wrong param %d", choice->type );
            result = STATUS_INVALID_PARAMETER;
            break;
        }
    }

    return result;
}

void setInitialValues( VisionCam * gCam )
{
    int index = 0;
    bool_e setManualInitValues = false_e;

    if( setManualInitValues )
        printf("Please set first these values.\n");

    while( setManualInitValues )
    {
        index = Menu( menu_Primary );

        if( index < 0 )
            continue;

        /// 'q' button (quit) is pressed
        if( menu_Primary[ index ]->ID == VCAM_CMD_QUIT )
            break;

        executeEntry( menu_Primary[ index ] , gCam );
    }

    for ( int i = 0 ; menu_Primary[i]->ID != VCAM_CMD_QUIT ; i++ )
    {
        if( menu_Primary[i]->ID < VCAM_PARAM_MAX && menu_Primary[i]->ID > VCAM_PARAM_MIN )
            gCam->setParameter((VisionCamParam_e)menu_Primary[i]->ID,
                               menu_Primary[i]->localValue,
                               sizeof(int),
                               currentPort
                               );
    }

}

entryIndex Menu( const menuEnrty ** menu )
{
    entryIndex index = 0;
    char choice [64];

    printf("\n\n");
    printf("\t###========================================================###\n");
    printf("\t#                   Vision Camera Test Menu                  #\n");
    printf("\t###========================================================###\n");
    printf("\n\n");
    /// print the main menu
    while( menu[ index ] != NULL )
    {
        printf("\t\t%-2s. %s\n", menu[ index ]->menuKey, menu[ index ]->itemDescriptor);
        index++;
    }

    scanf( "%s", choice );
    printf("\nChoosen: %s\n", choice );

    index = getEntryIndex( menu , (entryKey)choice );

    if( index == -1 )
        printf("Incorrect Input: %s", choice );

    return index;
}

entryIndex getEntryIndex( const menuEnrty ** menu , entryKey desc )
{
    /// get the index of item choosen
    entryIndex index = 0;
    while( menu[ index ] != NULL )
    {
        if( strcmp( ( entryKey )desc, menu[ index ]->menuKey ) == 0 )
        {
            break;
        }
        index++;
    }

    if ( menu[ index ] == NULL )
        index = -1;

    return index;
}

bool checkValue( const definitionValues * defVal, int value)
{
    bool isExeption = false;

    if( defVal->exeptions )
        for( int i = 0; i < defVal->exeptions->numValues && isExeption == false; i++)
            isExeption = ( defVal->exeptions->values[i] == value );

    return (value >= defVal->min && value <= defVal->max) || isExeption;
}

int getValue( const menuEnrty *choice )
{
    int value;
    bool isCorrect = false;

    if( choice->subMenu  )
    {
        for( int i = 0; i <= choice->defVal.max - choice->defVal.min; i++ )
            printf("%d.\t%s\n", i , (( subMenuEnrty* )(choice->subMenu))->optString[ i ] );
    }
    else
    {
        if (choice->defVal.min == choice->defVal.max)
            return choice->defVal.min;

        printf("%s\n", choice->itemDescriptor );
        printf("enter a numer between %d and %d\n", choice->defVal.min, choice->defVal.max );
    }

    for(;;)
    {
        fflush(stdin);
        scanf( "%d", &value  );

        isCorrect = checkValue( &choice->defVal , value );

        if( isCorrect )
        {
            if( choice->subMenu  )
                value = (( subMenuEnrty* )(choice->subMenu))->keyValue[ value ];

            if( choice->localValue )
                memcpy( choice->localValue, &value, sizeof(value) );
            break;
        }

        printf("incorrect input");
        printf("possible values are in the range of: %d and %d\n", choice->defVal.min, choice->defVal.max);
    }

    printf("value = %d (0x%x)\n", value, value);

    return value;
}

void setCropParams( void * input )
{
    char descr[32];

    menuEnrty crop = {
        0,
        neutralEntry,
        VCAM_PARAM_CROP,
        descr,
        { 0 , 0, NULL },
        NULL,
        NULL
    };

    descr[0] = 0;

    crop.defVal.min = 0;
    crop.defVal.max = VisionCamResolutions[prevResIdx].mWidth;
    crop.itemDescriptor = "\nset crop left offset:";
    cropParams.mLeft = getValue( &crop );

    crop.defVal.min = 0;
    crop.defVal.max = VisionCamResolutions[prevResIdx].mHeight;
    crop.itemDescriptor = "\nset crop top offset: ";
    cropParams.mTop = getValue( &crop );

    crop.defVal.min = 10; /// @todo define proper value
    crop.defVal.max = VisionCamResolutions[prevResIdx].mWidth - cropParams.mLeft;
    crop.itemDescriptor = "\nset crop width: ";
    cropParams.mWidth = getValue( &crop );

    crop.defVal.min = 10; /// @todo define proper value
    crop.defVal.max = VisionCamResolutions[prevResIdx].mHeight - cropParams.mTop;
    crop.itemDescriptor = "\nset crop height: ";
    cropParams.mHeight = getValue( &crop );

    ((VisionCam*)input)->setParameter( (VisionCamParam_e)crop.ID, (void *)&cropParams, sizeof(VisionCamRectType) );
}

void setVariableFrameRate( void * input )
{
    menuEnrty minFrate, maxFrate;

    minFrate.itemDescriptor = "min frame rate";
    minFrate.defVal.min = 0;
    minFrate.defVal.max = 30;
    minFrate.defVal.exeptions = NULL;
    minFrate.type = paramEntry_set;
    minFrate.localValue = NULL;///varFrate.mMin;
    minFrate.subMenu = NULL;

    maxFrate.itemDescriptor = "max frame rate";
    maxFrate.defVal.min = 1;
    maxFrate.defVal.max = 30;
    maxFrate.defVal.exeptions = NULL;
    maxFrate.type = paramEntry_set;
    maxFrate.localValue = NULL;///varFrate.mMax;
    maxFrate.subMenu = NULL;

    varFrate.mMin = getValue( &minFrate );
    varFrate.mMax = getValue( &maxFrate );

    ((VisionCam*)input)->setParameter( (VisionCamParam_e)frameRate_variable.ID ,
                                        (void *)&varFrate ,
                                        sizeof( VisionCamVarFramerateType )
                                      );
}

uint32_t getQ16Value(const char * itemDescriptor, uint16_t min, uint16_t max)
{
    uint32_t retVal = 0;
    uint32_t test_value;
    int  i, cnt_f, cnt_i, frac;
    bool isValid;
    char in[64];
    char subin_frac[64];
    char subin_int[64];

    if(min >= max) {
        printf("ERROR: min: %d max: %d\n", min, max);
        return 0;
    }

    isValid = false;
    for(; !isValid;) {
        printf("%s [%d, %d]:\n", itemDescriptor, min, max);
        fflush(stdin);
        scanf( "%s", in );

        i = 0;
        frac = 0;
        cnt_f = 0;
        cnt_i = 0;
        isValid = true;
        while(in[i] != '\0')
        {
            if(in[i] == '.') {
                i++;
                frac++;
                continue;
            }

            if(in[i] < '0' || in[i] > '9') {
                isValid = false;
                break;
            }

            if(frac == 0) {
                subin_int[cnt_i++] = in[i];
            } else if(frac == 1) {
                subin_frac[cnt_f++] = in[i];
            } else {
                isValid = false;
                break;
            }

            i++;
        }

        subin_int[cnt_i]  = '\0';
        subin_frac[cnt_f] = '\0';

        if((cnt_i == 0) || (isValid == false)) {
            isValid = false;
            continue;
        }

        test_value = atoi(subin_int);
        retVal = test_value << 16;
        if(cnt_f) {
            int dev;
            for(dev = 1, i = 0; i < cnt_f; i++) {
                dev *= 10;
            }
            retVal |= (atoi(subin_frac) * (uint32_t)0xFFFF) / dev;
            if(atoi(subin_frac)) {
                test_value++;
            }
        }

        if(test_value < min || test_value > max) {
            printf("Value %s.%s is not in range [%d, %d]", subin_int, subin_frac, min, max);
            isValid = false;
            continue;
        }
    }

    return retVal;
}

void setImagePyramid( void * input )
{
    VisionCamImagePyramidType imagePyramid;
    menuEnrty LevelsCount;

    LevelsCount.itemDescriptor = "Image pyramid levels";
    LevelsCount.defVal.min = 0;
    LevelsCount.defVal.max = 16;
    LevelsCount.defVal.exeptions = NULL;
    LevelsCount.type = paramEntry_set;
    LevelsCount.localValue = NULL;
    LevelsCount.subMenu = NULL;

    imagePyramid.mLevelsCount = getValue( &LevelsCount );

    imagePyramid.mScalingFactor = 1 << 16;
    if (imagePyramid.mLevelsCount > 1) {
        imagePyramid.mScalingFactor = getQ16Value("Image pyramid scaling factor [1-16] (for example: 1.5)", 1, 16);
    }

    ((VisionCam*)input)->setParameter( (VisionCamParam_e)setImgPyramid.ID,
                                        (void *)&imagePyramid,
                                        sizeof( VisionCamImagePyramidType ),
                                        VCAM_PORT_VIDEO
                                      );
}

void setPreviewResolution( void * input )
{
    VisionCam* vcam = (VisionCam*)input;
    int32_t result = STATUS_SUCCESS;
    int res;
    bool_e restartPreview = false_e;

    /// choose new resolution
    /// stop preview if running
    /// release buffers
    /// set new resolution
    /// alloc new buffers
    /// start preview if it was previously running
    do {
        for(int i = 0; i < VCAM_RESOL_MAX; i++ )
            printf("%d.\t%d x %d\n", i, VisionCamResolutions[i].mWidth, VisionCamResolutions[i].mHeight );

        scanf("%d", &res);

        if( res < 0 && res >= VCAM_RESOL_MAX )
            printf("incorrect input; possible values are between %d and %d\n", 0, VCAM_RESOL_MAX - 1);

    } while ( res < 0 && res >= VCAM_RESOL_MAX );

    result = vcam->setParameter( VCAM_PARAM_RESOLUTION , (void*)&res, sizeof(int), currentPort );
    if( STATUS_INVALID_STATE == result)
    {
        if STATUS_PASSED(vcam->sendCommand( VCAM_CMD_PREVIEW_STOP , 0, 0, currentPort) )
        {
            restartPreview = true_e;
            result = vcam->setParameter( VCAM_PARAM_RESOLUTION , (void*)&res, sizeof(int), currentPort);
        }
    }

    if( STATUS_SUCCESS == result ) {
        result = stopServices( vcam );
    }

    if( STATUS_SUCCESS == result )
    {
        prevResIdx = res;
        result = startServices( vcam );
    }
    else
    {
        printf("Error setting preview resolution");
    }

    if( restartPreview && STATUS_SUCCESS == result )
        result = vcam->sendCommand(VCAM_CMD_PREVIEW_START, 0, 0, currentPort);
}

void setFaceDetect( void * input )
{
    VisionCam* vcam = (VisionCam*)input;
    faceDetectEnabled = !faceDetectEnabled;
    bool_e en = ( faceDetectEnabled ? true_e : false_e);
    vcam->sendCommand( VCAM_CMD_FACE_DETECTION, &en, sizeof(bool_e) );
    vcam->sendCommand( VCAM_CMD_FACE_DETECTION_RAW, &en, sizeof(bool_e) );
}

#ifndef EXPORTED_3A

void whiteBalGains( void * input )
{
    VisionCam* vcam = (VisionCam*)input;
    VisionCamWhiteBalGains gainValues;
    menuEnrty wbColorGains = wbGains;

    wbColorGains.subMenu = NULL;

    wbColorGains.itemDescriptor = "\nEnter gain for RED color";
    gainValues.mRed = getValue((const menuEnrty*)&wbColorGains);

    wbColorGains.itemDescriptor = "\nEnter gain for GREEN RED color";
    gainValues.mGreen_r = getValue((const menuEnrty*)&wbColorGains);

    wbColorGains.itemDescriptor = "\nEnter gain for GREEN BLUE color";
    gainValues.mGreen_b = getValue((const menuEnrty*)&wbColorGains);

    wbColorGains.itemDescriptor = "\nEnter gain for BLUE color";
    gainValues.mBlue = getValue((const menuEnrty*)&wbColorGains);

    vcam->setParameter( (VisionCamParam_e)wbGains.ID,
                        (void *)&gainValues,
                        sizeof(VisionCamWhiteBalGains)
                      );
}

void whiteBalGains_get( void * input )
{
    int ret = STATUS_SUCCESS;
    VisionCam* vcam = (VisionCam*)input;
    VisionCamWhiteBalGains wbGains;
    memset(&wbGains, 0, sizeof(VisionCamWhiteBalGains));

    ret = vcam->getParameter((VisionCamParam_e)wbGains_get.ID ,
                             (void *)&wbGains,
                             sizeof(VisionCamWhiteBalGains));
    if( STATUS_SUCCESS == ret )
    {
        printf("gain red         = %d\n", wbGains.mRed    );
        printf("gain green red   = %d\n", wbGains.mGreen_r);
        printf("gain green blue  = %d\n", wbGains.mGreen_b);
        printf("gain blue        = %d\n", wbGains.mBlue   );
    }
    else
        printf("Error getting WB gains. Error = %d\n", ret);
}

void gammaTablesTest( void * input )
{
    VisionCam* vcam = (VisionCam*)input;
    VisionCamGammaTableType gTable;
    static unsigned int a = 3;

    vcam->getParameter((VisionCamParam_e)gammaTbl_test.ID,
                      (void*)&gTable,
                      sizeof(VisionCamGammaTableType));

    if( a%2 )
        memset(gTable.mRedTable, 0, gTable.mTableSize);
    else
        gTable.mRedTable = NULL;/// this will cause gamma reset

    a++;
    vcam->setParameter((VisionCamParam_e)gammaTbl_test.ID,
                       (void*)&gTable,
                       sizeof(VisionCamGammaTableType));
}
#endif // EXPORTED_3A

void setStereoLayout( void * input )
{
    VisionCam * vcam = (VisionCam*)input;
    menuEnrty lout;

    lout.ID = stereoLayout.ID;
//    lout.defVal = stereoLayout.defVal;
//    lout.itemDescriptor = stereoLayout.itemDescriptor;
//    lout.menuKey = stereoLayout.menuKey;
//    lout.type = neutralEntry;
//    lout.localValue = &layout.layout;
//    lout.subMenu = (void*)&stereoLayoutSubMenu;

//    layout.layout = (VisionCamStereoLayout)getValue( &lout );

    VisionCamStereoInfo l;
    l.layout = VCAM_STEREO_LAYOUT_TOPBOTTOM;
    l.subsampling = 1;

    int i = vcam->setParameter((VisionCamParam_e)lout.ID, &l, sizeof(l), currentPort);
    if( i != STATUS_SUCCESS ) printf("error set stsreo layout: %d \n", i);

}

void enableFramePack( void *input )
{
    input = input;
    usePackaging = !usePackaging;
    if( usePackaging ) puts("\tframe pack enabled");
    else puts("\tframe pack disabled");
}

bool save2Dframe( VisionCamFrame *fr)
{
    bool success = true;
    DVP_Image_t *pImage = NULL;
    static int cur = -1;
    DVP_U08 *pos = NULL;
    char filename[33];
    FILE *outFile = NULL;

    cur++;
    if( cur%10 )
        return false;

    sprintf(filename, "/sdcard/vcam_frame_%05d.yuv", cur);
    outFile = fopen( filename, "wb");

    if( outFile )
    {
        pImage = (DVP_Image_t *)fr->mFrameBuff;

        // beginning of meaningful image data
        pos = DVP_Image_Addressing(pImage, fr->mOffsetX, fr->mOffsetY, 0);

        size_t widthInBytes = DVP_Image_LineSize(pImage, 0);

        for( uint32_t h = 0; h < fr->mHeight; h++ , pos += pImage->y_stride )
        {
            if( widthInBytes != fwrite(pos, 1, widthInBytes, outFile) )
            {
                success = false;
                break;
            }
        }

        if( success )
        {
            printf("%s written !\n", filename);
        }

        fclose(outFile);
    }
    else
    {
        success = false;
    }

    return success;
}
#ifdef CAP_FRAME
bool save2Dimage( DVP_Image_t *pImage)
{
    bool success = true;

    if( recNextFrame )
    {
        static int cur = -1;
        DVP_U08 *pos = NULL;
        char filename[33];
        FILE *outFile = NULL;

        cur++;
        if( cur%100 )
        {
            return false;
        }

        sprintf(filename, "/sdcard/vcam_image_%05d.yuv", cur);
        outFile = fopen( filename, "wb");

        if( outFile )
        {
            // beginning of meaningful image data
            pos = DVP_Image_Addressing(pImage, 0, 0, 0);

            size_t widthInBytes = DVP_Image_PatchLineSize(pImage, 0);

            for( uint32_t h = 0; h < pImage->bufHeight; h++ , pos += pImage->y_stride )
            {
                if( widthInBytes != fwrite(pos, 1, widthInBytes, outFile) )
                {
                    success = false;
                    break;
                }
            }

            if( success )
            {
                printf("%s written !\n", filename);
            }
            else
            {
                printf("Couldn't write raw image o a file.\n");
            }

            fflush(outFile);
            fclose(outFile);
        }
        else
        {
            printf("Couldn't write raw image o a file.\n");
            printf("Couild not open a file for writing!!!\n");
            success = false;
        }

        recNextFrame = false;
    }

    return success;
}
#endif // CAP_FRAME

void setDislpayableFrame(void *vCam __attribute__((unused)))
{
    menuEnrty e;
    memcpy(&e, &setDispFrame, sizeof(menuEnrty));
    e.type = paramEntry_set;
    e.subMenu = (void*)&dispFrameSubmenu;
    /**((int*)e.localValue) = */getValue(&e);
}

void sendBufferTo_V4L(VisionCamFrame *cameraFrame)
{
    DVP_Image_t *pImage = (DVP_Image_t *)cameraFrame->mFrameBuff;
    static int32_t frameCount[VCAM_PORT_MAX - VCAM_PORT_MIN] = {0};

    if( displayedFrame == cameraFrame->mFrameSource )
    {
#ifdef CAP_FRAME
    if( recNextFrame )
    {
        save2Dimage(pImage);
    }
#endif
        if( frameQ[displayedFrame] )
        {
            queue_write(frameQ[displayedFrame], true_e, &pImage);
            if( pImage != NULL && queue_read(frameQ[displayedFrame], true_e, &pImage) )
                DVP_Display_Render(dvpd[displayedFrame], pImage);
        }
    }

    ++frameCount[cameraFrame->mFrameSource - VCAM_PORT_MIN];

//#define PRINT_FACE_COORDS
#ifdef PRINT_FACE_COORDS
    printf("\rfaces caught : %d , %d", cameraFrame->mDetectedFacesNum, cameraFrame->mDetectedFacesNumRaw);
    for(int i = 0; i < cameraFrame->mDetectedFacesNum; i++)
    {
        printf("\nface %d [T,L,W,H]:%d,%d,%d,%d\n", i/*cameraFrame->mDetectedFacesNum*/,
                                                  cameraFrame->mFaces[i].mFacesCoordinates.mTop,
                                                  cameraFrame->mFaces[i].mFacesCoordinates.mLeft,
                                                  cameraFrame->mFaces[i].mFacesCoordinates.mWidth,
                                                  cameraFrame->mFaces[i].mFacesCoordinates.mHeight
                                              );

    }
    for(int i = 0; i < cameraFrame->mDetectedFacesNumRaw ; i++)
    {
        printf("\nface RAW %d [T,L,W,H]:%d,%d,%d,%d\n", i/*cameraFrame->mDetectedFacesNumRaw*/,
                                                  cameraFrame->mFacesRaw[i].mFacesCoordinates.mTop,
                                                  cameraFrame->mFacesRaw[i].mFacesCoordinates.mLeft,
                                                  cameraFrame->mFacesRaw[i].mFacesCoordinates.mWidth,
                                                  cameraFrame->mFacesRaw[i].mFacesCoordinates.mHeight
                                              );

    }

#endif
#define PRINT_WB_DATA
#ifdef PRINT_WB_DATA
    if( cameraFrame->mMetadata.mAutoWBGains)
    {
        printf("Auto WB [R,Gr,Gb,B]:%10d,%10d,%10d,%10d\n",
                                                cameraFrame->mMetadata.mAutoWBGains->mRed,
                                                cameraFrame->mMetadata.mAutoWBGains->mGreen_r,
                                                cameraFrame->mMetadata.mAutoWBGains->mGreen_b,
                                                cameraFrame->mMetadata.mAutoWBGains->mBlue
                                          );
    }

    if( cameraFrame->mMetadata.mManualWBGains)
    {
        printf("Manual WB [R,Gr,Gb,B]:%10d,%10d,%10d,%10d\n",
                                                cameraFrame->mMetadata.mManualWBGains->mRed,
                                                cameraFrame->mMetadata.mManualWBGains->mGreen_r,
                                                cameraFrame->mMetadata.mManualWBGains->mGreen_b,
                                                cameraFrame->mMetadata.mManualWBGains->mBlue
                                          );
    }
#endif
#define PRINT_ANCILLARY
#ifdef PRINT_ANCILLARY
        if( cameraFrame->mMetadata.mAncillary)
        {
            printf("Ancillary [Fr#,Exp,Ag,ISO]:%10d,%10d,%10d,%10d\n",
                                                    cameraFrame->mMetadata.mAncillary->nFrameNumber,
                                                    cameraFrame->mMetadata.mAncillary->nExposureTime,
                                                    cameraFrame->mMetadata.mAncillary->nAnalogGainValue,
                                                    cameraFrame->mMetadata.mAncillary->nCurrentISO
                                              );
        }
#endif
//#define PRINT_HISTOGRAM
#ifdef PRINT_HISTOGRAM
        if( cameraFrame->mMetadata.mHistogram2D)
        {
            printf("Histogram2D [#Bins, Type]:%d,%d\n",
                                                    cameraFrame->mMetadata.mHistogram2D->nBins,
                                                    cameraFrame->mMetadata.mHistogram2D->eComponentType
                                              );
            for(uint32_t i = 0; i < cameraFrame->mMetadata.mHistogram2D->nBins; i++)
                printf("%3d,", cameraFrame->mMetadata.mHistogram2D->data[i]);
        }
#endif

#define PRINT_HISTOGRAM_GAMMA
#ifdef PRINT_HISTOGRAM_GAMMA
        if( 0 && 100 == frameCount[cameraFrame->mFrameSource - VCAM_PORT_MIN] )
        {
            if( cameraFrame->mMetadata.mGammaL)
            {
                printf("Gamma L [#items]:%d\n",
                                                        cameraFrame->mMetadata.mGammaL->nItems/*,
                                                        cameraFrame->mMetadata.mGammaL*/
                                                  );
                for(uint32_t i = 0; i < ( cameraFrame->mMetadata.mGammaL->nItems ) / 16 ; i+=16)
                {
                    puts("");
                    for(int j = 0; j < 16; j++)
                        printf("%3d, ", cameraFrame->mMetadata.mGammaL->data[i + j]);
                }
            }
            if( cameraFrame->mMetadata.mGammaR)
            {
                printf("Gamma R [#items]:%d\n", cameraFrame->mMetadata.mGammaR->nItems);
                for(uint32_t i = 0; i < ( cameraFrame->mMetadata.mGammaR->nItems ) / 16 ; i+=16)
                {
                    puts("");
                    for(int j = 0; j < 16; j++)
                        printf("%3d, ", cameraFrame->mMetadata.mGammaR->data[i + j]);
                }
            }
            if( cameraFrame->mMetadata.mGamma2D)
            {here
                printf("Gamma 2D [#items]:%d\n",cameraFrame->mMetadata.mGamma2D->nItems);
                for(uint32_t i = 0; i < ( cameraFrame->mMetadata.mGamma2D->nItems ) / 16 ; i+=16)
                {
                    puts("");
                    for(int j = 0; j < 16; j++)
                        printf("%3d, ", cameraFrame->mMetadata.mGamma2D->data[i + j]);
                }
            }
        }
#endif

#ifdef DVP_DEBUG_FILEOUT
    if( cameraFrame->mExtraDataBuf )
    {
        static int cur = 1;
        char filename[33];
        sprintf(filename, "gre_cam_fr_%05d.edt", cur);
        FILE * XtraDataFile = fopen(filename, "wb");
        fwrite( prvFrame->mExtraDataBuf , EXTRA_DATA_FILE_LENGHT , 1 , XtraDataFile );
//         printf("\r gre_cam_fr_%05d.edt written", cur);
        cur++;
        fclose(XtraDataFile);
        XtraDataFile = NULL;
    }
#endif
    fprintf(stdout, "\r total frames: video - %d, preview - %d, (diff: %d)",
            frameCount[VCAM_PORT_VIDEO - VCAM_PORT_MIN],
            frameCount[VCAM_PORT_PREVIEW - VCAM_PORT_MIN],
            ( frameCount[VCAM_PORT_PREVIEW - VCAM_PORT_MIN] - frameCount[VCAM_PORT_VIDEO - VCAM_PORT_MIN] )
           );

    fflush(stdout);
    ((VisionCam *)cameraFrame->mContext)->returnFrame(cameraFrame);
}

void receiveFramePackage(VisionCamFramePack *pack)
{
    VisionCamFrame *frame = NULL;
    DVP_Image_t *pImage = NULL;

    static int32_t packs = 0;

    for( int32_t i = VCAM_PORT_ALL + 1; i < VCAM_PORT_MAX; i++ )
    {
        if( pack->mExpectedFrames[i] )
        {
            frame = pack->mFrame[i];
            if( frame )
            {
                pImage = (DVP_Image_t *)frame->mFrameBuff;
                ((VisionCam *)frame->mContext)->returnFrame(frame);
            }
        }
    }
    packs++;
    fprintf(stdout, "\rtotal frames: %d", packs );
    fflush(stdout);
}

void drawFaceBox(VisionCamFrame *cameraFrame)
{
    unsigned char * pos = NULL;
    unsigned int bottom;
    DVP_Image_t *pFrame = (DVP_Image_t *)cameraFrame->mFrameBuff;

    if (pFrame->color == FOURCC_BIN1)
        return;

    for(uint32_t face = 0; face < cameraFrame->mDetectedFacesNum; face++ )
    {
        uint32_t top = cameraFrame->mFaces[face].mFacesCoordinates.mTop;
        uint32_t left = cameraFrame->mFaces[face].mFacesCoordinates.mLeft;
        uint32_t width = cameraFrame->mFaces[face].mFacesCoordinates.mWidth;
        uint32_t height = cameraFrame->mFaces[face].mFacesCoordinates.mHeight;

        if (width > cameraFrame->mWidth - left)
        {
            width = cameraFrame->mWidth - left;
        }

        if (height > cameraFrame->mHeight - top)
        {
            height = cameraFrame->mHeight - top;
        }
        bottom = (unsigned int)(pFrame->pData[0]+(cameraFrame->mOffsetY + top + height)*pFrame->y_stride);

        /// calculate top left posotion
        pos = pFrame->pData[0] + (cameraFrame->mOffsetY + top) * pFrame->y_stride
                               + (cameraFrame->mOffsetX + left) * pFrame->x_stride;

        /// draw two lines - horizontal border, top
        memset(pos, 0 , width*pFrame->x_stride);

        pos += pFrame->y_stride;
        if( (unsigned int)pos > bottom )
            pos -= pFrame->y_stride;

        memset(pos, 0 , width*pFrame->x_stride);

        pos += pFrame->y_stride;
        if( (unsigned int)pos > bottom )
            pos -= pFrame->y_stride;

        /// draw vertical border
        for( ; (unsigned int)pos < bottom - 2 ; pos += pFrame->y_stride)
        {
            pos[0] = 0;
            pos[1] = 0;
            pos[width*pFrame->x_stride + 0] = 0;
            pos[width*pFrame->x_stride + 1] = 0;
        }
        /// draw two lines - horizontal border, bottom
        memset(pos, 0 , width*pFrame->x_stride);

        pos += pFrame->y_stride;
        if( (unsigned int)pos > bottom )
            pos -= pFrame->y_stride;

        memset(pos, 0 , width*pFrame->x_stride);
    }
}

void setFormat(  void * input )
{
    VisionCam *vcam = (VisionCam *)input;
    menuEnrty subFormat;
    int tempColorSpace;
    int result = STATUS_SUCCESS;

    memcpy( &subFormat, &format, sizeof(menuEnrty));

    subFormat.type = paramEntry_set;
    subFormat.subMenu = (void*)&colorFormatSubMenu;

    if( VCAM_PORT_PREVIEW == currentPort )
        subFormat.localValue = &colorSpacePeview;
    else if( VCAM_PORT_VIDEO == currentPort )
        subFormat.localValue = &colorSpaceVideo;
    else
        subFormat.localValue = &colorSpacePeview;

    tempColorSpace = getValue( &subFormat );

    if( STATUS_INVALID_STATE == vcam->setParameter( (VisionCamParam_e)format.ID , (void*)&tempColorSpace, sizeof(int), currentPort))
    {
        bool restartPreview[VCAM_PORT_MAX] = { false };// one flag for each port, so we'd know which to start again

        if STATUS_PASSED(vcam->sendCommand( VCAM_CMD_PREVIEW_STOP, NULL, 0, VCAM_PORT_PREVIEW ) )
            restartPreview[VCAM_PORT_PREVIEW] = true;

        if STATUS_PASSED(vcam->sendCommand( VCAM_CMD_PREVIEW_STOP, NULL, 0, VCAM_PORT_VIDEO ) )
            restartPreview[VCAM_PORT_VIDEO] = true;

        result = stopServices( vcam );

        if( STATUS_SUCCESS == result )
        {
            if STATUS_FAILED( vcam->setParameter( (VisionCamParam_e)format.ID, (void *)&tempColorSpace, sizeof(int), currentPort) )
                printf("Error setting color format.");
        }

        if( STATUS_SUCCESS == result )
        {
            *((int32_t*)subFormat.localValue) = tempColorSpace;
            result = startServices(vcam);
        }

        if( restartPreview[VCAM_PORT_VIDEO] && STATUS_SUCCESS == result )
            result = vcam->sendCommand(VCAM_CMD_PREVIEW_START, NULL, 0, VCAM_PORT_VIDEO);

        if( restartPreview[VCAM_PORT_PREVIEW] && STATUS_SUCCESS == result )
            result = vcam->sendCommand(VCAM_CMD_PREVIEW_START, NULL, 0, VCAM_PORT_PREVIEW);
    }
}
