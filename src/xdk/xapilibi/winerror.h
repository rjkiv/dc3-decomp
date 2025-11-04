#pragma once

// holy moly there are a LOT of possible errors in Windows
// https://learn.microsoft.com/en-us/windows/win32/debug/system-error-codes

// clang-format off
#define ERROR_SUCCESS                   0x0000
#define ERROR_INVALID_FUNCTION          0x0001
#define ERROR_FILE_NOT_FOUND            0x0002
#define ERROR_PATH_NOT_FOUND            0x0003
#define ERROR_INVALID_NAME              0x007B
#define ERROR_IO_INCOMPLETE             0x03E4
#define ERROR_IO_PENDING                0x03E5
#define ERROR_FILE_CORRUPT              0x0570
#define ERROR_DISK_CORRUPT              0x0571
// clang-format on
