/* ====================================================================
 *   Copyright (C) 2012 Texas Instruments Incorporated
 * ==================================================================== */

/*!
 * \file
 * \brief This file decribes the exact set of kernels supported by the
 * deinterlacer algorithm library as exported through DVP.
 * \note When using these enums, only the deinterlacer kernels will be used!
 * \defgroup group_algo_dei Algos: DEI
 * \brief This only works on platforms with DSP firmware with the
 * enabled DVP KGM.
*/

#ifndef _DVP_KL_DEI_H_
#define _DVP_KL_DEI_H_

#ifdef DVP_USE_DEI

#include <dvp/dvp_types.h>

/*!
 * \brief The explicit list of kernels supported by the DEI algorithm library.
 * \see DVP_Transform_t
 * \ingroup group_algo_dei
 */
enum DVP_KernelNode_Dei_e {
    DVP_KN_DEI_BASE = DVP_KN_LIBRARY_BASE(DVP_KL_DEI),

    /*!
     * Deinterlacer \n
     * Configuration Structure: DVP_Deinterlacer_t
     * \param [in,out] phy_fld_in_current Image color type supported: FOURCC_UYVY
     * \param [in] phy_fld_in_prev    Image color type supported: FOURCC_UYVY
     */
    DVP_KN_DEI_DEINTERLACER,

    /*!
     * Deinterlacer initialization.  This is used to acquire the resources
     * required for the deinterlacer kernel.\n
     * Configuration Structure: Not Applicable
     */
    DVP_KN_DEI_DEINTERLACER_INIT,

    /*!
     * Deinterlacer deinitialization.  This is used to release the resources
     * required for the deinterlacer kernel.\n
     * Configuration Structure: Not Applicable
     */
    DVP_KN_DEI_DEINTERLACER_DEINIT
};

/*!
 * \brief This structure is use with deinterlacer Kernels.
 * \ingroup group_algo_dei
 */
typedef struct _dvp_deinterlacer_t {
    DVP_Image_t phy_fld_in_current;     /*!<  Current Input/Output image when phy_virt_flag is 0*/
    DVP_Image_t phy_fld_in_prev;        /*!<  Previous image when phy_virt_flag is 0*/

    DVP_Buffer_t phy_luma_d0;           /*!<  internal working buffer */
    DVP_Buffer_t phy_luma_d1;           /*!<  internal working buffer */
    DVP_Buffer_t phy_luma_d2;           /*!<  internal working buffer */

    DVP_U32 bias;                       /*!<  deinterlacer tunning parameter bias */
    DVP_U32 sad_corr;                   /*!<  deinterlacer tunning parameter sad_corr */
    DVP_U32 sad_scale;                  /*!<  deinterlacer tunning parameter sad_scale */

    DVP_U32 fldnum;                     /*!<  input field number */

    DVP_U32 initialized;                /*!<  flag  0->not initialized 1->initialized*/

    DVP_U32 Width;                      /*!<  Image width */
    DVP_U32 Height;                     /*!<  Image height */

    DVP_U32 stride;                     /*!<  Image stride */

    DVP_U32 phy_virt_flag;              /*!<  0->virtual ; 1->physical */
    DVP_U32 phy_in_current;             /*!<  Current Input/Output image physical address when phy_virt_flag is set to 1*/
    DVP_U32 phy_in_prev;                /*!<  Previous image physical address when phy_virt_flag is set to 1*/

    DVP_U32 dbg_param0;                 /*!<  used for debuging */
    DVP_U32 dbg_param1;                 /*!<  used for debuging */
    DVP_U32 dbg_param2;                 /*!<  used for debuging */
    DVP_U32 dbg_param3;                 /*!<  used for debuging */
    DVP_U32 dbg_param4;                 /*!<  used for debuging */

} DVP_Deinterlacer_t;

#endif // DVP_USE_DEI

#endif // _DVP_KL_DEI_H_

