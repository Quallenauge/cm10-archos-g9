/* ====================================================================
 *   Copyright (C) 2012 Texas Instruments Incorporated
 * ==================================================================== */

#ifndef _DVP_TYPES_H_
#define _DVP_TYPES_H_

//******************************************************************************
// INCLUDES
//******************************************************************************

#include <time.h>
#include <stdint.h>
#if !defined(SYSBIOS)
#include <sosal/sosal.h>
#elif defined(SYSBIOS) && defined(CENTEVE)
#include <sosal/sosal.h>
#endif

//******************************************************************************
// DEFINES
//******************************************************************************

/*! \brief Major Interface Version Number.
 * \ingroup group_system
 */
#define DVP_VERSION_MAJOR (0x0002)

/*! \brief Minor Interface Version Number.
 * \ingroup group_system
 */
#define DVP_VERISON_MINOR (0x0000)

/*! \brief The version number of the interface.
 * \ingroup group_system
 */
#define DVP_VERSION       ((DVP_VERSION_MAJOR << 16) | DVP_VERISON_MINOR)

#if defined(ANDROID) || defined(LINUX) || defined(SYSBIOS) || defined(__QNX__) || defined(DARWIN)
#ifndef PAGE_SIZE
#define PAGE_SIZE           (4096)
#endif
#define DVP_PAGE_SIZE       (PAGE_SIZE)
#else
#define DVP_PAGE_SIZE       (4096)
#endif

#define DVP_MAX_NAME    (32)

#ifndef MAX_PATH
#define MAX_PATH        (255)
#endif

/*! \brief The maximum number of subplanes of an image.
 * \ingroup group_images
 */
#define DVP_MAX_PLANES (4)

#if defined(SYSBIOS) && !defined(CENTEVE)
#define MAX_RTIMER_T        (0xFFFFFFFFFFFFFFFF)
#define rtimer_freq()       (CLOCKS_PER_SEC)
#endif

// Default Systems Configuration
#if defined(ARM) && defined(ANDROID) && (defined(FROYO) || defined(GINGERBREAD))
#ifndef DVP_USE_TILER
#define DVP_USE_TILER
#endif
#elif defined(ARM) && defined(ANDROID) && (defined(ICS) || defined(JELLYBEAN))
#ifndef DVP_USE_ION
#define DVP_USE_ION
#endif
#ifndef DVP_USE_GRALLOC
#define DVP_USE_GRALLOC
#endif
#elif defined(ANDROID) // NDK or external users (we assume ICS NDK)
#ifndef DVP_USE_ION
#define DVP_USE_ION
#endif
#ifndef DVP_USE_GRALLOC
#define DVP_USE_GRALLOC
#endif
#ifndef DVP_USE_SHARED_T
#define DVP_USE_SHARED_T
#endif
#elif defined(ARM) && defined(LINUX)
#ifndef DVP_USE_BO
#define DVP_USE_BO
#endif
#elif defined(ARM) && defined(__QNX__)
#ifndef DVP_USE_TILER
#define DVP_USE_TILER
#endif
#endif

//******************************************************************************
// ENUMERATIONS AND TYPEDEFS
//******************************************************************************

/*! \brief A signed 8 bit value.
 * \ingroup group_types
 */
typedef  int8_t DVP_S08;

/*! \brief A signed 16 bit value.
 * \ingroup group_types
 */
typedef  int16_t DVP_S16;

/*! \brief A signed 32 bit value.
 * \ingroup group_types
 */
typedef  int32_t DVP_S32;

/*! \brief A signed 64 bit value.
 * \ingroup group_types
 */
typedef  int64_t DVP_S64;

/*! \brief An unsigned 8 bit value.
 * \ingroup group_types
 */
typedef uint8_t DVP_U08;

/*! \brief An unsigned 16 bit value.
 * \ingroup group_types
 */
typedef uint16_t DVP_U16;

/*! \brief An unsigned 32 bit value.
 * \ingroup group_types
 */
typedef uint32_t DVP_U32;

/*! \brief An unsigned 64 bit value.
 * \ingroup group_types
 */
typedef uint64_t DVP_U64;

#if defined(SYSBIOS) && !defined(CENTEVE)

/*! \brief A generic pointer.
 * \ingroup group_types
 */
typedef void    *DVP_PTR;

/*! \brief An architecture depth defined generic value.
 * \ingroup group_types
 */
typedef size_t DVP_VALUE;

#ifndef FMT_RTIMER_T
/*! \brief For platforms without SOSAL, this is a 64 bit unsigned number.
 * \ingroup group_types
 */
typedef uint64_t rtime_t;
#endif

#else // on other platforms, use SOSAL's definitions

/*! \brief A DVP typedef for a SOSAL type.
 * \ingroup group_types
 */
typedef ptr_t DVP_PTR;

/*! \brief A DVP typedef for a SOSAL type.
 * \ingroup group_types
 */
typedef value_t DVP_VALUE;

#endif

/*! \brief A fixed size enumeration field used to force compilers to
 * not automatically downsize fields.
 * \ingroup group_types
 */
typedef int32_t DVP_ENUM;

/*!
 * \brief This is a simple boolean typedef for DVP functions.
 * \ingroup group_types
 */
typedef enum _dvp_bool_e {
    DVP_FALSE = 0,          /*!<  Logically false */
    DVP_TRUE,               /*!<  Logically true */
} DVP_BOOL;

/*!
 * \brief These errors can be returned by the DVP_KernelGraphManager_Init call on the remote cores.
 * \ingroup group_system
 */
typedef enum _dvp_error_e {
    DVP_ERROR_NO_RESOURCE       = -10,     /*!<  A critical resource was not available. */
    DVP_ERROR_NO_MEMORY         =  -9,     /*!<  There was not enough memory to complete the operation */
    DVP_ERROR_INVALID_PARAMETER =  -8,     /*!<  A supplied parameter was incorrect */
    DVP_ERROR_NOT_IMPLEMENTED   =  -7,     /*!<  The requested operation has not been implemented */
    DVP_ERROR_VERSION_MISMATCH  =  -1,     /*!<  The initialization of the remote KGM failed due to a DVP mismatch */
    DVP_ERROR_FAILURE           =   0,     /*!<  Generic Failure */
    DVP_SUCCESS                 =   1,     /*!<  Success */
} DVP_Error_e;

/*! \brief Resolves to non-zero when DVP_Error_e codes return any failure
 * \ingroup group_debugging
 */
#define DVP_FAILED(x) (x <= DVP_ERROR_FAILURE)

/*! \brief Resolves to non-zero when DVP_Error_e codes are successful
 * \ingroup group_debugging
 */
#define DVP_PASSED(x) (x == DVP_SUCCESS)

/*!
 * \brief This structure defines a 1d buffer used with some DVP kernels.
 * \ingroup group_buffers
 */
typedef struct _dvp_buffer_t {
    DVP_U08 *pData;                 /*!<  The pointer to the data region */
    DVP_U32  elemSize;              /*!<  The size of each element in the buffer */
    DVP_U32  numBytes;              /*!<  The number of bytes allocated for the entire buffer */
    DVP_U32  memType;               /*!<  The memory mapping type, options are DVP_MemType_e enumeration */
    DVP_PTR  reserved;              /*!<  Used for internal tracking */
    DVP_S32  skipCacheOpInval;      /*!<  Used to indicate to the DVP lower layers to skip cache operations for this image because it will not be used on the local core */
    DVP_S32  skipCacheOpFlush;      /*!<  Used to indicate to the DVP lower layers to skip cache operations for this image because it will not be used on the local core */
} DVP_Buffer_t;

/*!
 * \brief This enumeration defines the memory types used in DVP.
 * \ingroup group_system
 */
