#include "HolmesClient.h"
#include "obj/Data.h"
#include "obj/DataFunc.h"
#include "obj/Msg.h"
#include "os/CritSec.h"
#include "os/Debug.h"
#include "os/File.h"
#include "os/HolmesUtl.h"
#include "os/NetworkSocket.h"
#include "os/System.h"
#include "os/Timer.h"
#include "utl/Loader.h"
#include "utl/MemStream.h"
#include "utl/Option.h"
#include "utl/Symbol.h"
#include "utl/TextFileStream.h"
#include <cstdio>
#include <list>

#pragma region Statics

#define HOLMES_CURRENT_VERSION 26
#define NETBIOS_NAME_MAX 64

String gLastCachedResource;

namespace {
    struct HolmesProfileData {
        Timer wait;
        Timer work;
        int count;
        u32 pad;
    };

    struct ReadRequest {
        // FILE *mRequestor;
        void *mBuffer;
        int mBytes;
    };

    BinStream *gHolmesStream;
    MemStream *gStreamBuffer;

    char gMachineName[NETBIOS_NAME_MAX] = { 0 };
    char gShareName[NETBIOS_NAME_MAX] = { 0 };
    bool gStackTraced;

    Holmes::Protocol gPendingResponse;
    int gRealMaxBufferSize;
    HolmesProfileData gProfile[20]; // to match protocol count
    CriticalSection gCrit;
    std::list<ReadRequest> gRequests;
    String gServerName;
    // HolmesInput gInput; // fuck you mfc
    String gHolmesTarget;
    bool gPollStreamEof;

#pragma region Private details

    void BeginCmd(Holmes::Protocol prot, bool b) {
        if (b) {
            gProfile[prot].count += 1;
        }
        gProfile[prot].work.Start();
    }

    void EndCmd(Holmes::Protocol prot) {
        gProfile[prot].work.Stop();
        if (gRealMaxBufferSize != 0) {
            MILO_NOTIFY_ONCE(
                "HolmesClient buffer exceeded %d < %d", 0x2000d, gRealMaxBufferSize
            );
        }
    }

    void HolmesFlushStreamBuffer();
    void WaitForAnyResponse(Holmes::Protocol prot);
    void WaitForResponse(Holmes::Protocol prot);
    bool CheckForResponse(Holmes::Protocol prot, bool b);
    // this should hopefully be correct when someone does HolmesInput
    void CheckInput(bool b) {
        if (CheckForResponse(gPendingResponse, b)) {
            BeginCmd(Holmes::kPollKeyboard, true);
            // gInput.LoadKeyboard(gHolmesStream);
            gPendingResponse = Holmes::kInvalidOpcode;
            EndCmd(Holmes::kPollKeyboard);
        }

        if (CheckForResponse(gPendingResponse, b)) {
            BeginCmd(Holmes::kPollJoypad, true);
            // gInput.LoadJoypad(gHolmesStream);
            gPendingResponse = Holmes::kInvalidOpcode;
            EndCmd(Holmes::kPollJoypad);
        }
    };
    bool CheckReads(bool b);
    void HolmesClientPollInternal(bool b) {
        CritSecTracker cst(&gCrit);

        if (!gHolmesStream)
            return;

        CheckInput(b);
        CheckReads(b);
    };
}

#pragma region Public API

bool UsingHolmes(int p1) {
    if (!gHolmesStream)
        return false;

    return CanUseHolmes(p1);
}

NetAddress HolmesResolveIP() {
    if (CanUseHolmes(3))
        return HolmesClient::PlatformResolveIP();
    else
        return NetAddress();
}

void HolmesClientPollKeyboard() { return; }

DataNode DumpHolmesLog(DataArray *) {
    TextFileStream *log = new TextFileStream("holmes.csv", true);
    FileStream &fs = log->File();
    if (!fs.Fail()) {
        *log << HolmesClient::PlatformGetHostName() << ", ";
        *log << -1 << ", ";
        *log << -1 << "\n";
        for (int i = 0; i < 20; i++) {
            int count = gProfile[i].count;
            float wait = gProfile[i].wait.SplitMs();
            float work = gProfile[i].work.SplitMs() - wait;
            *log << Holmes::ProtocolDebugString(i) << ", ";
            *log << count << ", ";
            *log << wait << ", ";
            *log << work << ", ";
        }
        fs.Flush();
    }
    delete log;
    return 0;
}

