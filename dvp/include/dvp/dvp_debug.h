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

#ifndef _DVP_DEBUG_H_
#define _DVP_DEBUG_H_

/*!
 * \file dvp_debug.h
 * \brief Contains the debugging definitions used by internal development.
 * \author Erik Rainey
 */

#ifdef DVP_DEBUG
#include <stdio.h>
#if defined(ANDROID)
#include <utils/Log.h>
#endif

//#pragma message("DVP_DEBUG: Debugging Enabled!")

/*! \brief A generic assert wrapper.
 * \ingroup group_debugging
 */
#define DVP_ASSERT(conditional) if (!(conditional)) {\
    printf("DVP_ASSERT in %s@%u: %s\n", __FILE__, __LINE__, #conditional);\
}

#if defined(WIN32) && !defined(CYGWIN)
#include <windows.h>
#ifdef DVP_CONSOLE
/*! \brief An inline function used to get around MS compiler limitations on variable
 * arguments to macros.
 * \ingroup group_debugging
 */
_inline void DVP_PRINT(int conditional, char *format, ...) {
    if (conditional)
    {
        char string[1024];
        va_list ap;
        va_start(ap, format);
        vsprintf(string, format, ap);
        va_end(ap);
        WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), string, strlen(string), NULL, NULL);
    }
}
#else
/*! \brief An inline function used to get around MS compiler limitations on variable
 * arguments to macros.
 * \ingroup group_debugging
 */
_inline void DVP_PRINT(int conditional, char *string, ...) {
    if (conditional)
    {
        va_list ap;
        va_start(ap, string);
        vprintf(string, ap);
        va_end(ap);
    }
}
#endif // DVP_CONSOLE
#elif defined(SYSBIOS)
/*! \brief An inline function which allows for variable arguments to the printing function
 * the TMS470 Compiler which does not allow variable arguments in macros.
 * \param [in] conditional Typically a DVP Zone flag.
 * \param [in] string The format string to print.
 * \param [in] ... The variable arguments used to fill in the format string.
 * \ingroup group_sosal_debug
 */
inline void DVP_PRINT(int conditional, char *string, ...) {
	if (conditional)
	{
		va_list ap;
        va_start(ap, string);
        vprintf(string, ap);
        va_end(ap);
	}
}
#else
#if (DVP_DEBUG == 2) && defined(ANDROID)
#include <android/log.h>
/*! \brief A wrapper around a platform specific debugging API.
 * \param [in] conditional Typically a DVP_ZONE_XXXX defintion.
 * \param [in] string A format string
 * \param [in] ... Variable arguments to match the format string.
 * \ingroup group_debugging
 */
#define DVP_PRINT(conditional, string, ...) if (conditional) { __android_log_print(ANDROID_LOG_DEBUG, #conditional, string, ## __VA_ARGS__); }
#else
/*! \brief A wrapper around a platform specific debugging API.
 * \param [in] conditional Typically a DVP_ZONE_XXXX defintion.
 * \param [in] string A format string
 * \param [in] ... Variable arguments to match the format string.
 * \ingroup group_debugging
 */
#define DVP_PRINT(conditional, string, ...) if (conditional) { printf(#conditional ": " string, ## __VA_ARGS__); }
#endif
#endif

#ifndef DVP_ZONE_MASK
#define DVP_RUNTIME_DEBUG

/*! \brief The runtime zone mask for DVP.
 * \ingroup group_debugging
 */
extern uint32_t dvp_zone_mask;

/*! The definition of which zones are enabled. Each bit in the mask is an
 * individual zone as specified by the zone definitions.
 * \ingroup group_debugging
 */
#define DVP_ZONE_MASK       dvp_zone_mask
#endif

#else  // ifdef DVP_DEBUG

#ifndef DVP_ZONE_MASK
/*! \brief The definition of which zones are enabled. Each bit in the mask is an
 * individual zone as specified by the zone definitions.
 * \ingroup group_debugging
 */
#define DVP_ZONE_MASK       (0x00000000)
#endif

//#pragma message("DVP_DEBUG: No debugging enabled!")

/*! \brief A wrapper around a platform specific debugging API.
 * \param conditional Typically a DVP_ZONE_XXXX defintion.
 * \param string A format string
 * \param ... Variable arguments to match the format string.
 * \ingroup group_debugging
 */
#define DVP_PRINT(conditional, string, ...)  {}

#endif // ifdef DVP_DEBUG