typedef enum _dvp_mtype_e {
    DVP_MTYPE_MPUCACHED_VIRTUAL = 0,     /*!<  MPU has a cached virtual view of the memory, but the accelerators can not be utilized in this memory */
#if defined(DVP_USE_TILER) || defined(DVP_USE_ION) || defined(DVP_USE_BO)
    DVP_MTYPE_MPUCACHED_1DTILED,         /*!<  MPU has a cached virtual view of the memory, and the accelerators can access it as contiguous 1D tiled memory */
    DVP_MTYPE_MPUNONCACHED_1DTILED,      /*!<  MPU has a non-cached 1D virtual view of the memory, and the accelerators can operate 1D tiled memory (test mode)  */
    DVP_MTYPE_MPUNONCACHED_2DTILED,      /*!<  MPU has a non-cached 2D virtual view of the memory, and the accelerators can operate optimally with 2D tiled memory  */
#endif
    //#if defined(DVP_USE_TILER) || defined(DVP_USE_GRALLOC)
    DVP_MTYPE_DISPLAY_2DTILED,           /*!<  MPU has a non-cached 2D virtual view of memory allocated out of the display subsystem */
    //#endif
#if defined(DVP_USE_GRALLOC) || defined(DVP_USE_BO)
    DVP_MTYPE_GRALLOC_2DTILED,           /*!<  MPU allocated GFX 2D buffers */
#endif
#if defined(DVP_USE_SHARED_T)
    DVP_MTYPE_MPUCACHED_VIRTUAL_SHARED,  /*!<  MPU has a cached virtual view of the memory, but the accelerators can not be utilized in this memory. The memory can be shared among processes.*/
#endif
    DVP_MTYPE_CAMERA_1DTILED,            /*!<  Virtual shared memory allocated by Camera Service */
    DVP_MTYPE_MAX,

    /*! The default memory allocation type on the platform. */
#if defined(DVP_USE_TILER) || defined(DVP_USE_ION)
    DVP_MTYPE_DEFAULT = DVP_MTYPE_MPUCACHED_1DTILED,
#elif defined(DVP_USE_BO)
    DVP_MTYPE_DEFAULT = DVP_MTYPE_MPUCACHED_1DTILED,
#else
    DVP_MTYPE_DEFAULT = DVP_MTYPE_MPUCACHED_VIRTUAL,
#endif

    /*! The memory type used to contain command structures shared across mulitple cores */
#if defined(DVP_USE_ION)
    DVP_MTYPE_KERNELGRAPH = DVP_MTYPE_MPUNONCACHED_1DTILED,
#elif defined(DVP_USE_BO)
    DVP_MTYPE_KERNELGRAPH = DVP_MTYPE_MPUNONCACHED_1DTILED,
#else
    DVP_MTYPE_KERNELGRAPH = DVP_MTYPE_MPUCACHED_VIRTUAL,
#endif
} DVP_MemType_e;

/*!
 * \brief The image structure defines a 1D, or 2D image of up to DVP_MAX_PLANES with arbitrary pixel depth and line stride.
 * \ingroup group_images
 */
typedef struct _dvp_image_t {
    DVP_U08 *pData[DVP_MAX_PLANES];  /*!<  The byte pointer to the image data, the color type will indicate how this pointer is indexed. Each plane has a separate pointer. Up to DVP_MAX_PLANES are supported */
    DVP_U08 *pBuffer[DVP_MAX_PLANES]; /*!<  The raw allocated buffers per plane. */
    DVP_U32  planes;                 /*!<  This indicates the number of planes in the planes array. An interleaved format will only have 1 plane. */
    DVP_U32  width;                  /*!<  pixel width of the (sub-)image */
    DVP_U32  height;                 /*!<  pixel height of the (sub-)image */
    DVP_U32  bufWidth;               /*!<  pixel width of the raw image */
    DVP_U32  bufHeight;              /*!<  pixel height of the raw image */
    DVP_U32  x_start;                /*!<  X Pixel Offset into the raw image for sub-image */
    DVP_U32  y_start;                /*!<  Y Pixel Offset into the raw image for sub-image */
    DVP_S32  x_stride;               /*!<  byte distance from one pixel to the next pixel in the x direction, not necessarily the same as pixel size */
    DVP_S32  y_stride;               /*!<  byte distance from one pixel to the next pixel in the y direction, not necessarily the same as pixel size times width. Note: all planes should use the same stride, thus should be allocated identically. Subsampled planes will be overallocated. */
    DVP_U32  color;                  /*!<  used a FourCC code (http://www.fourcc.org) \see fourcc_t */
    DVP_U32  numBytes;               /*!<  The allocated byte range of this image. */
    DVP_U32  memType;                /*!<  The memory mapping type, \see DVP_MemType_e */
    DVP_PTR  reserved;               /*!<  Used for internal tracking */
    DVP_S32  skipCacheOpInval;       /*!<  Used to indicate to the DVP lower layers to skip cache operations for this image because it will not be used on the local core */
    DVP_S32  skipCacheOpFlush;       /*!<  Used to indicate to the DVP lower layers to skip cache operations for this image because it will not be used on the local core */
} DVP_Image_t;

/*!
 * \brief This structure allows for image write-out during graph execution for debugging.
 * The name of the file will be determeined by the PYUV library. The name here will
 * be used to uniquely identify the file. The full filename will be stored in fullname.
 * \ingroup group_kernels
 */
typedef struct _dvp_image_debug_t {
    DVP_Image_t image;              /*!<  The image to dump */
    char        name[DVP_MAX_NAME]; /*!<  The short prefix name of the file. */
    char        path[MAX_PATH];     /*!<  The path to store the file in */
    char        fullname[MAX_PATH]; /*!<  The full path qualified name of the file */
#if defined(DVP_USE_FS)
    FILE       *fp;                 /*!<  (Internal) The open FILE pointer */
#endif
} DVP_ImageDebug_t;

/*!
 * \brief This structure allows for buffer write-out during graph execution for debugging.
 * \ingroup group_kernels
 */
typedef struct _dvp_buffer_debug_t {
    DVP_Buffer_t buffer;             /*!<  The buffer to dump. */
    char         fullname[MAX_PATH]; /*!<  The full path qualified name of the file */
#if defined(DVP_USE_FS)
    FILE        *fp;                 /*!<  (Internal) The open FILE pointer */
#endif
} DVP_BufferDebug_t;

/*!
 * \brief A transform structure is used on any kernel which goes from one image format to another with no parameters needed other than the parameters of the supplied images.
 * \ingroup group_kernels
 */
typedef struct _dvp_transform_t {
    DVP_Image_t input;      /*!<  Input image */
    DVP_Image_t output;     /*!<  Output image */
} DVP_Transform_t;

/*!
 * \brief This structure is used in conjunction with Morphology operations like Erosion and Dilation.
 * \ingroup group_kernels
 */
typedef struct _dvp_morphology_t {
    DVP_Image_t input;      /*!<  Input image */
    DVP_Image_t output;     /*!<  Output image */
    DVP_Image_t mask;       /*!<  Mask matrix, only used with mask operations */
} DVP_Morphology_t;

/*!
 * \brief This structure is used in kernels that need bounding information.
 * \ingroup group_kernels
 */
typedef struct _dvp_bounds_t {
    DVP_U08 *pData;         /*!<  Boundary array */
    DVP_U32  width;         /*!<  Width */
    DVP_U32  height;        /*!<  Height */
} DVP_Bounds_t;

/*!
 * \brief This structure is used with IIR Filters.
 * \ingroup group_kernels
 */
typedef struct _dvp_iir_t {
    DVP_Image_t  input;     /*!<  Input image */
    DVP_Image_t  output;    /*!<  Output image */
    DVP_Bounds_t bounds[2]; /*!<  Horz={left, right}, Vert={top,bottom} */
    DVP_Image_t  scratch;      // \TODO remove
    DVP_U16      weight;    /*!<  Weight */
} DVP_IIR_t;

/*!
 * \brief This strucutre is used with Image Convolution operations.
 * \ingroup group_kernels
 */
