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


#ifndef DVP_UNITTEST_DATA_H
#define DVP_UNITTEST_DATA_H

#include <sosal/fourcc.h>

typedef struct _color_conversion_t {
    enum _fourcc from;
    enum _fourcc to;
} color_conversion_t;

static color_conversion_t img_copy_conversions[] = {
//  {       from    ,       to      }
    {   FOURCC_Y800 , FOURCC_UYVY   },
    {   FOURCC_Y800 , FOURCC_VYUY   },
    {   FOURCC_Y800 , FOURCC_YUY2   },
    {   FOURCC_Y800 , FOURCC_YVYU   },

    {   FOURCC_YUY2 , FOURCC_UYVY   },
    {   FOURCC_YUY2 , FOURCC_VYUY   },
    {   FOURCC_YVYU , FOURCC_UYVY   },
    {   FOURCC_YVYU , FOURCC_VYUY   },

    {   FOURCC_NV12 , FOURCC_Y800   },
    {   FOURCC_YU24 , FOURCC_Y800   },
    {   FOURCC_YV24 , FOURCC_Y800   },
    {   FOURCC_YU16 , FOURCC_Y800   },
    {   FOURCC_YV16 , FOURCC_Y800   },
    {   FOURCC_IYUV , FOURCC_Y800   },
    {   FOURCC_YV12 , FOURCC_Y800   },
    {   FOURCC_YUV9 , FOURCC_Y800   },
    {   FOURCC_YVU9 , FOURCC_Y800   }
};

typedef struct color_format_name_table_t {
    enum _fourcc colorID;
    const char colorName[16];
} color_format_name_table_t;

color_format_name_table_t color_name_table[] = {
// YUV
    // interleave
    {   FOURCC_UYVY, "FOURCC_UYVY"      },
    {   FOURCC_YUY2, "FOURCC_YUY2"      },
    {   FOURCC_YVYU, "FOURCC_YVYU"      },
    {   FOURCC_VYUY, "FOURCC_VYUY"      },

    // planar
    {   FOURCC_YUV9, "FOURCC_YUV9"      },
    {   FOURCC_YVU9, "FOURCC_YVU9"      },
    {   FOURCC_YV12, "FOURCC_YV12"      },
    {   FOURCC_IYUV, "FOURCC_IYUV"      },
    {   FOURCC_YV16, "FOURCC_YV16"      },
    {   FOURCC_YU16, "FOURCC_YU16"      },
    {   FOURCC_YV24, "FOURCC_YV24"      },
    {   FOURCC_YU24, "FOURCC_YU24"      },
    {   FOURCC_Y800, "FOURCC_Y800"      },
    {   FOURCC_Y16 , "FOURCC_Y16 "      },
    {   FOURCC_Y32 , "FOURCC_Y32 "      },

    // semi-planar
    {   FOURCC_NV12, "FOURCC_NV12"      },
    {   FOURCC_NV21, "FOURCC_NV21"      },

// RGB
    {   FOURCC_RGB565, "FOURCC_RGB565"  },
    {   FOURCC_BGR565, "FOURCC_BGR565"  },

    // interleave
    {   FOURCC_BGR , "FOURCC_BGR "      },
    {   FOURCC_RGB , "FOURCC_RGB "      },
    {   FOURCC_RGBA, "FOURCC_RGBA"      },
    {   FOURCC_BGRA, "FOURCC_BGRA"      },
    {   FOURCC_ARGB, "FOURCC_ARGB"      },

    // planar
    {   FOURCC_RGBP, "FOURCC_RGBP"      },

// HSL
    // planar     // planar
    {   FOURCC_HSLP, "FOURCC_HSLP"      },

// BINARY
    {   FOURCC_BIN1, "FOURCC_BIN1"      }

};


#endif // DVP_UNITTEST_DATA_H
