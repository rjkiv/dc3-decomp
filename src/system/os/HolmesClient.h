#pragma once
#include "obj/Data.h"
#include "obj/Msg.h"
#include "os/NetworkSocket.h"
#include "os/File.h"
#include "types.h"
#include "utl/Cache.h"

// what's an `enum class`?
namespace Holmes {
    enum Protocol {
        kVersion, // 0x0
        kSysExec, // 0x1
        kGetStat, // 0x2
        kOpenFile, // 0x3
        kWriteFile, // 0x4
        kReadFile, // 0x5
        kCloseFile, // 0x6
        kPrint, // 0x7
        kMkDir, // 0x8
        kDelete, // 0x9
        kEnumerate, // 0xa
        kCacheFile, // 0xb
        kCompareFileTimes, // 0xc
        kTerminate, // 0xd
        kCacheResource, // 0xe
        kPollKeyboard, // 0xf
        kPollJoypad, // 0x10
        kStackTrace, // 0x11
        kSendMessage, // 0x12
        kTruncateFile, // 0x13
        kInvalidOpcode, // 0x14
    };

    inline const char *ProtocolDebugString(u8 c) {
        switch (c) {
        case kVersion:
            return "kVersion";
        case kSysExec:
            return "kSysExec";
        case kGetStat:
            return "kGetStat";
        case kOpenFile:
            return "kOpenFile";
        case kWriteFile:
            return "kWriteFile";
        case kReadFile:
            return "kReadFile";
        case kCloseFile:
            return "kCloseFile";
        case kPrint:
            return "kPrint";
        case kMkDir:
            return "kMkDir";
        case kDelete:
            return "kDelete";
        case kEnumerate:
            return "kEnumerate";
        case kCacheFile:
            return "kCacheFile";
        case kCompareFileTimes:
            return "kCompareFileTimes";
        case kTerminate:
            return "kTerminate";
        case kCacheResource:
            return "kCacheResource";
        case kPollKeyboard:
            return "kPollKeyboard";
        case kPollJoypad:
            return "kPollJoypad";
        case kStackTrace:
            return "kStackTrace";
        case kSendMessage:
            return "kSendMessage";
        case kTruncateFile:
            return "kTruncateFile";
        default:
            return "Unknown";
        }
    }
}

bool UsingHolmes(int iMode);
int HolmesClientSysExec(const char *cmdLine);
void HolmesClientReInit();
void HolmesClientPrint(const char *s);
void HolmesClientTerminate();
void HolmesClientInit();
NetAddress HolmesResolveIP();
void HolmesClientPollKeyboard();
unsigned int HolmesClientPollJoypad();
int HolmesClientGetStat(const char *filename, FileStat &stat);
int HolmesClientDelete(const char *path);
int HolmesClientMkDir(const char *path);
const char *HolmesFileShare();
void HolmesSetFileShare(const char *machineName, const char *shareName);
void HolmesClientEnumerate(
    const char *dir,
    void (*cb)(const char *, const char *),
    bool recurse,
    const char *pattern,
    bool dirs
);
void HolmesClientStackTrace(
    const char *file, struct StackData *stack, int stack_size, String &stack_desc
);
void HolmesClientTruncate(int file_handle, int length);
bool HolmesClientOpen(
    const char *filename, int mode, unsigned int &file_size, int &file_handle
);
void HolmesClientWrite(int file_handle, int start, int bytes, const void *buf);
void HolmesClientRead(int file_handle, int start, int bytes, void *buf, File *requestor);
bool HolmesClientReadDone(File *f);
void HolmesClientClose(File *f, int file_handle);
bool CanUseHolmes(int iMode);
void HolmesToLocal(char *out, const char *file);
const char *HolmesFileHostName();
void HolmesClientPoll();
DataNode DumpHolmesLog(DataArray *array);
void HolmesClientSendMessage(const Message &m);
CacheResourceResult
HolmesClientCacheResource(const char *filename, const char *cachedName);
bool HolmesClientCacheFile(char *localName, const char *filename);

// HolmesClient_NetSocket

namespace HolmesClient {
    String PlatformGetHostName();
    NetAddress PlatformResolveIP();
    BinStream *PlatformCreateServerStream(bool iHolmesOptional, const char *iHolmesShare);
}

struct RecurseInfo {
    String mDir;
    String mFile;
};