typedef struct _dvp_image_conv_t {
    DVP_Image_t input;      /*!<  Input image */
    DVP_Image_t output;     /*!<  Output image */
    DVP_Image_t mask;       /*!<  A signed 8-bit mask matrix (3x3, 5x5, 7x7, or MxN) */
    DVP_U16     shiftMask;  /*!<  Number of bits to right shift the mask values */
} DVP_ImageConvolution_t;

/*!
 * \brief This structure is used with Canny 2d Gradients.
 * \ingroup group_kernels
 */
typedef struct _dvp_canny_2d_gradient_t {
    DVP_Image_t input;      /*!<  Input image, 8bit */
    DVP_Image_t outGradX;   /*!<  Output horizontal gradient, 16bit */
    DVP_Image_t outGradY;   /*!<  Output vertical gradient, 16bit */
    DVP_Image_t outMag;     /*!<  Output magnitude, 16bit */
} DVP_Canny2dGradient_t;

/*!
 * \brief This structure is used with Canny Non-maximum suppression.
 * \ingroup group_kernels
 */
typedef struct _dvp_canny_non_maximum_t {
    DVP_Image_t inMag;      /*!<  Input magnitude, 16bit */
    DVP_Image_t inGradX;    /*!<  Input horizontal gradient, 16bit */
    DVP_Image_t inGradY;    /*!<  Input vertical gradient, 16bit */
    DVP_Image_t output;     /*!<  Output image, 8bit */
} DVP_CannyNonMaxSuppression_t;

/*!
 * \brief This structure is used with Canny Hysterisis Threshholding.
 * \ingroup group_kernels
 */
typedef struct _dvp_canny_hyst_thresh_t {
    DVP_Image_t inMag;      /*!<  Input magnitude, 16bit */
    DVP_Image_t inEdgeMap;  /*!<  Input edge map, 8bit */
    DVP_Image_t output;     /*!<  Output image, 8bit */
    DVP_U08     loThresh;   /*!<  Input lower threshold */
    DVP_U08     hiThresh;   /*!<  Input upper threshold */
    DVP_U32     numEdges;   /*!<  Output number of edges */
} DVP_CannyHystThresholding_t;

/*!
 * \brief This structure is used with thresholding operations.
 * \ingroup group_kernels
 */
typedef struct _dvp_threshold_t {
    DVP_Image_t input;      /*!<  Input image */
    DVP_Image_t output;     /*!<  Output image */
    DVP_S16     thresh;     /*!<  Threshold value */
} DVP_Threshold_t;

/*
 * \brief This structure is used with inteleaved to planar(more than 8bits) kernels.
 * \ingroup group_kernels
 */
typedef struct _dvp_int2pl_t {
    DVP_Image_t input;      /*!<  Input image */
    DVP_Image_t output1;    /*!<  Output image 1 */
    DVP_Image_t output2;    /*!<  Output image 2 */
    DVP_Image_t output3;    /*!<  Output image 3 */
    DVP_Buffer_t scratch;   /*!<  Scratch buffer */
    DVP_S32 factor;         /*!<  Factor value */
} DVP_Int2Pl_t;

/*!
 * \brief This structure is used with SAD Kernels.
 * \ingroup group_kernels
 */
typedef struct _dvp_sad_t {
    DVP_Image_t input;          /*!<  Input image */
    DVP_Image_t output;         /*!<  Output image */
    DVP_Image_t refImg;         /*!<  Reference image */
    DVP_U16     shiftMask;      /*!<  Downshift value */
    DVP_U32     refPitch;       /*!<  Reference pitch */
    DVP_U16     refStartOffset; /*!<  Reference offset */
} DVP_SAD_t;

/*!
 * \brief This structure is used with Histogram Kernels.
 * \ingroup group_kernels
 */
typedef struct _dvp_histogram_t {
    DVP_Image_t  input;         /*!<  Input Image */
    DVP_Buffer_t edges;         /*!<  Edges of each bin, array size=numBins, the last edge in this array is its own bin */
    DVP_U32      numBins;       /*!<  Used for weighted and nonweighted 1D histograms; must be a multiple of 2 */
    DVP_U32      dimX;          /*!<  Used for nD histograms; must be a multiple of 2 */
    DVP_U16      binWeight;     /*!<  Used for normal histograms */
    DVP_U16      clearFlag;     /*!<  When set to 1, the output is automatically cleared before processing */
    DVP_Buffer_t numBinsArray;  /*!<  Used for weighted nD histograms */
    DVP_Buffer_t binWeights;    /*!<  Used for weighted histograms */
    DVP_Buffer_t hOut;          /*!<  Final output Histogram, size=numBins */
    DVP_Buffer_t histArray;     /*!<  Internal scratch memory; initialize to 0 first time, size = largest expected input value+1 */
    DVP_Buffer_t h[3];          /*!<  Internal scratch memory; h[0] needed for 16bit & all h[0,1,2] needed for 8bit; size=numBins */
    DVP_Buffer_t scratch;       /*!<  Internal scratch memory; Used in the ND histogram */
} DVP_Histogram_t;

/*!
 * \brief This structure is used with image pyramid kernels, where the output is a single buffer with images of varying resoltions.
 * \ingroup group_kernels
 */
typedef struct _dvp_pyramid_t {
    DVP_Image_t input;
    DVP_Buffer_t output;
} DVP_Pyramid_t;

/*!
 * \brief This structure is used with Gamma kernels.
 * \ingroup group_kernels
 */
typedef struct _dvp_gamma_t {
    DVP_Image_t input;
    DVP_Image_t output;
    DVP_U08     gammaLut[256];
} DVP_Gamma_t;

/*! \brief An enumeration of all registered DVP feature sets. Features are generic
 * kernels that least two or more libraries implement. Implementations must be data-equivalent.
 * \ingroup group_kernels
 */
typedef enum _dvp_kfeat_e {
    DVP_KF_INVALID = 0,         /*!< An invalid kernel feature range */
    DVP_KF_DEBUG,               /*!< The debugging features range */
    DVP_KF_COLOR_CONVERT,       /*!< The color conversion feature range */
    DVP_KF_FILTER,              /*!< The image convolution feature range */
    DVP_KF_MORPH,               /*!< The image mophology feature range */
    DVP_KF_CANNY,               /*!< The canny image features range */
    DVP_KF_CONVERT,             /*!< The image bit depth conversion feature range */
    DVP_KF_IIR,                 /*!< The recursive image first order filters */
    DVP_KF_NONMAXSUPPRESS,      /*!< The non-maximum suppression filters */
    DVP_KF_CONV,                /*!< The image convolution filters */
    DVP_KF_THR,                 /*!< The image thresholding filters */
    DVP_KF_SOBEL,               /*!< The image sobel edge filters */
    DVP_KF_INTEGRAL,            /*!< The integral feature range */
    DVP_KF_OPTIMIZED = 0x30000, /*!< This base range will used for local KGM optimized implementations */
    DVP_KF_MAX       = 0x40000, /*!< This is the maximum feature set range */
} DVP_KernelFeature_e;

/*! \brief An enumeration of all registered DVP algorithm libraries. This values are used to set the algo-specific ranges.
 * \ingroup group_kernels
 */