bool HolmesClientInitOpcode(bool quiet) {
    bool fail = 0;
    *gStreamBuffer << u8(Holmes::kVersion) << HOLMES_CURRENT_VERSION;
    *gStreamBuffer << HolmesClient::PlatformGetHostName();
    *gStreamBuffer << gHolmesTarget;
    *gStreamBuffer << &gMachineName[0x40];
    *gStreamBuffer << FileSystemRoot();
    *gStreamBuffer << u8(TheLoadMgr.GetPlatform());
    *gStreamBuffer << u8(GetGfxMode());
    HolmesFlushStreamBuffer();
    if (!quiet) {
        WaitForAnyResponse(Holmes::kVersion);
        u8 response;
        *gHolmesStream >> response;
        fail = response != 0;
    } else {
        WaitForAnyResponse(Holmes::kVersion);
    }
    s32 host_ver = -1;
    if (!fail) {
        *gHolmesStream >> host_ver;
        fail = host_ver != HOLMES_CURRENT_VERSION;
    }
    if (fail) { // host/client version mismatch
        RELEASE(gHolmesStream);
        RELEASE(gStreamBuffer);
        if (gHostLogging) {
            gPendingResponse = Holmes::kInvalidOpcode;
            return fail;
        }
        if (host_ver >= 0) {
            MILO_FAIL(
                "Holmes version mismatch\nResync/rebuild both projects\nHolmes=%d  Console=%d",
                host_ver,
                HOLMES_CURRENT_VERSION
            );
        } else {
            MILO_FAIL("Holmes protocol mismatch\nCould not connect to console");
        }
    }
    if (!fail) {
        *gHolmesStream >> gServerName;
    }
    if (gHolmesTarget.c_str()[0] != 0) {
        bool b;
        *gHolmesStream >> b;
        if (b == 0) {
            MILO_FAIL("Failed to find holmes target '%s'", gHolmesTarget);
        }
    }
    if (!fail && gMachineName[0x40] == 0) {
        String my_name(gMachineName), host_name;
        *gHolmesStream >> host_name;
        if (host_name.c_str()[0] == 0) {
            MILO_FAIL(
                "Holmes fileroot missing!\nplease add -holmes_target <target> or -holmes_share <rootpath> to your commandline\n(-holmes_target is the preferred usage)"
            );
        }
        HolmesSetFileShare(my_name.c_str(), host_name.c_str());
    }
    gPendingResponse = Holmes::kInvalidOpcode;
    return fail;
}

void HolmesClientInit() {
    if (!UsingCD() || gHostConfig || gHostLogging) {
        MILO_LOG("Trying to connect to Holmes...\n");
        bool conf, log;
        if (!UsingCD()) {
            conf = gHostConfig = 0;
            log = gHostLogging = 0;
        } else {
            conf = gHostConfig;
            log = gHostLogging;
        }
        bool unk = !conf || log ? 0 : 1;
        BeginCmd(Holmes::kVersion, true);
        gHolmesTarget = OptionStr("holmes_target", gNullStr);
        String share(gShareName);
        share = OptionStr("holmes_share", share.c_str());
        share = OptionStr("xb_share", share.c_str());
        gHolmesStream = HolmesClient::PlatformCreateServerStream(unk, share.c_str());
        if (gHolmesStream == nullptr) {
            if (!unk) {
                MILO_FAIL("COULD NOT CONNECT TO HOLMES");
            }
            EndCmd(Holmes::kVersion);
            return;
        }
        bool fail = gHolmesStream->Fail();
        if (!fail) {
            gStreamBuffer = new MemStream(true);
            gStreamBuffer->Reserve(0x2000D);
            fail = HolmesClientInitOpcode(false);
            if (fail != 0 && unk) {
                return;
            }
        }
        if (fail) {
            RELEASE(gHolmesStream);
            RELEASE(gStreamBuffer);
        }
        if (fail && !unk) {
            MILO_FAIL("COULD NOT CONNECT TO HOLMES");
        }
        DataRegisterFunc("dump_holmes_log", DumpHolmesLog);
        EndCmd(Holmes::kVersion);
    }
}

void HolmesClientReInit() {
    CritSecTracker cst(&gCrit);
    if (!gHolmesStream) {
        return;
    }
    BeginCmd(Holmes::kVersion, true);
    HolmesClientInitOpcode(1);
    EndCmd(Holmes::kVersion);
    return;
}

int HolmesClientSysExec(const char *cc) {
    CritSecTracker cst(&gCrit);
    BeginCmd(Holmes::kSysExec, true);
    MILO_ASSERT(gHolmesStream, 750);
    *gStreamBuffer << u8(Holmes::kSysExec) << cc;
    HolmesFlushStreamBuffer();
    WaitForResponse(Holmes::kSysExec);
    int ret;
    *gHolmesStream >> ret;
    gPendingResponse = Holmes::kInvalidOpcode;
    EndCmd(Holmes::kSysExec);
    return ret;
}

