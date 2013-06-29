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

#include <sosal/serial.h>
#include <sosal/debug.h>

#if defined(SOSAL_SERIAL_API)

serial_t serial_open(uint32_t number)
{
    char device_name[256];
#ifdef POSIX_SERIAL
    FILE *ftty;
    sprintf(device_name, "/dev/ttyS%02u", number);
    ftty = fopen(device_name, "wb");
    return ftty;
#else
    HANDLE hCom;
    DCB dcb;

    sprintf(device_name, "\\\\.\\COM%u", number);

    hCom = CreateFile(device_name, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hCom)
    {
        //SecureZeroMemory(&dcb, sizeof(DCB));
        memset(&dcb, 0, sizeof(dcb));
        dcb.DCBlength = sizeof(DCB);

        GetCommState(hCom, &dcb);
        dcb.BaudRate = CBR_115200;
        dcb.ByteSize = 8;
        dcb.Parity   = NOPARITY;
        dcb.StopBits = ONESTOPBIT;
        // 115200 8n1
        SetCommState(hCom, &dcb);
        GetCommState(hCom, &dcb);
        if (dcb.BaudRate == 0) {
            CloseHandle(hCom);
            hCom = NULL;
        } else {
            printf("%s: BaudRate = %d, ByteSize = %d, Parity = %d, StopBits = %d\n", device_name, dcb.BaudRate, dcb.ByteSize, dcb.Parity, dcb.StopBits);
        }
    }
    return hCom;
#endif
}

void serial_close(serial_t ser)
{
#ifdef POSIX_SERIAL
    fclose(ser);
#else
    CloseHandle(ser);
#endif
}

size_t serial_read(serial_t ser, uint8_t *buffer, size_t numBytes)
{
#ifdef POSIX_SERIAL
    return fread(buffer, 1, numBytes, ser);
#else
    size_t read;
    if (ReadFile(ser, buffer, numBytes, (DWORD *)&read, NULL) == FALSE)
        read = 0;
    return read;
#endif
}

size_t serial_write(serial_t ser, uint8_t *buffer, size_t numBytes)
{
#ifdef POSIX_SERIAL
    return fwrite(buffer, 1, numBytes, ser);
#else
    size_t wrote;
    if (WriteFile(ser, buffer, numBytes, (DWORD *)&wrote, NULL) == FALSE)
        wrote = 0;
    return wrote;
#endif
}

#endif