typedef enum _dvp_klib_e {
    DVP_KL_INVALID = 0,         /*!< The invalid kernel range. */
    DVP_KL_YUV,                 /*!< A set of NEON functions designed around YUV color spaces */
    DVP_KL_IMGFILTER,           /*!< A set of NEON functions designed around 3x3 convolutions, and 3x1 filters */
    DVP_KL_OCL,                 /*!< A set of predefined OpenCL kernels. */
    DVP_KL_VLIB,                /*!< Texas Instruments Vision Library 2.2 */
    DVP_KL_IMGLIB,              /*!< Texas Instruments Imaging Library */
    DVP_KL_RVM,                 /*!< Texas Instruments Rear-View Module for Homography Transforms */
    DVP_KL_VRUN,                /*!< Texas Instruments VRUN algo library for iMX */
    DVP_KL_TISMO,               /*!< Texas Instruments Stereo Module */
    DVP_KL_ORB,                 /*!< A fast ORB implementation */
    DVP_KL_ARTI,                /*!< Augmented Reality Library from Texas Instruments */
    DVP_KL_POINTCLOUD,          /*!< A point-cloud library */
    DVP_KL_DSPLIB,              /*!< A set of DSP optimized algorithms */
    DVP_KL_DEI,                 /*!< Deinterlacer algorithm library */
    DVP_KL_TISMOV02,            /*!< Texas Instruments Stereo Module Version 2 */
    // new libraries get added here
    DVP_KL_EXTERNAL_LIBRARIES = 0x80,    /*!< 3rd parties can add algos libraries from here */
    DVP_KL_LIBRARY_MAX = 0xFF,  /*!< The maximum library set */
} DVP_KernelLibrary_e;

/*! \brief The maxium range a library set can have.
 * \ingroup group_kernels
 */
#define DVP_KN_RANGE                    (0x1000)    // 4096 kernels per library should be sufficient?

/*! \brief We cut the enum space in half. The lower half is for "features" which multiple libraries may overload.
 * \ingroup group_kernels
 */
#define DVP_KN_FEATURE_START            (0x0)

/*! \brief The upper half of the kernel space is for libraries to have explicitly targeted kernels.
 * \ingroup group_kernels
 */
#define DVP_KN_LIBRARY_START            (0x40000000)

/*! \brief Used to retrieve the library which a kernel is in.
 * \ingroup group_kernels
 */
#define DVP_KN_LIBRARY_MASK(k)          ((0x000FF000 & k) >> 12)

/*!
 * \brief Set your base kernel enum to your DVP_KN_LIBRARY_BASE(DVP_KL_XXXXX)
 * \ingroup group_kernels
 */
#define DVP_KN_LIBRARY_BASE(library)    (DVP_KN_LIBRARY_START + (library * DVP_KN_RANGE))

/*!
 * \brief Sets the base value of a feature set.
 * \ingroup group_kernels
 */
#define DVP_KN_FEATURE_BASE(set)        (DVP_KN_FEATURE_START + ((set) * DVP_KN_RANGE))

/*! \brief An enumeration of all possible DVP kernels (or functions).
 * \note New features should be added either in existing ranges if applicable but
 * always at the _end_ of a range.
 * \ingroup group_kernels
 */
