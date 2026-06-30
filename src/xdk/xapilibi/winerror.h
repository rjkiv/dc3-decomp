#pragma once

// holy moly there are a LOT of possible errors in Windows
// https://learn.microsoft.com/en-us/windows/win32/debug/system-error-codes

// clang-format off
#define ERROR_SUCCESS                   0x0000
#define ERROR_INVALID_FUNCTION          0x0001
#define ERROR_FILE_NOT_FOUND            0x0002
#define ERROR_PATH_NOT_FOUND            0x0003
#define ERROR_ACCESS_DENIED             0x0005
#define ERROR_OUTOFMEMORY               0x000e
#define ERROR_NO_MORE_FILES             0x0012
#define ERROR_NOT_READY                 0x0015
#define ERROR_TOO_MANY_CMDS             0x0038
#define ERROR_DISK_FULL                 0x0070
#define ERROR_INVALID_NAME              0x007B
#define ERROR_BUSY                      0x00AA
#define ERROR_ALREADY_EXISTS            0x00B7
#define ERROR_IO_INCOMPLETE             0x03E4
#define ERROR_IO_PENDING                0x03E5
#define ERROR_MEDIA_CHANGED             0x0456
#define ERROR_DEVICE_NOT_CONNECTED      0x048F
#define ERROR_CANCELLED                 0x04C7
#define ERROR_RETRY                     0x04D5
#define ERROR_NO_SUCH_USER              0x0525       
#define ERROR_FILE_CORRUPT              0x0570
#define ERROR_DISK_CORRUPT              0x0571
#define ERROR_DEVICE_REMOVED            0x0651

#define S_OK                            (long)0

#define E_NOINTERFACE                   (long)0x80004002
#define E_SPEECH_UNINITIALIZED          (long)0x80045001

#define E_OUTOFMEMORY                   (long)0x8007000E
#define E_NUI_DEVICE_NOT_READY          (long)0x80070015
#define E_INVALIDARG                    (long)0x80070057

#define E_NUI_DEVICE_NOT_CONNECTED      (long)0x8007048F
#define E_NUI_ALREADY_INITIALIZED       (long)0x800704DF
#define E_NUI_IMAGE_STREAM_IN_USE       (long)0x83010003
#define E_NUI_FEATURE_NOT_INITIALIZED   (long)0x83010005
#define E_NUI_SYSTEM_UI_PRESENT         (long)0x8301000B
#define E_NUI_DATABASE_NOT_FOUND        (long)0x8301000D
#define E_NUI_DATABASE_VERSION_MISMATCH (long)0x8301000E
// clang-format on