#define DVP_BIT(x)          (1 << (x))
#define DVP_ZONE_ERROR      (DVP_BIT(0) & DVP_ZONE_MASK) /*!< Intended for error cases in all the code. \ingroup group_debugging */
#define DVP_ZONE_WARNING    (DVP_BIT(1) & DVP_ZONE_MASK) /*!< Intended for warning in any code. \ingroup group_debugging  */
#define DVP_ZONE_API        (DVP_BIT(2) & DVP_ZONE_MASK) /*!< Intended for API tracing in any code. \ingroup group_debugging  */
#define DVP_ZONE_MOD        (DVP_BIT(3) & DVP_ZONE_MASK) /*!< Intended for informational purposes in module loading code only. \ingroup group_debugging  */

#define DVP_ZONE_RPC        (DVP_BIT(4) & DVP_ZONE_MASK) /*!< Intended for informational purposes in RPC code only. \ingroup group_debugging  */
#define DVP_ZONE_MEM        (DVP_BIT(5) & DVP_ZONE_MASK) /*!< Intended for informational purposes in MEM code only. \ingroup group_debugging  */
#define DVP_ZONE_KGM        (DVP_BIT(6) & DVP_ZONE_MASK) /*!< Intended for informational purposes in any KGM code only. \ingroup group_debugging  */
#define DVP_ZONE_KGB        (DVP_BIT(7) & DVP_ZONE_MASK) /*!< Intended for informational purposes in the KGB code only. \ingroup group_debugging  */

#define DVP_ZONE_LOAD       (DVP_BIT(8) & DVP_ZONE_MASK) /*!< Intended for load balanacing information. \ingroup group_debugging  */
#define DVP_ZONE_PERF       (DVP_BIT(9) & DVP_ZONE_MASK) /*!< Intended for performance metrics output only. \ingroup group_debugging  */
#define DVP_ZONE_CAM        (DVP_BIT(10)& DVP_ZONE_MASK) /*!< Intended for debugging the VisionCam. \ingroup group_debugging  */
#define DVP_ZONE_IMGDBG     (DVP_BIT(11)& DVP_ZONE_MASK) /*!< Intended for debugging the Image Debug Library. \ingroup group_debugging  */

#define DVP_ZONE_VIDEO      (DVP_BIT(12)& DVP_ZONE_MASK) /*!< Intended for debugging the display code. \ingroup group_debugging  */
#define DVP_ZONE_KGAPI      (DVP_BIT(13)& DVP_ZONE_MASK) /*!< Intended for debugging the multiple graph calling code in the API. \ingroup group_debugging  */
#define DVP_ZONE_ENGINE     (DVP_BIT(14)& DVP_ZONE_MASK) /*!< Intended for debugging the VisionEngine. \ingroup group_debugging  */

#define DVP_ZONE_ALWAYS     (1)

/*! A generic error checking macro which can take a parameterized action
 * \param [out] lvalue Typically a status variable.
 * \param [in] statement Typically a statement which returns a status variable.
 * \param [in] action A block of code to execute when lvalue is a failure code.
 * \ingroup group_debugging
 */
#define DVP_DO_IF_FAILED(lvalue, statement, action) {\
    lvalue = statement;\
    if (DVP_FAILED(lvalue)) {\
        DVP_PRINT(DVP_ZONE_ERROR, "ERROR: "#statement" returned error %d! (%s:%u)!\n", lvalue, __FUNCTION__,__LINE__);\
        action;\
    }\
}

/*! A generic waring macro when the statement is false.
 * \param [in] statement Typically a statement which returns a status variable.
 * \ingroup group_debugging
 */
#define DVP_COMPLAIN_IF_FALSE(statement)  {\
    if (DVP_FALSE == statement) {\
        DVP_PRINT(DVP_ZONE_WARNING, "WARNING: "#statement" is false! (%s:%u)!\n", __FUNCTION__,__LINE__);\
    }\
}

/*! Macro used to check the statement return value and if failed, return immediately. @note The function should return the DVP_Error_e type. \ingroup group_debugging*/
#define DVP_RETURN_IF_FAILED(lvalue, statement)          DVP_DO_IF_FAILED(lvalue, statement, return lvalue);
/*! Macro used to check the statement return value and if failed, break immediately. @note Should only be used in a for or while construct. \ingroup group_debugging */
#define DVP_BREAK_IF_FAILED(lvalue, statement)           DVP_DO_IF_FAILED(lvalue, statement, break);
/*! Macro used to check the statement return value and if failed, continue immediately. @note Should only be used in a for or while construct. \ingroup group_debugging*/
#define DVP_CONTINUE_IF_FAILED(lvalue, statement)        DVP_DO_IF_FAILED(lvalue, statement, continue);
/*! Macro used to check the statement return value and if failed, goto a supplied label immediately. @note The label must exist in the function. \ingroup group_debugging */
#define DVP_GOTO_IF_FAILED(lvalue, statement, label)     DVP_DO_IF_FAILED(lvalue, statement, goto label);

#endif