typedef enum _dvp_knode_e {
    DVP_KN_INVALID = 0,                 /*!<  An undefined kernel */

    /*!
     * \brief Debug Feature Base
     * \note This is a placeholder enumeration, not a valid kernel
     */
    DVP_KF_DEBUG_BASE = DVP_KN_FEATURE_BASE(DVP_KF_DEBUG),

    /*!
     * Copies the input image to the output memory, including cropping and padding\n
     * Configuration Structure: DVP_Transform_t
     */
    DVP_KN_COPY,

    /*!
     * Copies the input image to the output memory (DEPRECIATED)\n
     * Configuration Structure: DVP_Transform_t
     */
    DVP_KN_ECHO = DVP_KN_COPY,

    /*!
     * Writes an images to a file\n
     * Configuration Structure: DVP_ImageDebug_t
     */
    DVP_KN_IMAGE_DEBUG,

    /*!
     * Writes a buffer to a file\n
     * Configuration Structure: DVP_BufferDebug_t
     */
    DVP_KN_BUFFER_DEBUG,

    /*!
     * No Operation kernel.  Used, typically during debug, to turn a kernel off from executing
     * without modifying the remainder of the kernel node structure.  This can be done by replacing
     * the kernel function name with "DVP_KGM_NOOP".\n
     * Configuration Structure: Not Applicable
     */
    DVP_KN_NOOP,

    /*!
     * \brief Color Conversion Feature Base
     * \note This is a placeholder enumeration, not a valid kernel
     */
    DVP_KF_COLOR_CONVERT_BASE = DVP_KN_FEATURE_BASE(DVP_KF_COLOR_CONVERT),

    /*!
     * Converts FOURCC_UYVY or FOURCC_VYUY to FOURCC_Y800 image\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input Image color type supported: FOURCC_UYVY or FOURCC_VYUY
     * \param [out] output Image color type supported: FOURCC_Y800
     */
    DVP_KN_XYXY_TO_Y800,

    /*!
     * Converts FOURCC_YVYU or FOURCC_YUY2 format to FOURCC_Y800 image.\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input Image color type supported: FOURCC_YVYU or FOURCC_YUY2
     * \param [out] output Image color type supported: FOURCC_Y800
     */
    DVP_KN_YXYX_TO_Y800,

    /*!
     * Converts 1 plane 8 bit per pixel luma to FOURCC_UYVY\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input Image color type supported: FOURCC_Y800
     * \param [out] output Image color type supported: FOURCC_UYVY
     */
    DVP_KN_Y800_TO_XYXY,

    /*!
     * Converts FOURCC_UYVY format to 8 bits per pixels planar RGB using BT.601. (VLIB-API.Sec 58.0)\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input Image color type supported: FOURCC_UYVY
     * \param [out] output Image color type supported: FOURCC_RGBP
     */
    DVP_KN_UYVY_TO_RGBp,

    /*!
     * Converts FOURCC_UYVY format to FOURCC_IYUV or FOURCC_YV12 format. Chroma planes are downsampled vertically. (VLIB-API.Sec 55.0)\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input Image color type supported: FOURCC_UYVY
     * \param [out] output Image color type supported: FOURCC_IYUV or FOURCC_YV12
     */
    DVP_KN_UYVY_TO_YUV420p,

    /*!
     * Converts FOURCC_UYVY format to FOURCC_YU16 or FOURCC_YV16 format. (VLIB-API.Sec 54.0)\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input Image color type supported: FOURCC_UYVY
     * \param [out] output Image color type supported: FOURCC_YV16 or FOURCC_YU16
     */
    DVP_KN_UYVY_TO_YUV422p,

    /*!
     * Converts FOURCC_UYVY format to FOURCC_YU24 or FOURCC_YV24 format. Chroma planes are doubled.\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input Image color type supported: FOURCC_UYVY
     * \param [out] output Image color type supported: FOURCC_YV24 or FOURCC_YU24
     */
    DVP_KN_UYVY_TO_YUV444p,

    /*!
     * Converts FOURCC_UYVY format to 1 plane 8 bit pixel packed BGR byte ordered 24 bit per pixel format (FOURCC_BGR) using BT.601\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input Image color type supported: FOURCC_UYVY
     * \param [out] output Image color type supported: FOURCC_BGR
     */
    DVP_KN_UYVY_TO_BGR,

    /*!
     * Converts FOURCC_UYVY format to 1 plane 565 bit pixel packed BGR ordered format (FOURCC_RGB565) using BT.601 \n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input Image color type supported: FOURCC_UYVY
     * \param [out] output Image color type supported: FOURCC_RGB565
     */
    DVP_KN_YUV422p_TO_RGB565,

    /*!
     * Converts FOURCC_UYVY format to 3 plane HSV (Hue, Saturation, Value) 8 bit per plane format. (VLIB-API.Sec 56.0) \n
     * Configuration Structure: DVP_Int2Pl_t
     * \param [in] input Image color type supported: FOURCC_UYVY
     * \param [out] output1 Image color type supported: FOURCC_Y16
     * \param [out] output2 Image color type supported: FOURCC_Y800
     * \param [out] output3 Image color type supported: FOURCC_Y800
     */
    DVP_KN_UYVY_TO_HSLp,

    /*!
     * Converts FOURCC_UYVY format to 3 plane LAB D65 8 bit per pixel color space (VLIB-API.Sec 59.0)\n
     * Configuration Structure: DVP_Int2Pl_t
     * \param [in] input Image color type supported: FOURCC_UYVY
     * \param [out] output1 Image color type supported: FOURCC_Y16
     * \param [out] output2 Image color type supported: FOURCC_Y16
     * \param [out] output3 Image color type supported: FOURCC_Y16
     */
    DVP_KN_UYVY_TO_LABp,

    /*!
     * Converts FOURCC_IYUV or FOURCC_YV12 into FOURCC_UYVY. Chroma are doubled vertically\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input Image color type supported: FOURCC_IYUV or FOURCC_YV12
     * \param [out] output Image color type supported: FOURCC_UYVY
     */
    DVP_KN_YUV420p_TO_UYVY,

    /*!
     * Converts FOURCC_YU16 or FOURCC_YV16 into FOURCC_UYVY.\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input Image color type supported: FOURCC_YV16 or FOURCC_YU16
     * \param [out] output Image color type supported: FOURCC_UYVY
     */
    DVP_KN_YUV422p_TO_UYVY,

    /*!
     * Converts FOURCC_YU24 or FOURCC_YV24 into FOURCC_UYVY. Chroma are downsampled.\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input Image color type supported: FOURCC_YU24 or FOURCC_YV24
     * \param [out] output Image color type supported: FOURCC_UYVY
     */
    DVP_KN_YUV444p_TO_UYVY,

    /*!
     * Converts FOURCC_YU24 or FOURCC_YV24 into 8 bits per pixels planar RGB using BT.601.\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input Image color type supported: FOURCC_YU24 or FOURCC_YV24
     * \param [out] output Image color type supported: FOURCC_RGBP
     */
    DVP_KN_YUV444p_TO_RGBp,

    /*!
     * Converts FOURCC_NV12 into FOURCC_YU24 or FOURCC_YV24. Luma plane is half-scale downsampled. Output is 1/2w 1/2h from input.\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input Image color type supported: FOURCC_NV12
     * \param [out] output Image color type supported: FOURCC_YV24 or FOURCC_YU24
     */
    DVP_KN_NV12_TO_YUV444p,

    /*!
     * Converts 1 plane RGB 888 FOURCC_BGR to FOURCC_UYVY\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input Image color type supported: FOURCC_RGB888 or FOURCC_BGR
     * \param [out] output Image color type supported: FOURCC_UYVY
     */
    DVP_KN_BGR3_TO_UYVY,

    /*!
     * Converts 1 plane RGB 888 FOURCC_BGR to FOURCC_NV12\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input Image color type supported: FOURCC_RGB888 or FOURCC_BGR
     * \param [out] output Image color type supported: FOURCC_NV12
     */
    DVP_KN_BGR3_TO_NV12,

    /*!
     * Converts FOURCC_NV12 to UYVY.\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input Image color type supported: FOURCC_NV12
     * \param [out] output Image color type supported: FOURCC_UYVY
     */
    DVP_KN_NV12_TO_UYVY,

    /*!
     * Converts FOURCC_IYUV or FOURCC_YV12 into 8 bits per pixels planar RGB using BT.601.\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input Image color type supported: FOURCC_IYUV or FOURCC_YV12
     * \param [out] output Image color type supported: FOURCC_RGBP
     */
    DVP_KN_YUV420p_TO_RGBp,

    /*!
     * Converts 1 plane RGB 888 FOURCC_BGR to FOURCC_IYUV\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input Image color type supported: FOURCC_RGB888 or FOURCC_BGR
     * \param [out] output Image color type supported: FOURCC_IYUV
     */
    DVP_KN_BGR3_TO_IYUV,

    /*!
     * \brief Predefined Filter Feature Base
     * \note This is a placeholder enumeration, not a valid kernel
     */
    DVP_KF_FILTER_BASE = DVP_KN_FEATURE_BASE(DVP_KF_FILTER),

    /*!
     * Image Convolution of FOURCC_Y800 of data using the Sobel Gx and Gy operators\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input Image color type supported: FOURCC_Y800
     * \param [out] output Image color type supported: FOURCC_Y800
     */
    DVP_KN_SOBEL_8,

    /*!
     * Image Convolution of FOURCC_Y800 of data using the Scharr Gx and Gy operators\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input Image color type supported: FOURCC_Y800
     * \param [out] output Image color type supported: FOURCC_Y800
     */
    DVP_KN_SCHARR_8,

    /*!
     * Image Convolution of FOURCC_Y800 of data using the Scharr Gx and Gy operators\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input Image color type supported: FOURCC_Y800
     * \param [out] output Image color type supported: FOURCC_Y800
     */
    DVP_KN_KROON_8,

    /*!
     * Image Convolution of FOURCC_Y800 of data using the Prewitt Gx and Gy operators\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input Image color type supported: FOURCC_Y800
     * \param [out] output Image color type supported: FOURCC_Y800
     */
    DVP_KN_PREWITT_8,

    /*!
     * \brief Morphology Feature Base
     * \note This is a placeholder enumeration, not a valid kernel
     */
     DVP_KF_MORPH_BASE = DVP_KN_FEATURE_BASE(DVP_KF_MORPH),

    /*!
     * Image Dilation using a 3x3 Cross Pattern : [0,1,0],[1,1,1],[0,1,0]\n
     * Configuration Structure: DVP_Morphology_t
     * \param [in] input Image color type supported: FOURCC_Y800
     * \param [out] output Image color type supported: FOURCC_Y800
     * \param [in] mask Not applicable
     */
    DVP_KN_DILATE_CROSS,

    /*!
     * Image Dilation using a user specified 3x3 Mask pattern\n
     * Configuration Structure: DVP_Morphology_t
     * \param [in] input Image color type supported: FOURCC_Y800
     * \param [out] output Image color type supported: FOURCC_Y800
     * \param [in] mask Image color type supported: FOURCC_Y800
     */
    DVP_KN_DILATE_MASK,

    /*!
     * Image Dilation using a 3x3 Square pattern : [1,1,1],[1,1,1],[1,1,1]\n
     * Configuration Structure: DVP_Morphology_t
     * \param [in] input Image color type supported: FOURCC_Y800
     * \param [out] output Image color type supported: FOURCC_Y800
     * \param [in] mask Not applicable
     */
    DVP_KN_DILATE_SQUARE,

    /*!
     * Image Erosion using a 3x3 Cross Pattern : [0,1,0],[1,1,1],[0,1,0]\n
     * Configuration Structure: DVP_Morphology_t
     * \param [in] input Image color type supported: FOURCC_Y800
     * \param [out] output Image color type supported: FOURCC_Y800
     * \param [in] mask Not applicable
     */
    DVP_KN_ERODE_CROSS,

    /*!
     * Image Erosion using a user specified 3x3 Mask pattern\n
     * Configuration Structure: DVP_Morphology_t
     * \param [in] input Image color type supported: FOURCC_Y800
     * \param [out] output Image color type supported: FOURCC_Y800
     * \param [in] mask Image color type supported: FOURCC_Y800
     */
    DVP_KN_ERODE_MASK,

    /*!
     * Image Erosion using a 3x3 Square pattern : [1,1,1],[1,1,1],[1,1,1]\n
     * Configuration Structure: DVP_Morphology_t
     * \param [in] input Image color type supported: FOURCC_Y800
     * \param [out] output Image color type supported: FOURCC_Y800
     * \param [in] mask Not applicable
     */
    DVP_KN_ERODE_SQUARE,

    /*!
     * \brief Canny Feature Base
     * \note This is a placeholder enumeration, not a valid kernel
     */
    DVP_KN_CANNY_BASE = DVP_KN_FEATURE_BASE(DVP_KF_CANNY),

    /*!
     * Canny Image Smoothing (7x7 Image Convolution)\n
     * Configuration Structure: DVP_ImageConvolution_t
     * \param [in] input Image color type supported: FOURCC_Y800
     * \param [out] output Image color type supported: FOURCC_Y800
     * \param [in] mask Image color type supported: FOURCC_Y800, signed
     */
    DVP_KN_CANNY_IMAGE_SMOOTHING,

    /*!
     * Canny 2D Gradient from FOURCC_Y800 to FOURCC_Y16 (VLIB-API.Sec 19.0)\n
     * Configuration Structure: DVP_Canny2dGradient_t
     * \param [in] input Image color type supported: FOURCC_Y800
     * \param [out] outGradX Image color type supported: FOURCC_Y16
     * \param [out] outGradY Image color type supported: FOURCC_Y16
     * \param [out] outMag Image color type supported: FOURCC_Y16
     */
    DVP_KN_CANNY_2D_GRADIENT,

    /*!
     * Canny Non-Maximum Suppression from FOURCC_Y16 data to FOURCC_Y800 data)\n
     * Configuration Structure: DVP_CannyNonMaxSuppression_t
     * \param [in] inGradX Image color type supported: FOURCC_Y16
     * \param [in] inGradY Image color type supported: FOURCC_Y16
     * \param [in] inMag Image color type supported: FOURCC_Y16
     * \param [out] output Image color type supported: FOURCC_Y800
     */
    DVP_KN_CANNY_NONMAX_SUPPRESSION,

    /*!
     * Canny Hysterisis Threshholding from FOURCC_Y800 edge map data to FOURCC_Y800 edge image\n
     * Configuration Structure: DVP_CannyHystThresholding_t
     * \param [in] inMag Image color type supported: FOURCC_Y16
     * \param [in] inEdgeMap Image color type supported: FOURCC_Y800
     * \param [out] output Image color type supported: FOURCC_Y800
     */
    DVP_KN_CANNY_HYST_THRESHHOLD,

    /*!
     * \brief Convert Feature Base
     * \note This is a placeholder enumeration, not a valid kernel
     */
    DVP_KN_CONVERT_BASE = DVP_KN_FEATURE_BASE(DVP_KF_CONVERT),

    /*!
     * Converts an 8 bit planar image to 1 bit (stored in 8 bits) by down shifting all bytes by 7\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input Image color type supported: FOURCC_Y800
     * \param [out] output Image color type supported: FOURCC_Y800
     */
    DVP_KN_THRESHOLD,

    /*!
     * Converts a 8 bit to 16 bit per pixel or 16 bit to 8 bit per pixel planar image\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input Image color type supported: FOURCC_Y800 or FOURCC_Y16
     * \param [out] output Image color type supported: FOURCC_Y16  or FOURCC_Y800
     */
    DVP_KN_XSTRIDE_CONVERT,

    /*!
     * Converts a 8 bit to 16 bit per pixel or 16 bit to 8 bit per pixel planar image with 8 bit shift\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input Image color type supported: FOURCC_Y800 or FOURCC_Y16
     * \param [out] output Image color type supported: FOURCC_Y16  or FOURCC_Y800
     */
    DVP_KN_XSTRIDE_SHIFT,

    /*!
     * \brief IIR Feature Base
     * \note This is a placeholder enumeration, not a valid kernel
     */
    DVP_KN_IIR_BASE = DVP_KN_FEATURE_BASE(DVP_KF_IIR),

    /*!
     * Recursive First Order IIR Filter, Horizontal, FOURCC_Y800 input and output\n
     * Configuration Structure: DVP_IIR_t
     * \param [in] input Image color type supported: FOURCC_Y800
     * \param [out] output Image color type supported: FOURCC_Y800
     * \param [in] scratch Image color type supported: FOURCC_Y800
     */
    DVP_KN_IIR_HORZ,

    /*!
     * Recursive First Order IIR Filter, Vertical, FOURCC_Y800 input and output\n
     * Configuration Structure: DVP_IIR_t
     * \param [in] input Image color type supported: FOURCC_Y800
     * \param [out] output Image color type supported: FOURCC_Y800
     * \param [in] scratch Image color type supported: FOURCC_Y800
     */
    DVP_KN_IIR_VERT,

    /*!
     * \brief Nonmaximum Suppression Feature Base
     * \note This is a placeholder enumeration, not a valid kernel
     */
    DVP_KN_NONMAXSUPPRESS_BASE = DVP_KN_FEATURE_BASE(DVP_KF_NONMAXSUPPRESS),

    /*!
     * Non-maximum suppression using a local neighborhood size of 3x3 on FOURCC_Y16 data\n
     * Configuration Structure: DVP_Threshold_t
     * \param [in] input Image color type supported: FOURCC_Y16
     * \param [out] output Image color type supported: FOURCC_Y800
     */
    DVP_KN_NONMAXSUPPRESS_3x3_S16,

    /*!
     * Non-maximum suppression using a local neighborhood size of 5x5 on FOURCC_Y16 data\n
     * Configuration Structure: DVP_Threshold_t
     * \param [in] input Image color type supported: FOURCC_Y16
     * \param [out] output Image color type supported: FOURCC_Y800
     */
    DVP_KN_NONMAXSUPPRESS_5x5_S16,

    /*!
     * Non-maximum suppression using a local neighborhood size of 7x7 on FOURCC_Y16 data\n
     * Configuration Structure: DVP_Threshold_t
     * \param [in] input Image color type supported: FOURCC_Y16
     * \param [out] output Image color type supported: FOURCC_Y800
     */
    DVP_KN_NONMAXSUPPRESS_7x7_S16,

    /*!
     * \brief Image Convolution Feature Base
     * \note This is a placeholder enumeration, not a valid kernel
     */
    DVP_KN_CONV_BASE = DVP_KN_FEATURE_BASE(DVP_KF_CONV),

    /*!
     * Image Convolution using an arbitrary 3x3 matrix on FOURCC_Y800 data\n
     * Configuration Structure: DVP_ImageConvolution_t
     * \param [in] input Image color type supported: FOURCC_Y800
     * \param [out] output Image color type supported: FOURCC_Y800
     * \param [in] mask Image color type supported: FOURCC_Y800
     */
    DVP_KN_CONV_3x3,

    /*!
     * Image Convolution using an arbitrary 5x5 matrix on FOURCC_Y800 data\n
     * Configuration Structure: DVP_ImageConvolution_t
     * \param [in] input Image color type supported: FOURCC_Y800
     * \param [out] output Image color type supported: FOURCC_Y800
     * \param [in] mask Image color type supported: FOURCC_Y800
     */
    DVP_KN_CONV_5x5,

    /*!
     * Image Convolution using an arbitrary 7x7 matrix on FOURCC_Y800 data\n
     * Configuration Structure: DVP_ImageConvolution_t
     * \param [in] input Image color type supported: FOURCC_Y800
     * \param [out] output Image color type supported: FOURCC_Y800
     * \param [in] mask Image color type supported: FOURCC_Y800
     */
    DVP_KN_CONV_7x7,

    /*!
     * \brief Threshold Feature Base
     * \note This is a placeholder enumeration, not a valid kernel
     */
    DVP_KN_THR_BASE = DVP_KN_FEATURE_BASE(DVP_KF_THR),

    /*!
     * Image Thresholding : {out[i] = (in[i] >  thr) ?  255  : in[i]} for 8-bit  input\n
     * Configuration Structure: DVP_Threshold_t
     * \param [in] input Image color type supported: FOURCC_Y800
     * \param [out] output Image color type supported: FOURCC_Y800
     */
    DVP_KN_THR_GT2MAX_8,

    /*!
     * Image Thresholding : {out[i] = (in[i] >  thr) ? 65535 : in[i]} for 16-bit input\n
     * Configuration Structure: DVP_Threshold_t
     * \param [in] input Image color type supported: FOURCC_Y16
     * \param [out] output Image color type supported: FOURCC_Y16
     */
    DVP_KN_THR_GT2MAX_16,

    /*!
     * Image Thresholding : {out[i] = (in[i] >  thr) ?  thr  : in[i]} for 8-bit  input\n
     * Configuration Structure: DVP_Threshold_t
     * \param [in] input Image color type supported: FOURCC_Y800
     * \param [out] output Image color type supported: FOURCC_Y800
     */
    DVP_KN_THR_GT2THR_8,

    /*!
     * Image Thresholding : {out[i] = (in[i] >  thr) ?  thr  : in[i]} for 16-bit input\n
     * Configuration Structure: DVP_Threshold_t
     * \param [in] input Image color type supported: FOURCC_Y16
     * \param [out] output Image color type supported: FOURCC_Y16
     */
    DVP_KN_THR_GT2THR_16,

    /*!
     * Image Thresholding : {out[i] = (in[i] <= thr) ?   0   : in[i]} for 8-bit  input\n
     * Configuration Structure: DVP_Threshold_t
     * \param [in] input Image color type supported: FOURCC_Y800
     * \param [out] output Image color type supported: FOURCC_Y800
     */
    DVP_KN_THR_LE2MIN_8,

    /*!
     * Image Thresholding : {out[i] = (in[i] <= thr) ?   0   : in[i]} for 16-bit input\n
     * Configuration Structure: DVP_Threshold_t
     * \param [in] input Image color type supported: FOURCC_Y16
     * \param [out] output Image color type supported: FOURCC_Y16
     */
    DVP_KN_THR_LE2MIN_16,

    /*!
     * Image Thresholding : {out[i] = (in[i] <= thr) ?  thr  : in[i]} for 8-bit  input\n
     * Configuration Structure: DVP_Threshold_t
     * \param [in] input Image color type supported: FOURCC_Y800
     * \param [out] output Image color type supported: FOURCC_Y800
     */
    DVP_KN_THR_LE2THR_8,

    /*!
     * Image Thresholding : {out[i] = (in[i] <= thr) ?  thr  : in[i]} for 16-bit input\n
     * Configuration Structure: DVP_Threshold_t
     * \param [in] input Image color type supported: FOURCC_Y16
     * \param [out] output Image color type supported: FOURCC_Y16
     */
    DVP_KN_THR_LE2THR_16,

    /*!
     * \brief Sobel Feature Base
     * \note This is a placeholder enumeration, not a valid kernel
     */
    DVP_KN_SOBEL_BASE = DVP_KN_FEATURE_BASE(DVP_KF_SOBEL),
    /*!
     * Image Sobel : 3x3 for 8-bit  signed   input\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input Image color type supported: FOURCC_Y800
     * \param [out] output Image color type supported: FOURCC_Y800
     */
    DVP_KN_SOBEL_3x3_8s,

    /*!
     * Image Sobel : 3x3 for 8-bit  unsigned input\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input Image color type supported: FOURCC_Y800
     * \param [out] output Image color type supported: FOURCC_Y800
     */
    DVP_KN_SOBEL_3x3_8,

    /*!
     * Image Sobel : 3x3 for 16-bit signed   input\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input Image color type supported: FOURCC_Y16
     * \param [out] output Image color type supported: FOURCC_Y16
     */
    DVP_KN_SOBEL_3x3_16s,

    /*!
     * Image Sobel : 3x3 for 16-bit unsigned input\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input Image color type supported: FOURCC_Y16
     * \param [out] output Image color type supported: FOURCC_Y16
     */
    DVP_KN_SOBEL_3x3_16,

    /*!
     * Image Sobel : 5x5 for 8-bit  signed   input\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input Image color type supported: FOURCC_Y800
     * \param [out] output Image color type supported: FOURCC_Y800
     */
    DVP_KN_SOBEL_5x5_8s,

    /*!
     * Image Sobel : 5x5 for 8-bit  unsigned input\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input Image color type supported: FOURCC_Y800
     * \param [out] output Image color type supported: FOURCC_Y800
     */
    DVP_KN_SOBEL_5x5_8,

    /*!
     * Image Sobel : 5x5 for 16-bit signed   input\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input Image color type supported: FOURCC_Y16
     * \param [out] output Image color type supported: FOURCC_Y16
     */
    DVP_KN_SOBEL_5x5_16s,

    /*!
     * Image Sobel : 5x5 for 16-bit unsigned input\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input Image color type supported: FOURCC_Y16
     * \param [out] output Image color type supported: FOURCC_Y16
     */
    DVP_KN_SOBEL_5x5_16,

    /*!
     * Image Sobel : 7x7 for 8-bit  signed   input\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input Image color type supported: FOURCC_Y800
     * \param [out] output Image color type supported: FOURCC_Y800
     */
    DVP_KN_SOBEL_7x7_8s,

    /*!
     * Image Sobel : 7x7 for 8-bit  unsigned input\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input Image color type supported: FOURCC_Y800
     * \param [out] output Image color type supported: FOURCC_Y800
     */
    DVP_KN_SOBEL_7x7_8,

    /*!
     * Image Sobel : 7x7 for 16-bit signed   input\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input Image color type supported: FOURCC_Y16
     * \param [out] output Image color type supported: FOURCC_Y16
     */
    DVP_KN_SOBEL_7x7_16s,

    /*!
     * Image Sobel : 7x7 for 16-bit unsigned input\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input Image color type supported: FOURCC_Y16
     * \param [out] output Image color type supported: FOURCC_Y16
     */
    DVP_KN_SOBEL_7x7_16,

    /*!
     * \brief Integral Image Feature Base
     * \note This is a placeholder enumeration, not a valid kernel
     */
    DVP_KN_INTEGRAL_BASE = DVP_KN_FEATURE_BASE(DVP_KF_INTEGRAL),

    /*!
     * Computes a Integral Image over the FOURCC_Y800 data. FOURCC_Y32 output\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input Image color type supported: FOURCC_Y800
     * \param [out] output Image color type supported: FOURCC_Y32
     */
    DVP_KN_INTEGRAL_IMAGE_8,

    /*!
     * Applies a given gamma mapping to an image and produces it as a new output.
     * Configuration Structure: DVP_Transform_t
     * \param [in] input Image color type supported: FOURCC_Y800
     * \param [out] output Image color type supported: FOURCC_Y800
     */
     DVP_KN_GAMMA,

    DVP_KN_LOCAL_OPTIMIZED_BASE =  DVP_KN_FEATURE_BASE(DVP_KF_OPTIMIZED),/*!<  Used by Kernel Graph Managers to define a local optimized kernel set which combine multiple functions */

    /*!
     * \brief Used to ensure 32 bit enumeration across all cores and compilers\n
     * \note This is a placeholder enumeration, not a valid kernel
     */
    DVP_KN_MAXIMUM              = 0x7FFFFFFF,
} DVP_KernelNode_e;