int HolmesClientGetStat(const char *filename, FileStat &stat) {
    CritSecTracker cst(&gCrit);
    BeginCmd(Holmes::kGetStat, true);
    MILO_ASSERT(gHolmesStream, 770);
    *gStreamBuffer << u8(Holmes::kGetStat);
    *gStreamBuffer << filename;
    HolmesFlushStreamBuffer();
    WaitForResponse(Holmes::kGetStat);
    bool exists;
    *gHolmesStream >> exists;
    if (exists) {
        *gHolmesStream >> stat;
    }
    gPendingResponse = Holmes::kInvalidOpcode;
    EndCmd(Holmes::kGetStat);
    if (exists)
        return 0;
    else
        return -1;
}

int HolmesClientMkDir(const char *cc) {
    CritSecTracker cst(&gCrit);
    BeginCmd(Holmes::kMkDir, true);
    MILO_ASSERT(gHolmesStream, 818);
    *gStreamBuffer << u8(Holmes::kMkDir);
    *gStreamBuffer << cc;
    HolmesFlushStreamBuffer();
    WaitForResponse(Holmes::kMkDir);
    int ret;
    *gHolmesStream >> ret;
    gPendingResponse = Holmes::kInvalidOpcode;
    EndCmd(Holmes::kMkDir);
    return ret;
}

int HolmesClientDelete(const char *cc) {
    CritSecTracker cst(&gCrit);
    BeginCmd(Holmes::kDelete, true);
    MILO_ASSERT(gHolmesStream, 839);
    *gStreamBuffer << u8(Holmes::kDelete);
    *gStreamBuffer << cc;
    HolmesFlushStreamBuffer();
    WaitForResponse(Holmes::kDelete);
    int ret;
    *gHolmesStream >> ret;
    gPendingResponse = Holmes::kInvalidOpcode;
    EndCmd(Holmes::kDelete);
    return ret;
}

const char *HolmesFileShare() { return gShareName; }

void HolmesClientTruncate(int, int) { return; }

bool HolmesClientOpen(const char *, int, unsigned int &, int &) { return false; }

void HolmesClientWrite(int, int, int, const void *) { return; }

void HolmesClientRead(int, int, int, void *, File *) { return; }

bool HolmesClientReadDone(File *) { return false; }

void HolmesClientStackTrace(const char *cc, struct StackData *stack, int i, String &ret) {
    ret = "";
    CritSecTracker cst(&gCrit);
    if (gHolmesStream && !gHolmesStream->Fail()) {
        BeginCmd(Holmes::kStackTrace, true);
        *gStreamBuffer << u8(Holmes::kStackTrace);
        *gStreamBuffer << cc;
        *gStreamBuffer << i;
        for (int j = 0; j < i; j++) {
            *gStreamBuffer << stack->mFailThreadStack[j];
        }
        HolmesFlushStreamBuffer();
        gStackTraced = true;
        WaitForResponse(Holmes::kStackTrace);
        *gHolmesStream >> ret;
        gPendingResponse = Holmes::kInvalidOpcode;
        EndCmd(Holmes::kStackTrace);
    }
}

void HolmesClientSendMessage(const Message &msg) {
    DataNode dn(msg);
    CritSecTracker cst(&gCrit);
    if (gHolmesStream && !gHolmesStream->Fail()) {
        BeginCmd(Holmes::kSendMessage, true);
        *gStreamBuffer << u8(Holmes::kSendMessage) << dn;
        HolmesFlushStreamBuffer();
        WaitForResponse(Holmes::kSendMessage);
        int ret;
        *gHolmesStream >> ret;
        gPendingResponse = Holmes::kInvalidOpcode;
        EndCmd(Holmes::kSendMessage);
    }
}

void HolmesClientClose(File *, int) { return; }

void HolmesClientEnumerate(
    const char *, void (*)(const char *, const char *), bool, const char *, bool
) {}

bool CanUseHolmes(int p1) {
    if (!UsingCD())
        return true;

    if (gHostConfig != false && (p1 & 2U) != 0)
        return true;

    if (gHostLogging != false && (p1 & 1U) != 0)
        return true;

    return false;
}

void HolmesToLocal(char *p1, const char *p2) {}

char const *HolmesFileHostName() { return gMachineName; }

void HolmesClientPoll() {
    CritSecTracker cst(&gCrit);

    if (!gHolmesStream)
        return;

    gPollStreamEof = false;
    HolmesClientPollInternal(true);
}
