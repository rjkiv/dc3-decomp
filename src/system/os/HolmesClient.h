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

bool UsingHolmes(int);
int HolmesClientSysExec(const char *);
void HolmesClientReInit();
void HolmesClientPrint(const char *);
void HolmesClientTerminate();
void HolmesClientInit();
NetAddress HolmesResolveIP();
void HolmesClientPollKeyboard();
unsigned int HolmesClientPollJoypad();
int HolmesClientGetStat(const char *, FileStat &);
int HolmesClientDelete(const char *);
int HolmesClientMkDir(const char *);
const char *HolmesFileShare();
void HolmesSetFileShare(const char *, const char *);
void HolmesClientEnumerate(
    const char *, void (*)(const char *, const char *), bool, const char *, bool
);
void HolmesClientStackTrace(const char *, struct StackData *, int, String &);
void HolmesClientTruncate(int, int);
bool HolmesClientOpen(const char *, int, unsigned int &, int &);
void HolmesClientWrite(int, int, int, const void *);
void HolmesClientRead(int, int, int, void *, File *);
bool HolmesClientReadDone(File *);
void HolmesClientClose(File *, int);
bool CanUseHolmes(int);
void HolmesToLocal(char *, const char *);
char const *HolmesFileHostName();
void HolmesClientPoll();
DataNode DumpHolmesLog(DataArray *);
void HolmesClientSendMessage(const Message &);
CacheResourceResult HolmesClientCacheResource(const char *, const char *);
bool HolmesClientCacheFile(char *, const char *);

// HolmesClient_NetSocket

namespace HolmesClient {
    String PlatformGetHostName();
    NetAddress PlatformResolveIP();
    BinStream *PlatformCreateServerStream(bool, const char *);
}

struct RecurseInfo {
    String s1;
    String s2;
};