/*! \brief Handle to the DVP instance.
 * \ingroup group_system
 * \see DVP_KernelGraph_Init
 */
typedef size_t DVP_Handle;

/*!
 * \brief This structure contains variables need to capture performance data.
 * \ingroup group_performance
 */
typedef struct _dvp_perf_t {
    DVP_U32 numTimes;     /*!<  The number of times that performance information has been captured. */
    rtime_t tmpTime;     /*!<  Used to record the last start time */
    rtime_t minTime;     /*!<  Used to record the minimum time of execution */
    rtime_t maxTime;     /*!<  Used to record the maximum time of execution */
    rtime_t avgTime;     /*!<  Used to record the average time of execution */
    rtime_t sumTime;     /*!<  Used to record the total time of execution of all iterations */
    rtime_t rate;        /*!<  This is used to record the clock rate per second on the local core */
} DVP_Perf_t;

/*!
 * Initialize the DVP_Perf_t structure. Used with automatic variables.
 * <pre>DVP_Perf_t perf = DVP_PERF_INIT;</pre>
 * \ingroup group_performance
 */
#define DVP_PERF_INIT   { 0, 0, MAX_RTIMER_T, 0, 0, 0, rtimer_freq() }

/*!
 * \brief This enumeration names the valid cores on OMAP 4,5,6 chips (not all are present or enabled on each generation).
 * \ingroup group_system
 */
