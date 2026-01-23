#pragma once
#include "obj/Data.h"
#include "os/NetworkSocket.h"
#include "os/File.h"
#include "types.h"

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
        case 0:
            return "kVersion";
        case 1:
            return "kSysExec";
        case 2:
            return "kGetStat";
        case 3:
            return "kOpenFile";
        case 4:
            return "kWriteFile";
        case 5:
            return "kReadFile";
        case 6:
            return "kCloseFile";
        case 7:
            return "kPrint";
        case 8:
            return "kMkDir";
        case 9:
            return "kDelete";
        case 10:
            return "kEnumerate";
        case 11:
            return "kCacheFile";
        case 12:
            return "kCompareFileTimes";
        case 13:
            return "kTerminate";
        case 14:
            return "kCacheResource";
        case 15:
            return "kPollKeyboard";
        case 16:
            return "kPollJoypad";
        case 17:
            return "kStackTrace";
        case 18:
            return "kSendMessage";
        case 19:
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

// HolmesClient_NetSocket

namespace HolmesClient {
    String PlatformGetHostName();
    NetAddress PlatformResolveIP();
    BinStream *PlatformCreateServerStream(bool, const char *);
}
