/*
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
#include <string.h>
#include <dvp/dvp.h>
#include <dvp/dvp_debug.h>

#if defined(LINUX)
#include <sys/time.h>
#include <sys/resource.h>
#endif

#if defined(DVP_USE_DSPLIB)
#include <dsplib/dvp_kl_dsplib.h>
#define TEST_DSPLIB
#include <dsplib/Test_DSPLIB.h>
#endif

typedef struct _dvp_buffer_params_t {
    DVP_Buffer_t *pBuffer;
    DVP_U32 elemSize;
    DVP_U32 numElems;
    DVP_MemType_e type;
} DVP_Buffer_Params_t;

DVP_BOOL DVP_Buffer_Construct(DVP_Handle handle, DVP_Buffer_Params_t *params, DVP_U32 numParams)
{
    DVP_BOOL ret = DVP_TRUE;
    DVP_U32 i = 0;
    for (i = 0; i < numParams; i++)
    {
        DVP_Buffer_Init(params[i].pBuffer, params[i].elemSize, params[i].numElems);
        if (DVP_Buffer_Alloc(handle, params[i].pBuffer, params[i].type) == DVP_FALSE) {
            ret = DVP_FALSE;
            goto failed;
        }
    }
failed:
    return ret;
}

void DVP_KernelGraphCompleted(void *cookie, DVP_KernelGraph_t *graph, DVP_U32 sectionIndex, DVP_U32 numNodesExecuted)
{
    cookie = cookie; // warnings
    graph = graph; // warnings
    sectionIndex = sectionIndex; // warnings
    numNodesExecuted = numNodesExecuted; // warnings
    DVP_PRINT(DVP_ZONE_ALWAYS, "TEST: {%p} Graph %p Section [%u] completed %u of %u nodes\n", cookie, graph, sectionIndex, numNodesExecuted, graph->sections[sectionIndex].numNodes);
}

uint32_t width;
uint32_t height;
uint32_t type;
uint32_t elemSize;
uint32_t numElems;

uint32_t capacity;
uint32_t iterations;
bool_e   serial;
option_t opts[] = {
    {OPTION_TYPE_INT, &width, sizeof(width), "-w", "--width", "Image Width"},
    {OPTION_TYPE_INT, &height, sizeof(height), "-h", "--height", "Image Height"},
    {OPTION_TYPE_INT, &type, sizeof(type), "-t", "--type", "Memory Type see DVP_MemType_e"},
    {OPTION_TYPE_INT, &capacity, sizeof(capacity), "-c", "--capacity", "Limit Mhz on all cores to this cap"},
    {OPTION_TYPE_INT, &iterations, sizeof(iterations), "-i", "--iterations", "The number of times the graph is executed"},
    {OPTION_TYPE_BOOL, &serial, sizeof(serial), "-s", "--serial", "Forces the execution of the graph in serial"},
};
uint32_t numOpts = dimof(opts);

int main(int argc, char *argv[])
{
    const DVP_U32 numNodes = 44;
    DVP_Buffer_t buffers[123];

    DVP_Handle hDVP = 0;
    DVP_KernelNode_t *pNodes = NULL;
    DVP_U32 numSections = 0;
    DVP_U32 i;

#if defined(SOSAL_RUNTIME_DEBUG)
    debug_get_zone_mask("SOSAL_ZONE_MASK", &sosal_zone_mask);
#endif

#if defined(DVP_RUNTIME_DEBUG)
    debug_get_zone_mask("DVP_ZONE_MASK", &dvp_zone_mask);
#endif

    // default values...
    DVP_U32 noNodes = 0;
    elemSize = sizeof(DVP_U16);
    numElems = 128;


    type = DVP_MTYPE_DEFAULT;
    capacity = 100;
    iterations = 1;
    serial = true_e;

    // get the user values...
    option_process(argc, argv, opts, numOpts);

#if defined(LINUX)
    {
        int p = getpriority(PRIO_PROCESS, 0); // get our priority
        setpriority(PRIO_PROCESS, 0, p-1);
    }
#endif


#if (defined(DVP_USE_DSPLIB))
    // get the handle (will implicitly load .so/.dll and initialize RPC).
    hDVP = DVP_KernelGraph_Init();
    if (hDVP)
    {
#if 0
        DVP_Image_Params_t params[] = {
            {&images[0],  width, height, FOURCC_UYVY, type},
            {&images[1],  width, height, FOURCC_RGBP, DVP_MTYPE_DEFAULT},
            {&images[2],  width, height, FOURCC_Y800, DVP_MTYPE_DEFAULT},
            {&images[3],  width, height, FOURCC_IYUV, DVP_MTYPE_DEFAULT},
        };
#endif
        DVP_Buffer_Params_t params_buffer[] = {
                {&buffers[0], elemSize, numElems, type},
                {&buffers[1], elemSize, numElems, type},
                {&buffers[2], elemSize, numElems, type},
                {&buffers[3], elemSize, numElems, type},
                {&buffers[4], elemSize, numElems, type},
                {&buffers[5], elemSize, numElems, type},
                {&buffers[6], elemSize, numElems, type},
                {&buffers[7], elemSize, (numElems)>>1,type},
                {&buffers[8], elemSize, numElems, type},
#if 1
                {&buffers[9], elemSize, numElems, type},
                {&buffers[10], elemSize, numElems, type},
                {&buffers[11], elemSize, numElems, type},
                {&buffers[12], elemSize, numElems, type},
                {&buffers[13], elemSize, numElems, type},
                {&buffers[14], elemSize, numElems, type},
                {&buffers[15], elemSize, numElems, type},
                {&buffers[16], elemSize, numElems, type},
                {&buffers[17], elemSize, numElems, type},
                {&buffers[18], elemSize, numElems, type},
                {&buffers[19], elemSize, numElems, type},
                {&buffers[20], elemSize, numElems, type},
                {&buffers[21], elemSize, numElems, type},
                {&buffers[22], elemSize, numElems, type},
                {&buffers[23], elemSize, sizeof(FFT_x)/sizeof(DVP_U16), type},
                {&buffers[24], elemSize, sizeof(FFT_x)/sizeof(DVP_U16), type},
                {&buffers[25], elemSize, sizeof(FFT_x)/sizeof(DVP_U16), type},
                {&buffers[26], elemSize, numElems, type},
                {&buffers[27], elemSize, numElems, type},
                {&buffers[28], elemSize, numElems, type},
                {&buffers[29], elemSize, numElems, type},
                {&buffers[30], elemSize, numElems, type},
                {&buffers[31], elemSize, numElems, type},
                {&buffers[32], elemSize, numElems, type},
                {&buffers[33], elemSize, numElems, type},
                {&buffers[34], elemSize, numElems, type},
                {&buffers[35], elemSize, numElems, type},
                {&buffers[36], elemSize, numElems, type},
                {&buffers[37], elemSize, numElems, type},
                {&buffers[38], elemSize, numElems, type},
                {&buffers[39], elemSize, numElems, type},
                {&buffers[40], elemSize, numElems, type},
                {&buffers[41], elemSize, sizeof(FIR_x)/sizeof(DVP_U16),type},
                {&buffers[42], elemSize, sizeof(FIR_h)/sizeof(DVP_U16),type},
                {&buffers[43], elemSize, sizeof(FIR_x)/sizeof(DVP_U16),type},
                {&buffers[44], elemSize, sizeof(FIR_x)/sizeof(DVP_U16),type},
                {&buffers[45], elemSize, sizeof(FIR_h)/sizeof(DVP_U16),type},
                {&buffers[46], elemSize, sizeof(FIR_x)/sizeof(DVP_U16),type},
                {&buffers[47], elemSize, sizeof(FIR_x)/sizeof(DVP_U16),type},
                {&buffers[48], elemSize, sizeof(FIR_h)/sizeof(DVP_U16),type},
                {&buffers[49], elemSize, numElems, type},
                {&buffers[50], elemSize, sizeof(FIR_x)/sizeof(DVP_U16),type},
                {&buffers[51], elemSize, sizeof(FIR_h)/sizeof(DVP_U16),type},
                {&buffers[52], elemSize, numElems, type},
                {&buffers[53], elemSize, sizeof(FIR_x)/sizeof(DVP_U16),type},
                {&buffers[54], elemSize, sizeof(FIR_x)/sizeof(DVP_U16),type},
                {&buffers[55], elemSize, numElems, type},
                {&buffers[56], elemSize, sizeof(FIR_Rx)/sizeof(DVP_U16),type},
                {&buffers[57], elemSize, sizeof(FIR_Rh)/sizeof(DVP_U16), type},
                {&buffers[58], elemSize, numElems, type},
                {&buffers[59], elemSize, sizeof(FIR_Rx)/sizeof(DVP_U16), type},
                {&buffers[60], elemSize, sizeof(FIR_Rh)/sizeof(DVP_U16), type},
                {&buffers[61], elemSize, numElems, type},
                {&buffers[62], elemSize, sizeof(FIR_Rx)/sizeof(DVP_U16), type},
                {&buffers[63], elemSize, sizeof(FIR_Rh)/sizeof(DVP_U16), type},
                {&buffers[64], elemSize, numElems, type},
                {&buffers[65], elemSize, sizeof(FIR_xS)/sizeof(DVP_U16), type},
                {&buffers[66], elemSize, sizeof(FIR_hs)/sizeof(DVP_U16), type},
                {&buffers[67], elemSize, numElems, type},
                {&buffers[68], elemSize, sizeof(IFFT_x)/sizeof(DVP_U16), type},
                {&buffers[69], elemSize, sizeof(IFFT_x)/sizeof(DVP_U16), type},
                {&buffers[70], elemSize, sizeof(IFFT_x)/sizeof(DVP_U16), type},
                {&buffers[71], elemSize, sizeof(IFFT_x)/sizeof(DVP_U16), type},
                {&buffers[72], elemSize, sizeof(IFFT_x)/sizeof(DVP_U16), type},
                {&buffers[73], elemSize, sizeof(IFFT_x)/sizeof(DVP_U16), type},
                {&buffers[74], elemSize, sizeof(IFFT_x)/sizeof(DVP_U16), type},
                {&buffers[75], elemSize, sizeof(IFFT_x)/sizeof(DVP_U16), type},
                {&buffers[76], elemSize, sizeof(IFFT_x)/sizeof(DVP_U16), type},
                {&buffers[77], elemSize, sizeof(IFFT_x)/sizeof(DVP_U16), type},
                {&buffers[78], elemSize, sizeof(IFFT_x)/sizeof(DVP_U16), type},
                {&buffers[79], elemSize, sizeof(IFFT_x)/sizeof(DVP_U16), type},
                {&buffers[80], elemSize, numElems, type},
                {&buffers[81], elemSize, numElems, type},
                {&buffers[82], elemSize, numElems, type},
                {&buffers[83], elemSize, numElems, type},
                {&buffers[84], elemSize, numElems, type},
                {&buffers[85], elemSize, sizeof(x_IIR)/sizeof(DVP_U16), type},
                {&buffers[86], elemSize, sizeof(k_IIR)/sizeof(DVP_U16), type},
                {&buffers[87], elemSize, sizeof(b_c_IIR)/sizeof(DVP_U16), type},
                {&buffers[88], elemSize, numElems, type},
                {&buffers[89], elemSize, sizeof(ptr_Coefs)/sizeof(DVP_U16), type},
                {&buffers[90], elemSize, (N_IIRSS + PAD_IIRSS), type},
                {&buffers[91], elemSize, 256, type},
                {&buffers[92], elemSize, 256, type},
                {&buffers[93], elemSize, 256, type},
                {&buffers[94], elemSize, 256, type},
                {&buffers[95], elemSize, 256, type},
                {&buffers[96], elemSize, 256, type},
                {&buffers[97], elemSize, 256, type},
                {&buffers[98], elemSize, 256, type},
                {&buffers[99], elemSize, sizeof(buffer_x)/sizeof(DVP_U16), type},
                {&buffers[100], elemSize, sizeof(buffer_x)/sizeof(DVP_U16), type},
                {&buffers[101], elemSize, sizeof(buffer_x)/sizeof(DVP_U16), type},
                {&buffers[102], elemSize, 256*9, type},
                {&buffers[103], elemSize, 9, type},
                {&buffers[104], elemSize, numElems, type},
                {&buffers[105], elemSize, numElems, type},
                {&buffers[106], elemSize, numElems, type},
                {&buffers[107], elemSize, numElems, type},
                {&buffers[108], elemSize, numElems, type},
                {&buffers[109], elemSize, numElems, type},
                {&buffers[110], elemSize, numElems, type},
                {&buffers[111], elemSize, numElems, type},
                {&buffers[112], elemSize, sizeof(FIR_x)/sizeof(DVP_U16), type},
                {&buffers[113], elemSize, sizeof(a_W_VEC)/sizeof(DVP_U16), type},
                {&buffers[114], elemSize, sizeof(a_W_VEC)/sizeof(DVP_U16), type},
                {&buffers[115], elemSize, sizeof(a_W_VEC)/sizeof(DVP_U16), type},
                {&buffers[116], elemSize, numElems, type},
                {&buffers[117], elemSize, numElems, type},
                {&buffers[118], elemSize, numElems, type},
                {&buffers[119], elemSize, numElems, type},
                {&buffers[120], elemSize, numElems, type},
                {&buffers[121], elemSize, 256, type},
                {&buffers[122], elemSize, 256, type},
#endif
        };

        if (DVP_Buffer_Construct(hDVP, params_buffer, dimof(params_buffer)) == DVP_FALSE)
            goto teardown;




        memcpy(params_buffer[0].pBuffer->pData,buffer_x,sizeof(buffer_x));
        memcpy(params_buffer[1].pBuffer->pData,buffer_x,sizeof(buffer_y));
        memcpy(params_buffer[3].pBuffer->pData,buffer_x,sizeof(buffer_x));
        memcpy(params_buffer[4].pBuffer->pData,buffer_x,sizeof(buffer_y));
        memcpy(params_buffer[8].pBuffer->pData,buffer_x,sizeof(buffer_x));
        memcpy(params_buffer[9].pBuffer->pData,buffer_x,sizeof(buffer_y));
        memcpy(params_buffer[11].pBuffer->pData,buffer_x,sizeof(buffer_x));
        memcpy(params_buffer[13].pBuffer->pData,buffer_x,sizeof(buffer_x));
        memcpy(params_buffer[15].pBuffer->pData,buffer_x,sizeof(buffer_x));
        memcpy(params_buffer[17].pBuffer->pData,buffer_x,sizeof(buffer_x));
        memcpy(params_buffer[18].pBuffer->pData,buffer_x,sizeof(buffer_y));
        memcpy(params_buffer[20].pBuffer->pData,buffer_x,sizeof(buffer_x));
        memcpy(params_buffer[21].pBuffer->pData,buffer_x,sizeof(buffer_y));
        memcpy(params_buffer[23].pBuffer->pData,FFT_x,sizeof(FFT_x));
        memcpy(params_buffer[24].pBuffer->pData,FFT_x,sizeof(FFT_x));
        memcpy(params_buffer[26].pBuffer->pData,buffer_x,sizeof(buffer_x));
        memcpy(params_buffer[27].pBuffer->pData,buffer_x,sizeof(buffer_y));
        memcpy(params_buffer[29].pBuffer->pData,buffer_x,sizeof(buffer_x));
        memcpy(params_buffer[30].pBuffer->pData,buffer_x,sizeof(buffer_y));
        memcpy(params_buffer[32].pBuffer->pData,buffer_x,sizeof(buffer_x));
        memcpy(params_buffer[33].pBuffer->pData,buffer_x,sizeof(buffer_y));
        memcpy(params_buffer[35].pBuffer->pData,buffer_x,sizeof(buffer_x));
        memcpy(params_buffer[36].pBuffer->pData,buffer_x,sizeof(buffer_y));
        memcpy(params_buffer[38].pBuffer->pData,buffer_x,sizeof(buffer_x));
        memcpy(params_buffer[39].pBuffer->pData,buffer_x,sizeof(buffer_y));

        memcpy(params_buffer[41].pBuffer->pData,FIR_x,sizeof(FIR_x));
        memcpy(params_buffer[42].pBuffer->pData,FIR_h,sizeof(FIR_h));
        memcpy(params_buffer[44].pBuffer->pData,FIR_x,sizeof(FIR_x));
        memcpy(params_buffer[45].pBuffer->pData,FIR_h,sizeof(FIR_h));
        memcpy(params_buffer[47].pBuffer->pData,FIR_x,sizeof(FIR_x));
        memcpy(params_buffer[48].pBuffer->pData,FIR_h,sizeof(FIR_h));
        memcpy(params_buffer[50].pBuffer->pData,FIR_x,sizeof(FIR_x));
        memcpy(params_buffer[51].pBuffer->pData,FIR_h,sizeof(FIR_h));
        memcpy(params_buffer[53].pBuffer->pData,FIR_x,sizeof(FIR_x));
        memcpy(params_buffer[54].pBuffer->pData,FIR_x,sizeof(FIR_x));

        memcpy(params_buffer[56].pBuffer->pData,FIR_Rx,sizeof(FIR_Rx));
        memcpy(params_buffer[57].pBuffer->pData,FIR_Rh,sizeof(FIR_Rh));
        memcpy(params_buffer[59].pBuffer->pData,FIR_Rx,sizeof(FIR_Rx));
        memcpy(params_buffer[60].pBuffer->pData,FIR_Rh,sizeof(FIR_Rh));
        memcpy(params_buffer[62].pBuffer->pData,FIR_Rx,sizeof(FIR_Rx));
        memcpy(params_buffer[63].pBuffer->pData,FIR_Rh,sizeof(FIR_Rh));

        memcpy(params_buffer[65].pBuffer->pData,FIR_xS,sizeof(FIR_xS));
        memcpy(params_buffer[66].pBuffer->pData,FIR_hs,sizeof(FIR_hs));

        memcpy(params_buffer[68].pBuffer->pData,IFFT_x,sizeof(IFFT_x));
        memcpy(params_buffer[69].pBuffer->pData,IFFT_x,sizeof(IFFT_x));
        memcpy(params_buffer[71].pBuffer->pData,IFFT_x,sizeof(IFFT_x));
        memcpy(params_buffer[72].pBuffer->pData,IFFT_x,sizeof(IFFT_x));
        memcpy(params_buffer[74].pBuffer->pData,IFFT_x,sizeof(IFFT_x));
        memcpy(params_buffer[75].pBuffer->pData,IFFT_x,sizeof(IFFT_x));
        memcpy(params_buffer[77].pBuffer->pData,IFFT_x,sizeof(IFFT_x));
        memcpy(params_buffer[78].pBuffer->pData,IFFT_x,sizeof(IFFT_x));

        memcpy(params_buffer[85].pBuffer->pData,x_IIR,sizeof(x_IIR));
        memcpy(params_buffer[86].pBuffer->pData,k_IIR,sizeof(k_IIR));
        memcpy(params_buffer[87].pBuffer->pData,b_c_IIR,sizeof(b_c_IIR));
        memcpy(params_buffer[89].pBuffer->pData,ptr_Coefs,sizeof(ptr_Coefs));

        memcpy(params_buffer[91].pBuffer->pData,MAT_x,sizeof(MAT_x));
        memcpy(params_buffer[92].pBuffer->pData,MAT_y,sizeof(MAT_y));
        memcpy(params_buffer[94].pBuffer->pData,MAT_x,sizeof(MAT_x));
        memcpy(params_buffer[95].pBuffer->pData,MAT_y,sizeof(MAT_y));
        memcpy(params_buffer[97].pBuffer->pData,MAT_x,sizeof(MAT_x));
        memcpy(params_buffer[99].pBuffer->pData,buffer_x,sizeof(buffer_x));
        memcpy(params_buffer[100].pBuffer->pData,buffer_x,sizeof(buffer_x));
        memcpy(params_buffer[101].pBuffer->pData,buffer_x,sizeof(buffer_x));
        memcpy(params_buffer[102].pBuffer->pData,GSP0_TABLE,sizeof(GSP0_TABLE));
        memcpy(params_buffer[103].pBuffer->pData,errCoefs,sizeof(errCoefs));
        memcpy(params_buffer[112].pBuffer->pData,FIR_x,sizeof(FIR_x));
        memcpy(params_buffer[113].pBuffer->pData,a_W_VEC,sizeof(a_W_VEC));
        memcpy(params_buffer[114].pBuffer->pData,a_W_VEC,sizeof(a_W_VEC));
        memcpy(params_buffer[121].pBuffer->pData,MAT_x,sizeof(MAT_x)/sizeof(DVP_U16));
        // create the nodes (will implicitly map to other cores)
        pNodes = DVP_KernelNode_Alloc(hDVP, numNodes);
        if (pNodes)
        {
            DVP_KernelGraphSection_t ssections[] = {
                {&pNodes[0], 44, DVP_PERF_INIT, DVP_CORE_LOAD_INIT, DVP_FALSE},
            };

            DVP_U32            orders[] = {0};
            DVP_KernelGraph_t graph = {
                ssections,
                dimof(ssections),
                orders,
                DVP_PERF_INIT,
                DVP_FALSE,
            };

            // DSP Processing
            pNodes[noNodes].header.kernel = DVP_KN_DSP_ADD16;
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->input0 = buffers[0];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->input1 = buffers[1];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->output0 = buffers[2];
            pNodes[noNodes].header.affinity = DVP_CORE_DSP;
            noNodes++;

            pNodes[noNodes].header.kernel = DVP_KN_DSP_ADD32;
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->input0 = buffers[3];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->input1 = buffers[4];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->output0 = buffers[5];
            pNodes[noNodes].header.affinity = DVP_CORE_DSP;
            noNodes++;

            pNodes[noNodes].header.kernel = DVP_KN_DSP_AUTOCORR16;
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->input0 = buffers[6];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->output0 = buffers[7];
            pNodes[noNodes].header.affinity = DVP_CORE_DSP;
            noNodes++;

            pNodes[noNodes].header.kernel = DVP_KN_DSP_BITEXP32;
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->input0 = buffers[8];
            pNodes[noNodes].header.affinity = DVP_CORE_DSP;
            noNodes++;

            pNodes[noNodes].header.kernel = DVP_KN_DSP_BLKESWAP16;
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->input0 = buffers[9];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->output0 = buffers[10];
            noNodes++;

            pNodes[noNodes].header.kernel = DVP_KN_DSP_BLKESWAP32;
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->input0 = buffers[11];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->output0 = buffers[12];
            noNodes++;

            pNodes[noNodes].header.kernel = DVP_KN_DSP_BLKESWAP64;
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->input0 = buffers[13];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->output0 = buffers[14];
            noNodes++;

            pNodes[noNodes].header.kernel = DVP_KN_DSP_BLKMOVE;
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->input0 = buffers[15];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->output0 = buffers[16];
            noNodes++;

            pNodes[noNodes].header.kernel = DVP_KN_DSP_DOTPRODSQR;
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->input0 = buffers[17];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->input1 = buffers[18];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->output0 = buffers[19];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->val1 = 2;//Needs attn
            noNodes++;

            pNodes[noNodes].header.kernel = DVP_KN_DSP_DOTPROD;
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->input0 = buffers[20];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->input1 = buffers[21];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->output0 = buffers[22];
            noNodes++;

            pNodes[noNodes].header.kernel = DVP_KN_DSP_FFT_16x16;
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->input0 = buffers[23];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->twoway = buffers[24];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->output0 = buffers[25];
            noNodes++;

            pNodes[noNodes].header.kernel = DVP_KN_DSP_FFT_16x16R;
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->input0 = buffers[26];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->twoway = buffers[27];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->output0 = buffers[28];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->val1 = 2;
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->val2 = 2;
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->val3 = 8;
            noNodes++;

            pNodes[noNodes].header.kernel = DVP_KN_DSP_FFT_16x16_IMRE;
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->input0 = buffers[29];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->twoway = buffers[30];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->output0 = buffers[31];
            noNodes++;

            pNodes[noNodes].header.kernel = DVP_KN_DSP_FFT_16x32;
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->input0 = buffers[32];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->twoway = buffers[33];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->output0 = buffers[34];
            noNodes++;

            pNodes[noNodes].header.kernel = DVP_KN_DSP_FFT_32x32;
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->input0 = buffers[35];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->twoway = buffers[36];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->output0 = buffers[37];
            noNodes++;

            pNodes[noNodes].header.kernel = DVP_KN_DSP_FFT_32x32s;
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->input0 = buffers[38];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->twoway = buffers[39];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->output0 = buffers[40];
            noNodes++;

            pNodes[noNodes].header.kernel = DVP_KN_DSP_FIR_CPLX;
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->input0 = buffers[41];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->twoway = buffers[42];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->output0 = buffers[43];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->val1  = sizeof(FIR_h)>>1;
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->val2  = sizeof(FIR_x)>>1;
            noNodes++;

            pNodes[noNodes].header.kernel = DVP_KN_DSP_FIR_HM4x4;
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->input0 = buffers[44];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->twoway = buffers[45];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->output0 = buffers[46];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->val1  = sizeof(FIR_h)>>1;
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->val2  = sizeof(FIR_x)>>1;
            noNodes++;

            pNodes[noNodes].header.kernel = DVP_KN_DSP_FIR_GEN;
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->input0 = buffers[47];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->twoway = buffers[48];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->output0 = buffers[49];
            noNodes++;

            pNodes[noNodes].header.kernel = DVP_KN_DSP_FIR_HM17_RA8x8;
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->input0 = buffers[50];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->twoway = buffers[51];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->output0 = buffers[52];
            noNodes++;

            pNodes[noNodes].header.kernel = DVP_KN_DSP_FIR_LMS2;
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->input0 = buffers[53];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->twoway = buffers[54];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->output0 = buffers[55];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->val1  = 0x1000;
            noNodes++;

            pNodes[noNodes].header.kernel = DVP_KN_DSP_FIR_R4;
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->input0 = buffers[56];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->twoway = buffers[57];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->output0 = buffers[58];
            noNodes++;

            pNodes[noNodes].header.kernel = DVP_KN_DSP_FIR_R8;
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->input0 = buffers[59];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->twoway = buffers[60];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->output0 = buffers[61];
            noNodes++;

            pNodes[noNodes].header.kernel = DVP_KN_DSP_FIR_HM16_RM8A8x8;
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->input0 = buffers[62];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->twoway = buffers[63];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->output0 = buffers[64];
            noNodes++;

            pNodes[noNodes].header.kernel = DVP_KN_DSP_FIR_SYM;
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->input0 = buffers[65];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->twoway = buffers[66];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->output0 = buffers[67];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->val1  = 0;
            noNodes++; //25


            pNodes[noNodes].header.kernel = DVP_KN_DSP_IFFT_16x16;
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->input0 = buffers[68];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->twoway = buffers[69];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->output0 = buffers[70];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->val1  = (sizeof(IFFT_x)/sizeof(DVP_U16))>>1; // Size of coefficient array = 2*nx
            noNodes++;

            pNodes[noNodes].header.kernel = DVP_KN_DSP_IFFT_16x16_IMRE;
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->input0 = buffers[71];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->twoway = buffers[72];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->output0 = buffers[73];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->val1  = (sizeof(IFFT_x)/sizeof(DVP_U16))>>1;
            noNodes++;

            pNodes[noNodes].header.kernel = DVP_KN_DSP_IFFT_16x32;
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->input0 = buffers[74];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->twoway = buffers[75];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->output0 = buffers[76];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->val1  = (sizeof(IFFT_x)/sizeof(DVP_U16))>>1;
            noNodes++;

            pNodes[noNodes].header.kernel = DVP_KN_DSP_IFFT_32x32;
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->input0 = buffers[77];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->twoway = buffers[78];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->output0 = buffers[79];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->val1  =    (sizeof(IFFT_x)/sizeof(DVP_U16))>>1;
            noNodes++;

            pNodes[noNodes].header.kernel = DVP_KN_DSP_IIR;
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->input0 = buffers[80];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->input1 = buffers[81];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->twoway = buffers[82];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->output0 = buffers[83];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->output1 = buffers[84];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->val1   = numElems;
            noNodes++;

            pNodes[noNodes].header.kernel = DVP_KN_DSP_IIR_LAT;
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->input0 = buffers[85];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->input1 = buffers[86];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->twoway = buffers[87];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->output0 = buffers[88];
            noNodes++;//3  //31

            pNodes[noNodes].header.kernel = DVP_KN_DSP_IIR_SS;
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->val1 = 0;
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->input0 = buffers[89];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->input1 = buffers[90];
            noNodes++;


            pNodes[noNodes].header.kernel = DVP_KN_DSP_MUL;
            dvp_knode_to(&pNodes[noNodes], DVP_DSP_MatMul)->input0 = buffers[91];
            dvp_knode_to(&pNodes[noNodes], DVP_DSP_MatMul)->input1 = buffers[92];
            dvp_knode_to(&pNodes[noNodes], DVP_DSP_MatMul)->output0 = buffers[93];
            dvp_knode_to(&pNodes[noNodes], DVP_DSP_MatMul)->r1 = 16;
            dvp_knode_to(&pNodes[noNodes], DVP_DSP_MatMul)->c1 = 16;
            dvp_knode_to(&pNodes[noNodes], DVP_DSP_MatMul)->r2 = 16;
            dvp_knode_to(&pNodes[noNodes], DVP_DSP_MatMul)->c2 = 16;
            dvp_knode_to(&pNodes[noNodes], DVP_DSP_MatMul)->qs = 1;
            noNodes++;

            pNodes[noNodes].header.kernel = DVP_KN_DSP_MUL_CPLX;
            dvp_knode_to(&pNodes[noNodes], DVP_DSP_MatMul)->input0 = buffers[94];
            dvp_knode_to(&pNodes[noNodes], DVP_DSP_MatMul)->input1 = buffers[95];
            dvp_knode_to(&pNodes[noNodes], DVP_DSP_MatMul)->output0 = buffers[96];
            dvp_knode_to(&pNodes[noNodes], DVP_DSP_MatMul)->r1 = 16;
            dvp_knode_to(&pNodes[noNodes], DVP_DSP_MatMul)->c1 = 16;
            dvp_knode_to(&pNodes[noNodes], DVP_DSP_MatMul)->r2 = 16;
            dvp_knode_to(&pNodes[noNodes], DVP_DSP_MatMul)->c2 = 16;
            dvp_knode_to(&pNodes[noNodes], DVP_DSP_MatMul)->qs = 1;
            noNodes++;


            pNodes[noNodes].header.kernel = DVP_KN_DSP_W_VEC;
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->input0 = buffers[113];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->input1 = buffers[114];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->output0 = buffers[115];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->val1 = 16384;
            noNodes++;

            pNodes[noNodes].header.kernel = DVP_KN_DSP_MAXVAL;
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->input0 = buffers[99];
            noNodes++;

            pNodes[noNodes].header.kernel = DVP_KN_DSP_MINVAL;
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->input0 = buffers[100];
            noNodes++;

            pNodes[noNodes].header.kernel = DVP_KN_DSP_MAXIDX;
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->input0 = buffers[101];
            noNodes++;


            pNodes[noNodes].header.kernel = DVP_KN_DSP_MINERROR;
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->input0 = buffers[102];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->input1 = buffers[103];
            noNodes++;

            pNodes[noNodes].header.kernel = DVP_KN_DSP_MUL32;
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->input0 = buffers[104];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->input1 = buffers[105];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->output0 = buffers[106];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->val1 = numElems;
            noNodes++;

            pNodes[noNodes].header.kernel = DVP_KN_DSP_NEG32;
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->input0 = buffers[107];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->output0 = buffers[108];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->val1 = numElems;
            noNodes++;

            pNodes[noNodes].header.kernel = DVP_KN_DSP_RECIP16;
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->input0 = buffers[109];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->twoway = buffers[110];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->output0 = buffers[111];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->val1 = numElems;
            noNodes++;


            pNodes[noNodes].header.kernel = DVP_KN_DSP_VECSUMSQ;
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->input0 = buffers[112];
            noNodes++;

            pNodes[noNodes].header.kernel = DVP_KN_DSP_MAT_TRANS;
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->input0 = buffers[121];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->output0 = buffers[122];
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->val1 = 16;
            dvp_knode_to(&pNodes[noNodes], DVP_DSPFunc)->val2 = 16;
            noNodes++;


            for (i = 0; i < DVP_CORE_MAX; i++)
            {
                DVP_U32 cap2, cap1 = DVP_GetCoreCapacity(hDVP, i);
                DVP_SetCoreCapacity(hDVP, i, capacity);
                cap2 = DVP_GetCoreCapacity(hDVP, i);
                DVP_PRINT(DVP_ZONE_ALWAYS, "Core[%u] was set to limit at %u capacity, now at %u\n", i, cap1, cap2);
                cap1 = cap1; // warnings
            }

            do {
                DVP_PRINT(DVP_ZONE_ALWAYS, "Iteration %u\n", iterations);
                // perform the graph processing
                numSections = DVP_KernelGraph_Process(hDVP, &graph, NULL, DVP_KernelGraphCompleted);
                DVP_PRINT(DVP_ZONE_ALWAYS, "%u Sections Completed!\n", numSections);
            } while (iterations-- && numSections > 0);
            // show us the results
            DVP_PrintPerformanceGraph(hDVP, &graph);
        teardown:
            // free the nodes
            DVP_KernelNode_Free(hDVP, pNodes, numNodes);
        }

        // free the image memory
        for (i = 0; i < dimof(buffers); i++)
            DVP_Buffer_Free(hDVP, &buffers[i]);

    }
    // tear down the graph resources
    DVP_KernelGraph_Deinit(hDVP);
#else
    DVP_PRINT(DVP_ZONE_ERROR, "Required VISION libraries are not present in the build!\n");
#endif
    return 0;
}
