#include "obj/Data.h"
#include "obj/DataFunc.h"
#include "obj/Msg.h"
#include "os/CritSec.h"
#include "os/Debug.h"
#include "os/File.h"
#include "os/HolmesClient.h"
#include "os/HolmesKeyboard.h"
#include "os/HolmesUtl.h"
#include "os/NetworkSocket.h"
#include "os/System.h"
#include "os/Timer.h"
#include "utl/BinStream.h"
#include "utl/Cache.h"
#include "utl/Loader.h"
#include "utl/MemStream.h"
#include "utl/Option.h"
#include "utl/Symbol.h"
#include "utl/TextFileStream.h"
#include "xdk/xapilibi/fileapi.h"
#include "xdk/xapilibi/minwinbase.h"
#include <cstdio>
#include <list>

#pragma region Statics

#define HOLMES_CURRENT_VERSION 26
#define NETBIOS_NAME_MAX 64

CacheResourceResult gLastCacheResult;
String gLastCachedResource;

#pragma region Private details

namespace {
    struct HolmesProfileData {
        Timer wait;
        Timer work;
        int count;
        u32 pad;
    };

    struct ReadRequest {
        File *mRequestor;
        void *mBuffer;
        int mBytes;
    };

    int gRealMaxBufferSize;
    bool gStackTraced;
    bool gPollStreamEof;
    bool gInputPolling;
    BinStream *gHolmesStream;
    MemStream *gStreamBuffer;
    int gActivePrintCount;
    char gMachineName[NETBIOS_NAME_MAX] = { 0 };
    char gShareName[NETBIOS_NAME_MAX] = { 0 };
    HolmesProfileData gProfile[20]; // to match protocol count
    CriticalSection gCrit;
    std::list<ReadRequest> gRequests;
    String gServerName;
    HolmesInput gInput(nullptr);
    String gHolmesTarget;

    bool gHolmesPrintEnable = true;
    Holmes::Protocol gPendingResponse = Holmes::kInvalidOpcode;

    void BeginCmd(Holmes::Protocol prot, bool b) {
        if (b) {
            gProfile[prot].count += 1;
        }
        gProfile[prot].work.Start();
    }

    static const int sEndCmdState = 0x2000d;

    void EndCmd(Holmes::Protocol prot) {
        gProfile[prot].work.Stop();
        if (gRealMaxBufferSize != 0) {
            MILO_NOTIFY_ONCE(
                "HolmesClient buffer exceeded %d < %d", sEndCmdState, gRealMaxBufferSize
            );
        }
    }

    void HolmesFlushStreamBuffer() {
        if (gStreamBuffer->Size() > 0x2000D) {
            gRealMaxBufferSize = gStreamBuffer->Size();
        }
        gHolmesStream->Write(gStreamBuffer->Buffer(), gStreamBuffer->Size());
        gStreamBuffer->Seek(0, BinStream::kSeekEnd);
        gStreamBuffer->Compact();
    }

    void WaitForAnyResponse(Holmes::Protocol prot) {
        if (gPendingResponse == Holmes::kInvalidOpcode
            && gHolmesStream->Eof() != NotEof) {
            AutoSlowFrame frame(__FUNCTION__, 5);
            gProfile[prot].wait.Start();
            float split = gProfile[prot].wait.SplitMs();
            float f9 = 2000;
            while (gHolmesStream->Eof() != NotEof) {
                Timer::Sleep(0);
                if (!gStackTraced && gProfile[prot].wait.SplitMs() - split > f9) {
                    printf(
                        "[Holmes] %s opcode blocked for %.0f seconds\n",
                        Holmes::ProtocolDebugString(prot),
                        f9 / 1000
                    );
                    f9 += 1000;
                }
            }
            gProfile[prot].wait.Stop();
        }
    }

    static const int sPossibleResponses[] = { Holmes::kReadFile,
                                              Holmes::kPollKeyboard,
                                              Holmes::kPollJoypad,
                                              Holmes::kPrint,
                                              Holmes::kInvalidOpcode };
    static Timer *holmesReadopcTimer;