typedef enum _dvp_cores_e {
    DVP_CORE_MIN = -1,
    DVP_CORE_DSP = 0,       /*!<  On OMAP4 this is the Tesla C64T, on OMAP 6 this is the C66x */
    DVP_CORE_SIMCOP,        /*!<  On OMAP4 this is the SIMCOP, controlled by the APP M3 Core */
    DVP_CORE_MCU0,          /*!<  On OMAP4 this is the APP M3 Core, on OMAP5, this is one of the M4 cores */
    DVP_CORE_MCU1,          /*!<  On OMAP4 this is the SYS M3 Core, on OMAP5, this is one of the M4 cores */
    DVP_CORE_GPU,           /*!<  On OMAP4, this is an SGX 540, in PC builds, this will be your local GPU, if present. */
    DVP_CORE_EVE,           /*!<  On OMAP6 there will be an EVE */
    // CPU must be last in the list
    DVP_CORE_CPU,           /*!<  On OMAP4, this is a Dual Cortex A9, in the PC build it will be your local CPU */
    DVP_CORE_MAX,
} DVP_Core_e;

/*! \brief Used to initialize an array of DVP_U32 values which correspond to the DVP_CORE_MAX number of cores.
 * \ingroup group_capacity
 */
#define DVP_CORE_LOAD_INIT      { 0, 0, 0, 0, 0, 0 }

