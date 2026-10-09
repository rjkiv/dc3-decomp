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
    static char gMachineName[NETBIOS_NAME_MAX] = { 0 };
    static char gShareName[NETBIOS_NAME_MAX] = { 0 };
    HolmesProfileData gProfile[20]; // to match protocol count
    CriticalSection gCrit;
    std::list<ReadRequest> gRequests;
    String gServerName;
    HolmesInput gInput(nullptr);
    String gHolmesTarget;

    bool gHolmesPrintEnable = true;
    Holmes::Protocol gPendingResponse = Holmes::kInvalidOpcode;

    void BeginCmd(Holmes::Protocol type, bool count) {
        if (count) {
            gProfile[type].count += 1;
        }
        gProfile[type].work.Start();
    }

    static const int sEndCmdState = 0x2000d;

    void EndCmd(Holmes::Protocol type) {
        gProfile[type].work.Stop();
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

    void WaitForAnyResponse(Holmes::Protocol type) {
        if (gPendingResponse == Holmes::kInvalidOpcode
            && gHolmesStream->Eof() != NotEof) {
            AutoSlowFrame frame(__FUNCTION__, 5);
            gProfile[type].wait.Start();
            float split = gProfile[type].wait.SplitMs();
            float f9 = 2000;
            while (gHolmesStream->Eof() != NotEof) {
                Timer::Sleep(0);
                if (!gStackTraced && gProfile[type].wait.SplitMs() - split > f9) {
                    printf(
                        "[Holmes] %s opcode blocked for %.0f seconds\n",
                        Holmes::ProtocolDebugString(type),
                        f9 / 1000
                    );
                    f9 += 1000;
                }
            }
            gProfile[type].wait.Stop();
        }
    }

    static const int sPossibleResponses[] = { Holmes::kReadFile,
                                              Holmes::kPollKeyboard,
                                              Holmes::kPollJoypad,
                                              Holmes::kPrint,
                                              Holmes::kInvalidOpcode };
    static Timer *holmesReadopcTimer;

    bool CheckForResponse(Holmes::Protocol type, bool IsPolling) {
        if (gPendingResponse == Holmes::kInvalidOpcode) {
            bool b9;
            if (IsPolling) {
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
        bool isPending = gPendingResponse == type;
        if (!isPending) {
            for (int i = 0; i < DIM(sPossibleResponses); i++) {
                if (sPossibleResponses[i] == gPendingResponse
                    || sPossibleResponses[i] == type) {
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
                Holmes::ProtocolDebugString(type)
            );
        }
        return gPendingResponse == type;
    }

    void CheckInput(bool isPolling) {
        if (CheckForResponse(Holmes::kPollKeyboard, isPolling)) {
            BeginCmd(Holmes::kPollKeyboard, true);
            gInput.LoadKeyboard(*gHolmesStream);
            gPendingResponse = Holmes::kInvalidOpcode;
            EndCmd(Holmes::kPollKeyboard);
        }

        if (CheckForResponse(Holmes::kPollJoypad, isPolling)) {
            BeginCmd(Holmes::kPollJoypad, true);
            gInput.LoadJoypad(*gHolmesStream);
            gPendingResponse = Holmes::kInvalidOpcode;
            EndCmd(Holmes::kPollJoypad);
        }
    };

    bool CheckReads(bool isPolling) {
        FOREACH (it, gRequests) {
            if (!CheckForResponse(Holmes::kReadFile, isPolling)) {
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

    void WaitForResponse(Holmes::Protocol type) {
        while (true) {
            if (CheckForResponse(type, false)) {
                return;
            }
            WaitForAnyResponse(type);
            if (CheckReads(false) && type == 5) {
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

    void HolmesClientPollInternal(bool isPolling) {
        CritSecTracker cst(&gCrit);

        if (!gHolmesStream)
            return;

        CheckInput(isPolling);
        CheckReads(isPolling);
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

bool CanUseHolmes(int iMode) {
    if (!UsingCD())
        return true;

    if (gHostConfig != false && (iMode & 2U) != 0)
        return true;

    if (gHostLogging != false && (iMode & 1U) != 0)
        return true;

    return false;
}

bool UsingHolmes(int iMode) {
    if (!gHolmesStream)
        return false;

    return CanUseHolmes(iMode);
}

void HolmesSetFileShare(const char *machineName, const char *shareName) {
    strncpy(gMachineName, machineName, 64);
    strncpy(gShareName, shareName, 64);
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

bool HolmesClientInitOpcode(bool reinit) {
    bool fail = 0;
    *gStreamBuffer << u8(Holmes::kVersion) << HOLMES_CURRENT_VERSION;
    *gStreamBuffer << HolmesClient::PlatformGetHostName();
    *gStreamBuffer << gHolmesTarget;
    *gStreamBuffer << &gMachineName[0x40];
    *gStreamBuffer << FileSystemRoot();
    *gStreamBuffer << u8(TheLoadMgr.GetPlatform());
    *gStreamBuffer << u8(GetGfxMode());
    HolmesFlushStreamBuffer();
    if (!reinit) {
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

int HolmesClientSysExec(const char *cmdLine) {
    CritSecTracker cst(&gCrit);
    BeginCmd(Holmes::kSysExec, true);
    MILO_ASSERT(gHolmesStream, 750);
    *gStreamBuffer << u8(Holmes::kSysExec) << cmdLine;
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

int HolmesClientMkDir(const char *path) {
    CritSecTracker cst(&gCrit);
    BeginCmd(Holmes::kMkDir, true);
    MILO_ASSERT(gHolmesStream, 818);
    *gStreamBuffer << u8(Holmes::kMkDir);
    *gStreamBuffer << path;
    HolmesFlushStreamBuffer();
    WaitForResponse(Holmes::kMkDir);
    int ret;
    *gHolmesStream >> ret;
    gPendingResponse = Holmes::kInvalidOpcode;
    EndCmd(Holmes::kMkDir);
    return ret;
}

int HolmesClientDelete(const char *path) {
    CritSecTracker cst(&gCrit);
    BeginCmd(Holmes::kDelete, true);
    MILO_ASSERT(gHolmesStream, 839);
    *gStreamBuffer << u8(Holmes::kDelete);
    *gStreamBuffer << path;
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

void HolmesClientTruncate(int file_handle, int length) {
    CritSecTracker tracker(&gCrit);
    MILO_ASSERT(gHolmesStream, 0x3AD);
    if (!gHolmesStream->Fail() || !gHostLogging) {
        BeginCmd(Holmes::kTruncateFile, true);
        *gStreamBuffer << (unsigned char)Holmes::kTruncateFile << file_handle << length;
        HolmesFlushStreamBuffer();
        WaitForResponse(Holmes::kTruncateFile);
        int x;
        *gHolmesStream >> x;
        gPendingResponse = Holmes::kInvalidOpcode;
        EndCmd(Holmes::kTruncateFile);
        return;
    }
}

bool HolmesClientOpen(const char *filename, int mode, unsigned int &file_size, int &file_handle) {
    CritSecTracker tracker(&gCrit);
    if (gHostLogging) {
        if (mode & 1U) {
            if (!gHolmesStream) {
                return false;
            }
        } else if (!gHostConfig) {
            MILO_FAIL("gHostLogging tried to read file: %s", filename);
        }
    }
    MILO_ASSERT(gHolmesStream, 0x36A);
    if (gHolmesStream->Fail()) {
        return false;
    } else {
        BeginCmd(Holmes::kOpenFile, true);
        unsigned char val = 3;
        *gStreamBuffer << val << filename;
        val = (mode >> 1) & 1;
        *gStreamBuffer << val;
        *gStreamBuffer << (unsigned char)((mode >> 0x12) & 1);
        if (val == 0) {
            *gStreamBuffer << (unsigned char)((mode >> 8) & 1);
            val = (mode >> 9) & 1;
            *gStreamBuffer << val;
        }
        HolmesFlushStreamBuffer();
        WaitForResponse(Holmes::kOpenFile);
        int i7c;
        *gHolmesStream >> i7c;
        if (i7c != -1) {
            *gHolmesStream >> file_handle;
            file_size = i7c;
        }
        gPendingResponse = Holmes::kInvalidOpcode;
        EndCmd(Holmes::kOpenFile);
        if (i7c != -1) {
            return true;
        }
    }
    return false;
}

void HolmesClientWrite(int file_handle, int start, int bytes, const void *buf) {
    if (bytes != 0) {
        CritSecTracker tracker(&gCrit);
        MILO_ASSERT(gHolmesStream, 0x395);
        if (!gHolmesStream->Fail() || !gHostLogging) {
            BeginCmd(Holmes::kWriteFile, true);
            *gStreamBuffer << (unsigned char)Holmes::kWriteFile << file_handle << start << bytes;
            gStreamBuffer->Write(buf, bytes);
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

void HolmesClientRead(int file_handle, int start, int bytes, void *buf, File *requestor) {
    if (bytes != 0) {
        CritSecTracker tracker(&gCrit);
        MILO_ASSERT(gHolmesStream, 0x3C7);
        BeginCmd(Holmes::kReadFile, true);
        *gStreamBuffer << (unsigned char)Holmes::kReadFile << file_handle << start << bytes;
        HolmesFlushStreamBuffer();

        ReadRequest req;
        req.mRequestor = requestor;
        req.mBuffer = buf;
        req.mBytes = bytes;
        gRequests.push_back(req);
        EndCmd(Holmes::kReadFile);
        return;
    }
}

bool HolmesClientReadDone(File *f) {
    CritSecTracker tracker(&gCrit);
    if (PendingRead(f)) {
        HolmesClientPollInternal(false);
        return !PendingRead(f);
    } else {
        return false;
    }
}

void HolmesClientClose(File *f, int file_handle) {
    CritSecTracker tracker(&gCrit);
    BeginCmd(Holmes::kCloseFile, true);
    MILO_ASSERT(gHolmesStream, 0x3F4);
    if (PendingRead(f)) {
        WaitForReads();
    }
    *gStreamBuffer << (unsigned char)Holmes::kCloseFile << file_handle;
    HolmesFlushStreamBuffer();
    EndCmd(Holmes::kCloseFile);
}

CacheResourceResult HolmesClientCacheResource(const char *filename, const char *cachedName) {
    AutoSlowFrame frame(__FUNCTION__, 1000);
    CritSecTracker tracker(&gCrit);
    BeginCmd(Holmes::kCacheResource, true);
    gLastCachedResource = cachedName;
    MILO_ASSERT(gHolmesStream, 0x4CC);
    *gStreamBuffer << (unsigned char)Holmes::kCacheResource;
    *gStreamBuffer << filename;
    HolmesFlushStreamBuffer();
    WaitForResponse(Holmes::kCacheResource);
    char result;
    *gHolmesStream >> result;
    gPendingResponse = Holmes::kInvalidOpcode;
    gLastCacheResult = (CacheResourceResult)result;
    EndCmd(Holmes::kCacheResource);
    return gLastCacheResult;
}

bool HolmesClientCacheFile(char *localName, const char *filename) {
    CritSecTracker tracker(&gCrit);
    AutoSlowFrame frame(__FUNCTION__, 20000);
    BeginCmd(Holmes::kCacheFile, true);
    String str(filename);
    HolmesToLocal(localName, str.c_str());
    if (*localName == '\0') {
        EndCmd(Holmes::kCacheFile);
        return false;
    } else {
        FileStat curStat;
        bool fileRes = GetFileAttributesExA(localName, GetFileExInfoStandard, &curStat);
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
    const char *dir,
    void (*cb)(const char *, const char *), // param called cb in the pdb
    bool recurse,
    const char *pattern,
    bool dirs
) {
    CritSecTracker tracker(&gCrit);
    BeginCmd(Holmes::kEnumerate, true);
    *gStreamBuffer << (unsigned char)Holmes::kEnumerate;
    *gStreamBuffer << dir << recurse << pattern << dirs;
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
        cb(info[i].s1.c_str(), info[i].s2.c_str());
    }
    EndCmd(Holmes::kEnumerate);
}

void HolmesClientStackTrace(const char *file, struct StackData *stack, int stack_size, String &stack_desc) {
    stack_desc = "";
    CritSecTracker cst(&gCrit);
    if (gHolmesStream && !gHolmesStream->Fail()) {
        BeginCmd(Holmes::kStackTrace, true);
        *gStreamBuffer << u8(Holmes::kStackTrace);
        *gStreamBuffer << file;
        *gStreamBuffer << stack_size;
        for (int j = 0; j < stack_size; j++) {
            *gStreamBuffer << stack->mFailThreadStack[j];
        }
        HolmesFlushStreamBuffer();
        gStackTraced = true;
        WaitForResponse(Holmes::kStackTrace);
        *gHolmesStream >> stack_desc;
        gPendingResponse = Holmes::kInvalidOpcode;
        EndCmd(Holmes::kStackTrace);
        return;
    }
}

void HolmesClientSendMessage(const Message &m) {
    DataNode dn(m);
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

void HolmesToLocal(char *out, const char *file) {
    String path;
    path = HolmesXboxPath(gServerName.c_str(), file);
    strcpy(out, path.c_str());
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