    bool CheckForResponse(Holmes::Protocol prot, bool b) {
        if (gPendingResponse == Holmes::kInvalidOpcode) {
            bool b9;
            if (b) {
                gPollStreamEof = gHolmesStream->Eof() != NotEof;
                b9 = gPollStreamEof;
            } else {
                b9 = gHolmesStream->Eof() != NotEof;
            }
            if (!b9) {
                if (!holmesReadopcTimer) {
                    holmesReadopcTimer = AutoTimer::GetTimer("holmes_readopc");
                }
                AutoTimer _at(holmesReadopcTimer, 50.0f, NULL, NULL);
                unsigned char response;
                *gHolmesStream >> response;
                gPendingResponse = (Holmes::Protocol)response;
                MILO_ASSERT(gPendingResponse != Holmes::kInvalidOpcode, 0xEF);
            }
        }
        bool isPending = gPendingResponse == prot;
        if (!isPending) {
            for (int i = 0; i < DIM(sPossibleResponses); i++) {
                if (sPossibleResponses[i] == gPendingResponse
                    || sPossibleResponses[i] == prot) {
                    isPending = true;
                    break;
                }
            }
        }
        if (gHolmesStream->Fail()) {
            MILO_FAIL("holmes closed");
        } else if (!isPending) {
            MILO_FAIL(
                "this shouldn't be happening %s %s\n",
                Holmes::ProtocolDebugString(gPendingResponse),
                Holmes::ProtocolDebugString(prot)
            );
        }
        return gPendingResponse == prot;
    }

    void CheckInput(bool b) {
        if (CheckForResponse(Holmes::kPollKeyboard, b)) {
            BeginCmd(Holmes::kPollKeyboard, true);
            gInput.LoadKeyboard(*gHolmesStream);
            gPendingResponse = Holmes::kInvalidOpcode;
            EndCmd(Holmes::kPollKeyboard);
        }

        if (CheckForResponse(Holmes::kPollJoypad, b)) {
            BeginCmd(Holmes::kPollJoypad, true);
            gInput.LoadJoypad(*gHolmesStream);
            gPendingResponse = Holmes::kInvalidOpcode;
            EndCmd(Holmes::kPollJoypad);
        }
    };

    bool CheckReads(bool b) {
        FOREACH (it, gRequests) {
            if (!CheckForResponse(Holmes::kReadFile, b)) {
                return false;
            }
            BeginCmd(Holmes::kReadFile, false);
            ReadRequest &cur = *it;
            int i2 = gHolmesStream->ReadAsync(cur.mBuffer, cur.mBytes);
            char *buffer = (char *)cur.mBuffer;
            buffer += i2;
            cur.mBytes -= i2;
            EndCmd(Holmes::kReadFile);
            if (i2 <= 0) {
                return false;
            }
            if (cur.mBytes == 0) {
                gRequests.erase(it);
                gPendingResponse = Holmes::kInvalidOpcode;
                return true;
            }
        }
        return false;
    }

    void WaitForResponse(Holmes::Protocol prot) {
        while (true) {
            if (CheckForResponse(prot, false)) {
                return;
            }
            WaitForAnyResponse(prot);
            if (CheckReads(false) && prot == 5) {
                return;
            }
            CheckInput(false);
        }
    }

    void WaitForReads() {
        CritSecTracker tracker(&gCrit);
        while (true) {
            if (gRequests.empty()) {
                return;
            }
            while (!CheckForResponse(Holmes::kReadFile, false)) {
                WaitForAnyResponse(Holmes::kReadFile);
                if (CheckReads(false)) {
                    break;
                }
                CheckInput(false);
            }
            CheckReads(false);
        }
    }

    void HolmesClientPollInternal(bool b) {
        CritSecTracker cst(&gCrit);

        if (!gHolmesStream)
            return;

        CheckInput(b);
        CheckReads(b);
    };
}

#pragma region Public API

bool PendingRead(File *file) {
    FOREACH (it, gRequests) {
        if (it->mRequestor == file) {
            return true;
        }
    }
    return false;
}

bool CanUseHolmes(int p1) {
    if (!UsingCD())
        return true;

    if (gHostConfig != false && (p1 & 2U) != 0)
        return true;

    if (gHostLogging != false && (p1 & 1U) != 0)
        return true;

    return false;
}

bool UsingHolmes(int p1) {
    if (!gHolmesStream)
        return false;

    return CanUseHolmes(p1);
}

void HolmesSetFileShare(const char *machine, const char *share) {
    strncpy(gMachineName, machine, 64);
    strncpy(gShareName, share, 64);
}

const char *HolmesFileHostName() { return gMachineName; }
const char *HolmesFileShare() { return gShareName; }

NetAddress HolmesResolveIP() {
    if (CanUseHolmes(3))
        return HolmesClient::PlatformResolveIP();
    else
        return NetAddress();
}

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

static const int kHolmesCurrentVersion = HOLMES_CURRENT_VERSION;

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
                kHolmesCurrentVersion
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
        if (!UsingCD()) {
            gHostConfig = false;
            gHostLogging = false;
        }
        bool unk = gHostConfig && !gHostLogging;
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

