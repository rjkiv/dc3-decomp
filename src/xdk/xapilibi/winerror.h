#pragma once

// holy moly there are a LOT of possible errors in Windows
// https://learn.microsoft.com/en-us/windows/win32/debug/system-error-codes

// clang-format off
#define ERROR_SUCCESS                   0x0000
#define ERROR_INVALID_FUNCTION          0x0001
#define ERROR_FILE_NOT_FOUND            0x0002
#define ERROR_PATH_NOT_FOUND            0x0003
#define ERROR_OUTOFMEMORY               0x000e
#define ERROR_NO_MORE_FILES             0x0012
#define ERROR_NOT_READY                 0x0015
#define ERROR_DISK_FULL                 0x0070
#define ERROR_INVALID_NAME              0x007B
#define ERROR_ALREADY_EXISTS            0x00B7
#define ERROR_IO_INCOMPLETE             0x03E4
#define ERROR_IO_PENDING                0x03E5
#define ERROR_MEDIA_CHANGED             0x0456
#define ERROR_DEVICE_NOT_CONNECTED      0x048F
#define ERROR_NO_SUCH_USER              0x0525       
#define ERROR_FILE_CORRUPT              0x0570
#define ERROR_DISK_CORRUPT              0x0571
#define ERROR_DEVICE_REMOVED            0x0651

#define E_OUTOFMEMORY                   0x8007000E
// clang-format on
