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

#ifndef _SOSAL_SERIAL_H_
#define _SOSAL_SERIAL_H_

/*! \file
 * \brief The SOSAL Serial API.
 * \author Erik Rainey <erik.rainey@ti.com>
 */
 
#include <sosal/types.h>

#if defined(ANDROID) || defined(LINUX) || defined(DARWIN) || defined(__QNX__)
#define POSIX_SERIAL
#define SOSAL_SERIAL_API
#elif defined(CYGWIN) || defined(WIN32)
#include <windows.h>
#undef POSIX_SERIAL
#define SOSAL_SERIAL_API
#elif defined(SYSBIOS)
#undef POSIX_SERIAL
#undef SOSAL_SERIAL_API
#endif

#ifdef POSIX_SERIAL
/*! \brief On POSIX systems this wraps a FILE pointer. 
 * \ingroup group_serial
 */
typedef FILE* serial_t;
#elif defined(WIN32)
/*! \brief On Windows systems this wraps a HANDLE.
 * \ingroup group_serial
 */
typedef HANDLE serial_t;
#endif

#if defined(SOSAL_SERIAL_API)

#ifdef __cplusplus
extern "C" {
#endif

/*! \brief This function opens a serial port.
 * \param [in] number The port number.
 * \return Returns the handle to the serial port.
 * \retval 0 if the port could not be opened.
 * \post \ref serial_close
 * \ingroup group_serial
 */
serial_t serial_open(uint32_t number);

/*! \brief This function closes a serial port.
 * \param [in] ser The serial port handle.
 * \pre \ref serial_open
 * \ingroup group_serial
 */
void serial_close(serial_t ser);

/*! \brief This function reads data from a serial port. 
 * \param [in] ser The serial port handle.
 * \param [in] buffer The buffer to read the data into.
 * \param [in] numBytes The number of bytes to read into buffer.
 * \return Returns the number of bytes read from the serial port. 
 * \retval 0 No data was read.
 * \pre \ref serial_open
 * \post \ref serial_close
 * \ingroup group_serial
 */
size_t serial_read(serial_t ser, uint8_t *buffer, size_t numBytes);

/*! \brief This function writes data to a serial port. 
 * \param [in] ser The serial port handle.
 * \param [in] buffer The buffer to write the data from.
 * \param [in] numBytes The number of bytes to write from buffer.
 * \return Returns the number of bytes written to the serial port. 
 * \retval 0 No data was written.
 * \pre \ref serial_open
 * \post \ref serial_close
 * \ingroup group_serial
 */
size_t serial_write(serial_t ser, uint8_t *buffer, size_t numBytes);

#ifdef __cplusplus
}
#endif

#endif // SOSAL_SERIAL_API

#endif // FILE lock