/*! \brief Used to determine if a supplied value is within the valid range of DVP_Core_e.
 * \ingroup group_system
 */
#define DVP_VALID_CORE(x)       (DVP_CORE_MIN < (x) && (x) < DVP_CORE_MAX)

/*!
 * \brief This structure contains the metadata for each kernel node.
 * \ingroup group_nodes
 */
typedef struct _dvp_knode_hdr_t {
    DVP_ENUM   kernel;      /*!<  Specific kernel to execute for this node */
    DVP_S32    affinity;    /*!<  This indicates that the programmer knows which core this function should execute on. This will supercede priority and load balancing */
    DVP_Error_e error;      /*!<  Used to hold the error status of this node */
    DVP_S32    configured;  /*!<  \private This is hint which indicates to the Kernel Graph Boss that it if true has already determined which core to execute this node on. */
    DVP_U32    mgrIndex;    /*!<  \private This will be filled in after the nodes are given to the kernel graph */
    DVP_U32    funcIndex;   /*!<  \private This will be filled in after the nodes are given to the kernel graph */
    DVP_Perf_t perf;        /*!<  \private Used to capture the performance of each node */
    DVP_U32    resv[2];     /*!<  \private Used by M3 to store handle - Set to NULL only during creation */
} DVP_KernelNodeHeader_t;

/*! \brief Defines the size of the data region in a kernel node available to kernels for parameters.
 * \ingroup group_nodes
 */
#define DVP_KNODE_DATA_SIZE (DVP_PAGE_SIZE-sizeof(DVP_KernelNodeHeader_t))

/*!
 * \brief This structure specifies a single kernel function call and it's associated parameters plus overhead.
 * \ingroup group_nodes
 */
typedef struct _dvp_knode_t2 {
    DVP_KernelNodeHeader_t header;
    DVP_U08 data[DVP_KNODE_DATA_SIZE];  /*!< cast your data structure to this array. \see dvp_knode_to */
} DVP_KernelNode_t;

/*! \brief This macro is used to cast the data section to a user's requested data structure type.
 * \ingroup group_nodes
 */
#define dvp_knode_to(pNode, type)  ((type *)&((pNode)->data[0]))

/*!
 * \brief This structure allows a user to specify a series of nodes which are called a section.
 * \note This structure will currently stay on the HOST process space so no special allocator is required.
 * The structure may be declared as an automatic stack structure in your process.
 * \ingroup group_sections
 */
typedef struct _dvp_kernelgraphsection_t {
    DVP_KernelNode_t *pNodes;       /*!<  The array of node structures */
    DVP_U32           numNodes;     /*!<  The number of nodes in the array */
    DVP_Perf_t        perf;         /*!<  The performance information related to this section only */
    DVP_S32           coreLoad[DVP_CORE_MAX];     /*!<  Used internally to store the local calculation of load due to this section on each DVP CORE */
    DVP_BOOL          skipSection;     /*!<  A boolean to determine if the section should be skipped, defaults to false. */
} DVP_KernelGraphSection_t;

/*!
 * \brief This data structure allows a user to specify multiple sections into a larger graph structure.
 * The order field allows the users to specify which sections run in series or in parallel with other sections.
 * \note This structure will currently stay on the HOST process space so no special allocator is required.
 * The structure may be declared as an automatic stack structure in your process.
 * \ingroup group_graphs
 */
typedef struct _dvp_kernelgraph_t {
    DVP_KernelGraphSection_t *sections;     /*!< The array of sections of the graph */
    DVP_U32                   numSections;  /*!< This is the length of sections, and order, individually. */
    DVP_U32                  *order;        /*!< The array of order declarations */
    DVP_Perf_t                totalperf;    /*!< This is the total performance of the entire graph */
    DVP_BOOL                  verified;     /*!< This indicates that the graph has been verified. */
} DVP_KernelGraph_t;

/*! \brief The structure defines how a set of data is shifted as it moves through
 * a graph. The user must call \ref DVP_KernelGraph_ImageShiftAccum on each
 * node which they use to modifiy an image to accumulate the entire shift.
 * \ingroup group_images
 */
typedef struct _dvp_image_shift_t {
    DVP_S32 centerShiftHorz;    /*!< Indicates the horizontal shift on the center of the image */
    DVP_S32 centerShiftVert;    /*!< Indicates the vertical shift on the center of the image */
    DVP_S32 topBorder;          /*!< Indicates the change in the top border */
    DVP_S32 rightBorder;        /*!< Indicates the change in the right border */
    DVP_S32 bottomBorder;       /*!< Indicates the change in the bottom border */
    DVP_S32 leftBorder;         /*!< Indicates the change in the left border */
} dvp_image_shift_t;

/*! \brief The maximum kernel name length.
 * \ingroup group_kernels
 */
#define DVP_KERNEL_MAX (128)

/*! \brief A typedef of the function pointer to an image shift calculator
 * \param [in] node The pointer to the node to use as an input to the calculator.
 * \param [in] shift The image shift accumulator structure.
 */
typedef void (*dvp_image_shift_f)(DVP_KernelNode_t *node, dvp_image_shift_t *shift);

/*!
 * \brief Defines which kernels a manager supports.
 * This defines the entry for the kernel table each KGM must define in order to inform the
 * the Kernel Graph Boss which kernel it supports.
 * \ingroup group_kernels
 */
typedef struct _dvp_core_function_t {
    char                        name[DVP_KERNEL_MAX];   /*!< The name of the kernel, used in debugging */
    DVP_U32                     kernel;                 /*!< The enumerated name of the kernel. Use a \ref DVP_KernelNode_e value.  */
    DVP_U32                     load;                   /*!< The load value of this kernel */
    dvp_image_shift_t          *shift;                  /*!< The pointer, if needed, to use to calculate image shifts for this kernel */
    dvp_image_shift_f           shift_func;             /*!< The pointer to a function to compute the image shift. */
} DVP_CoreFunction_t;

/*!
 * \brief This data structure allows a user to query the status of the core.
 * \ingroup group_system
 */
typedef struct _dvp_coreinfo_t
{
    DVP_BOOL enabled;                       /*!< The core state */
    DVP_BOOL libraries[DVP_KL_LIBRARY_MAX]; /*!< The list of kernel libraries enabled on this core. Use \ref DVP_KernelLibrary_e to index this array to get the boolean. */
    const DVP_CoreFunction_t *kernels;      /*!< The list of kernel libraries enabled on this core */
    DVP_U32 numKernels;                     /*!< The number of kernerls enabled. */
} DVP_CoreInfo_t;

#endif // _DVP_TYPES_H_