void HolmesClientPollKeyboard() {
    HolmesClientPollInternal(true);
    if (!gInputPolling) {
        gInputPolling = true;
        gInput.SendKeyboardMessages();
        gInputPolling = false;
    }
}

unsigned int HolmesClientPollJoypad() {
    unsigned int ret;
    HolmesClientPollInternal(true);
    if (gInputPolling) {
        ret = 0;
    } else {
        gInputPolling = true;
        ret = gInput.SendJoypadMessages();
        gInputPolling = false;
    }
    return ret;
}

void HolmesClientTruncate(int i1, int i2) {
    CritSecTracker tracker(&gCrit);
    MILO_ASSERT(gHolmesStream, 0x3AD);
    if (!gHolmesStream->Fail() || !gHostLogging) {
        BeginCmd(Holmes::kTruncateFile, true);
        *gStreamBuffer << (unsigned char)Holmes::kTruncateFile << i1 << i2;
        HolmesFlushStreamBuffer();
        WaitForResponse(Holmes::kTruncateFile);
        int x;
        *gHolmesStream >> x;
        gPendingResponse = Holmes::kInvalidOpcode;
        EndCmd(Holmes::kTruncateFile);
        return;
    }
}

bool HolmesClientOpen(const char *cc1, int i2, unsigned int &uiref, int &iref) {
    CritSecTracker tracker(&gCrit);
    if (gHostLogging) {
        if (i2 & 1U) {
            if (!gHolmesStream) {
                return false;
            }
        } else if (!gHostConfig) {
            MILO_FAIL("gHostLogging tried to read file: %s", cc1);
        }
    }
    MILO_ASSERT(gHolmesStream, 0x36A);
    if (gHolmesStream->Fail()) {
        return false;
    } else {
        BeginCmd(Holmes::kOpenFile, true);
        unsigned char val = 3;
        *gStreamBuffer << val << cc1;
        val = (i2 >> 1) & 1;
        *gStreamBuffer << val;
        *gStreamBuffer << (unsigned char)((i2 >> 0x12) & 1);
        if (val == 0) {
            *gStreamBuffer << (unsigned char)((i2 >> 8) & 1);
            val = (i2 >> 9) & 1;
            *gStreamBuffer << val;
        }
        HolmesFlushStreamBuffer();
        WaitForResponse(Holmes::kOpenFile);
        int i7c;
        *gHolmesStream >> i7c;
        if (i7c != -1) {
            *gHolmesStream >> iref;
            uiref = i7c;
        }
        gPendingResponse = Holmes::kInvalidOpcode;
        EndCmd(Holmes::kOpenFile);
        if (i7c != -1) {
            return true;
        }
    }
    return false;
}

void HolmesClientWrite(int i1, int i2, int i3, const void *v) {
    if (i3 != 0) {
        CritSecTracker tracker(&gCrit);
        MILO_ASSERT(gHolmesStream, 0x395);
        if (!gHolmesStream->Fail() || !gHostLogging) {
            BeginCmd(Holmes::kWriteFile, true);
            *gStreamBuffer << (unsigned char)Holmes::kWriteFile << i1 << i2 << i3;
            gStreamBuffer->Write(v, i3);
            HolmesFlushStreamBuffer();
            WaitForResponse(Holmes::kWriteFile);
            int x;
            *gHolmesStream >> x;
            gPendingResponse = Holmes::kInvalidOpcode;
            EndCmd(Holmes::kWriteFile);
            return;
        }
    }
}

void HolmesClientRead(int i1, int i2, int i3, void *v, File *file) {
    if (i3 != 0) {
        CritSecTracker tracker(&gCrit);
        MILO_ASSERT(gHolmesStream, 0x3C7);
        BeginCmd(Holmes::kReadFile, true);
        *gStreamBuffer << (unsigned char)Holmes::kReadFile << i1 << i2 << i3;
        HolmesFlushStreamBuffer();

        ReadRequest req;
        req.mRequestor = file;
        req.mBuffer = v;
        req.mBytes = i3;
        gRequests.push_back(req);
        EndCmd(Holmes::kReadFile);
        return;
    }
}

bool HolmesClientReadDone(File *file) {
    CritSecTracker tracker(&gCrit);
    if (PendingRead(file)) {
        HolmesClientPollInternal(false);
        return !PendingRead(file);
    } else {
        return false;
    }
}

