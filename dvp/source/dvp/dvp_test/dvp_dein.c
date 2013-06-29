/**
 *  Copyright (C) 2009-2012 Texas Instruments, Inc.
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

#include <sosal/sosal.h>

#include <dvp/dvp.h>
#include <dvp/dvp_debug.h>
#include <dvp/dvp_display.h>
#if defined(DVP_USE_YUV)
#include <yuv/yuv_armv7.h>
#include <yuv/dvp_kl_yuv.h>
#endif
#if defined(DVP_USE_DEI)
#include <dei/dvp_kl_dei.h>

#define DEINTERLACER_MAX_SIZE_X  (1936)
#define DEINTERLACER_MAX_SIZE_Y (1100)

#define DEIN_BLKW (48)
#define DEIN_BLKH (10)

DVP_U08 line_buf[4096];

void DVP_KernelGraphCompleted(void *cookie, DVP_KernelGraph_t *graph, DVP_U32 sectionIndex, DVP_U32 numNodesExecuted)
{
    DVP_PRINT(DVP_ZONE_ALWAYS, "TEST: {%p} Graph %p Section [%u] completed %u of %u nodes\n", cookie, graph, sectionIndex, numNodesExecuted, graph->sections[sectionIndex].numNodes);
#ifndef DVP_DEBUG
    cookie = cookie;
    graph = graph;
    sectionIndex = sectionIndex;
    numNodesExecuted = numNodesExecuted;
#endif
    return;
}

#define DEIN_BLKW (48)
#define DEIN_BLKH (10)

int main(int argc, char *argv[])
{
    FILE *fp_inp, *fp_outp;
    int fldnum = 0;
    DVP_U32 i,j;
    DVP_U32 image_buffer_stride;
    DVP_U32 x_stride;
    DVP_U32 TEST_IMG_INPUT_X;
    DVP_U32 TEST_IMG_INPUT_Y;

    const DVP_U32 numNodes = 2;
    DVP_KernelNode_t *pNodes = NULL;

    DVP_Buffer_t m_buffers[3];
    DVP_Image_t m_images[3];
    DVP_Handle hDVP = 0;

    DVP_U08 *virt_fld_in_current;

    DVP_Image_t *fld_in_prev_ptr;
    DVP_Image_t *fld_in_current_ptr;

    DVP_Buffer_t *luma_d0_ptr;
    DVP_Buffer_t *luma_d1_ptr;
    DVP_Buffer_t *luma_d2_ptr;

    if (argc > 4)
    {
        fp_inp  = fopen(argv[1],  "rb");

        if(fp_inp == NULL)
            exit(-1);
        fp_outp = fopen(argv[4], "wb");
        if(fp_outp == NULL)
            exit(-1);

        TEST_IMG_INPUT_X = atoi(argv[2]);
        TEST_IMG_INPUT_Y = atoi(argv[3]);
    }
    else
    {
        DVP_PRINT(DVP_ZONE_ERROR,"Invalid arguments. Please specify input filename, img width,img height and output filename \n");
        return 0;
    }

    hDVP = DVP_KernelGraph_Init();
    if (hDVP)
    {
        DVP_U32 numElems = 2*128*(TEST_IMG_INPUT_X/DEIN_BLKW)*((2*TEST_IMG_INPUT_Y)/DEIN_BLKH);

        DVP_Buffer_Init(&m_buffers[0], 1, numElems);
        DVP_Buffer_Init(&m_buffers[1], 1, numElems);
        DVP_Buffer_Init(&m_buffers[2], 1, numElems);
        if ( !DVP_Buffer_Alloc(hDVP, &m_buffers[0], DVP_MTYPE_DEFAULT) ||
             !DVP_Buffer_Alloc(hDVP, &m_buffers[1], DVP_MTYPE_DEFAULT) ||
             !DVP_Buffer_Alloc(hDVP, &m_buffers[2], DVP_MTYPE_DEFAULT) )
        {
             DVP_KernelGraph_Deinit(hDVP);
             return STATUS_NOT_ENOUGH_MEMORY;
        }

        //clear scratch buffers
        memset(m_buffers[0].pData, 0, numElems);
        memset(m_buffers[1].pData, 0, numElems);
        memset(m_buffers[2].pData, 0, numElems);

        DVP_Image_Init(&m_images[0],(TEST_IMG_INPUT_X+16), (2*TEST_IMG_INPUT_Y+20), FOURCC_UYVY);
        DVP_Image_Init(&m_images[1],(TEST_IMG_INPUT_X), (TEST_IMG_INPUT_Y), FOURCC_ARGB);
        DVP_Image_Init(&m_images[2],(TEST_IMG_INPUT_X+16), (2*TEST_IMG_INPUT_Y+20), FOURCC_UYVY);
        if(!DVP_Image_Alloc(hDVP, &m_images[0], DVP_MTYPE_MPUCACHED_1DTILED) ||
           !DVP_Image_Alloc(hDVP, &m_images[1], DVP_MTYPE_MPUCACHED_1DTILED) ||
           !DVP_Image_Alloc(hDVP, &m_images[2], DVP_MTYPE_MPUCACHED_1DTILED) )
        {
            DVP_KernelGraph_Deinit(hDVP);
            fclose(fp_inp);
            fclose(fp_outp);
            return STATUS_NOT_ENOUGH_MEMORY;
        }

         //clear image buffers
        image_buffer_stride = m_images[0].y_stride;
        virt_fld_in_current =  m_images[0].pData[0];
        for(i = 0 ; i < (2*TEST_IMG_INPUT_Y+20) ; i++)
            memset((void *)(virt_fld_in_current+i * image_buffer_stride),0, 2*(TEST_IMG_INPUT_X + 16));

        virt_fld_in_current =  m_images[1].pData[0];
        image_buffer_stride = m_images[1].y_stride;
        for(i = 0 ; i < (TEST_IMG_INPUT_Y) ; i++)
            memset((void *)(virt_fld_in_current+i * image_buffer_stride),0, 4*(TEST_IMG_INPUT_X));

        virt_fld_in_current =  m_images[2].pData[0];
        image_buffer_stride = m_images[2].y_stride;
        for(i = 0 ; i < (2*TEST_IMG_INPUT_Y+20) ; i++)
            memset((void *)(virt_fld_in_current+i * image_buffer_stride),0, 2*(TEST_IMG_INPUT_X+16));

        // create the nodes (will implicitly map to other cores)
        pNodes = DVP_KernelNode_Alloc(hDVP, numNodes);
        if (pNodes)
        {
            DVP_KernelGraphSection_t sections[] = {
            {&pNodes[0], numNodes, DVP_PERF_INIT, DVP_CORE_LOAD_INIT, DVP_FALSE},
            };
            DVP_U32  orders[] = {0};
            DVP_KernelGraph_t graph = {
                sections,
                dimof(sections),
                orders,
                DVP_PERF_INIT,
                DVP_FALSE,
            };

            DVP_Transform_t *pT = dvp_knode_to(&pNodes[0], DVP_Transform_t);
            pNodes[0].header.kernel = DVP_KN_YUV_ARGB_TO_UYVY;
            pNodes[0].header.affinity = DVP_CORE_CPU;
            memcpy(&pT->input,&m_images[1],sizeof(DVP_Image_t));
            memcpy(&pT->output,&m_images[2],sizeof(DVP_Image_t));
            x_stride = m_images[2].x_stride;
            pT->output.y_stride = pT->output.y_stride*2;
            pT->output.pData[0] = pT->output.pData[0]+ 5*(TEST_IMG_INPUT_X+16)*x_stride + 8*x_stride;

            image_buffer_stride = m_images[1].y_stride;
            virt_fld_in_current = m_images[1].pData[0];
            for(i = 0 ; i < (TEST_IMG_INPUT_Y) ; i++)
                fread((void *)(virt_fld_in_current+i * image_buffer_stride), sizeof(char), (4*TEST_IMG_INPUT_X ), fp_inp);


            luma_d0_ptr = &m_buffers[0];
            luma_d1_ptr = &m_buffers[1];
            luma_d2_ptr = &m_buffers[2];
            fld_in_current_ptr = &m_images[2];
            fld_in_prev_ptr    = &m_images[0];

            pNodes[1].header.kernel = DVP_KN_DEI_DEINTERLACER;
            DVP_Deinterlacer_t *dein = dvp_knode_to(&pNodes[1], DVP_Deinterlacer_t);
            pNodes[1].header.affinity = DVP_CORE_DSP;
            memcpy(&dein->phy_fld_in_current, fld_in_current_ptr, sizeof(DVP_Image_t));
            memcpy(&dein->phy_fld_in_prev, fld_in_prev_ptr, sizeof(DVP_Image_t));
            memcpy(&dein->phy_luma_d0,        luma_d0_ptr,        sizeof(DVP_Buffer_t));
            memcpy(&dein->phy_luma_d1,        luma_d1_ptr,        sizeof(DVP_Buffer_t));
            memcpy(&dein->phy_luma_d2,        luma_d2_ptr,        sizeof(DVP_Buffer_t));

            dein->phy_virt_flag=0;
            dein->fldnum = fldnum;
            dein->bias = 8;
            dein->sad_corr = 3;
            dein->sad_scale = 128;
            dein->initialized = 0;
            dein->Width = TEST_IMG_INPUT_X;
            dein->Height = TEST_IMG_INPUT_Y;
            dein->stride = (TEST_IMG_INPUT_X+16)*2;
            /*32*1024 for 2D tiler memory physical stride */

            /*init deinterlacer */
            if (DVP_KernelGraph_Process (hDVP, &graph, NULL, DVP_KernelGraphCompleted) == dimof(sections))
            {
                DVP_PRINT(DVP_ZONE_ALWAYS, "Init OK\n");
            }

            if(dein->initialized == 0)
            {
                DVP_PRINT(DVP_ZONE_ERROR,"De-interlacer Init failed\n");
                DVP_KernelGraph_Deinit(hDVP);
                fclose(fp_inp);
                fclose(fp_outp);
                return 0;
            }


            if (DVP_KernelGraph_Process (hDVP, &graph, NULL, DVP_KernelGraphCompleted) == dimof(sections))
            {
                DVP_PrintPerformanceGraph(hDVP, &graph);
                DVP_PRINT(DVP_ZONE_ALWAYS, "Frame processed\n");
            }

            image_buffer_stride = m_images[2].y_stride;
            virt_fld_in_current = m_images[2].pData[0];
            for(i = 0; i < (2*TEST_IMG_INPUT_Y+20);i++)
            {
                for( j = 0; j < (TEST_IMG_INPUT_X+16)*2; j++)
                {
                    line_buf[j] = virt_fld_in_current[i*(image_buffer_stride)+j];
                }
                fwrite(line_buf, sizeof(char), (TEST_IMG_INPUT_X+16)*2, fp_outp);
            }

            // Destroy the Nodes
            DVP_KernelNode_Free(hDVP, pNodes, numNodes);

        }//end of if (pNodes)

        // Free memory
        for (i = 0; i < dimof(m_images); i++)
        {
            DVP_Image_Free(hDVP, &m_images[i]);
            DVP_Image_Deinit(&m_images[i]);
        }
        for (i = 0; i < dimof(m_buffers); i++)
        {
            DVP_Buffer_Free(hDVP, &m_buffers[i]);
            DVP_Buffer_Deinit(&m_buffers[i]);
        }

        DVP_KernelGraph_Deinit(hDVP);

        fclose(fp_inp);
        fclose(fp_outp);
    }//end of if (hDVP)
    return 0;
}
#else
int main(int argc, char *argv[])
{
    DVP_PRINT(DVP_ZONE_ERROR,"De-interlacer not supported in your version\n");
    return 0;
}
#endif