void HolmesClientClose(File *file, int i2) {
    CritSecTracker tracker(&gCrit);
    BeginCmd(Holmes::kCloseFile, true);
    MILO_ASSERT(gHolmesStream, 0x3F4);
    if (PendingRead(file)) {
        WaitForReads();
    }
    *gStreamBuffer << (unsigned char)Holmes::kCloseFile << i2;
    HolmesFlushStreamBuffer();
    EndCmd(Holmes::kCloseFile);
}

CacheResourceResult HolmesClientCacheResource(const char *c1, const char *c2) {
    AutoSlowFrame frame(__FUNCTION__, 1000);
    CritSecTracker tracker(&gCrit);
    BeginCmd(Holmes::kCacheResource, true);
    gLastCachedResource = c2;
    MILO_ASSERT(gHolmesStream, 0x4CC);
    *gStreamBuffer << (unsigned char)Holmes::kCacheResource;
    *gStreamBuffer << c1;
    HolmesFlushStreamBuffer();
    WaitForResponse(Holmes::kCacheResource);
    char result;
    *gHolmesStream >> result;
    gPendingResponse = Holmes::kInvalidOpcode;
    gLastCacheResult = (CacheResourceResult)result;
    EndCmd(Holmes::kCacheResource);
    return gLastCacheResult;
}

bool HolmesClientCacheFile(char *c1, const char *cc2) {
    CritSecTracker tracker(&gCrit);
    AutoSlowFrame frame(__FUNCTION__, 20000);
    BeginCmd(Holmes::kCacheFile, true);
    String str(cc2);
    HolmesToLocal(c1, str.c_str());
    if (*c1 == '\0') {
        EndCmd(Holmes::kCacheFile);
        return false;
    } else {
        FileStat curStat;
        bool fileRes = GetFileAttributesExA(c1, GetFileExInfoStandard, &curStat);
        u64 time = curStat.st_mtime;
        if (str == gLastCachedResource && (gLastCacheResult > 0 || fileRes)) {
            EndCmd(Holmes::kCacheFile);
            return true;
        } else {
            *gStreamBuffer << (unsigned char)Holmes::kCacheFile << str << fileRes;
            if (fileRes) {
                *gStreamBuffer >> time;
            }
            HolmesFlushStreamBuffer();
            WaitForResponse(Holmes::kCacheFile);
            bool ret;
            *gHolmesStream >> ret;
            gPendingResponse = Holmes::kInvalidOpcode;
            EndCmd(Holmes::kCacheFile);
            return ret;
        }
    }
}

void HolmesClientEnumerate(
    const char *cc1,
    void (*func)(const char *, const char *),
    bool b3,
    const char *cc4,
    bool b5
) {
    CritSecTracker tracker(&gCrit);
    BeginCmd(Holmes::kEnumerate, true);
    *gStreamBuffer << (unsigned char)Holmes::kEnumerate;
    *gStreamBuffer << cc1 << b3 << cc4 << b5;
    HolmesFlushStreamBuffer();
    std::vector<RecurseInfo> info;
    WaitForResponse(Holmes::kEnumerate);
    while (true) {
        bool b80;
        *gHolmesStream >> b80;
        if (!b80)
            break;
        info.push_back(RecurseInfo());
        *gHolmesStream >> info.back().s1 >> info.back().s2;
    }
    gPendingResponse = Holmes::kInvalidOpcode;
    for (int i = 0; i < info.size(); i++) {
        func(info[i].s1.c_str(), info[i].s2.c_str());
    }
    EndCmd(Holmes::kEnumerate);
}

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
        return;
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
        unsigned char ret;
        *gHolmesStream >> ret;
        gPendingResponse = Holmes::kInvalidOpcode;
        EndCmd(Holmes::kSendMessage);
        return;
    }
}

void HolmesToLocal(char *p1, const char *p2) {
    String path;
    path = HolmesXboxPath(gServerName.c_str(), p2);
    strcpy(p1, path.c_str());
}

void HolmesClientPoll() {
    CritSecTracker cst(&gCrit);

    if (!gHolmesStream)
        return;

    gPollStreamEof = false;
    HolmesClientPollInternal(true);
}

void HolmesClientTerminate() {
    CritSecTracker tracker(&gCrit);
    if (!gHolmesStream)
        return;
    else {
        BeginCmd(Holmes::kTerminate, true);
        DumpHolmesLog(nullptr);
        if (gHolmesStream) {
            if (!gHolmesStream->Fail()) {
                unsigned char uc = 0xD;
                *gStreamBuffer << uc;
                HolmesFlushStreamBuffer();
            }
            delete gHolmesStream;
        }
        gHolmesStream = nullptr;
        RELEASE(gStreamBuffer);
    }
}
