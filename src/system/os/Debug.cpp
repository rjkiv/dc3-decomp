#include "os/Debug.h"
#include "HolmesClient.h"
#include "obj/Data.h"
#include "os/AppChild.h"
#include "os/CritSec.h"
#include "os/File.h"
#include "os/NetworkSocket.h"
#include "os/OSFuncs.h"
#include "os/SynchronizationEvent.h"
#include "os/System.h"
#include "os/Timer.h"
#include "utl/Cheats.h"
#include "utl/DataPointMgr.h"
#include "utl/Loader.h"
#include "utl/MemMgr.h"
#include "utl/Option.h"
#include "utl/TextFileStream.h"
#include "xdk/win_types.h"
#include "xdk/xapilibi/errhandlingapi.h"
#include "xdk/xbdm/xbdm.h"
#include <vector>
#include "xdk/XAPILIB.h"
#include "utl/Std.h"

const char *kAssertStr = "File: %s Line: %d Error: %s\n";
bool gMemoryUsageTest;
DebugWarner TheDebugWarner;
DebugNotifier TheDebugNotifier;
DebugFailer TheDebugFailer;
SynchronizationEvent gNotifyThreadSync;
CriticalSection gNotifyThreadSec;
Debug TheDebug;
std::vector<String> gNotifies;

typedef void ModalCallbackFunc(Debug::ModalType &, FixedString &, bool);

void Debug::SetDisabled(bool d) { mNoDebug = d; }

void Debug::StopLog() { RELEASE(mLog); }

const char *DevHostname(Symbol s) {
    static Symbol hostnames = "hostnames";
    return SystemConfig() ? SystemConfig(hostnames, s)->Str(1) : nullptr;
}

ModalCallbackFunc *Debug::SetModalCallback(ModalCallbackFunc *func) {
    if (mNoModal) {
        return nullptr;
    }
    ModalCallbackFunc *oldFunc = mModalCallback;
    mModalCallback = func;
    if (gNotifies.size() != 0) {
        for (int i = 0; i < gNotifies.size(); i++) {
            MILO_LOG("%s\n", gNotifies[i].c_str());
        }
        gNotifies.clear();
    }
    return oldFunc;
}

void DebugModal(enum Debug::ModalType &ty, class FixedString &str, bool b3) {
    if (ty == Debug::kModalFail) {
        str += "\n\n-- Program ended --\n";
    } else {
        gNotifies.push_back(str.c_str());
    }
    MILO_LOG("%s\n", str.c_str());
}

Debug::Debug()
    : mNoDebug(0), mFailing(0), mExiting(0), mNoTry(0), mNoModal(0), mTry(0), mLog(0),
      mAlwaysFlush(0), mReflect(0), mModalCallback(DebugModal), unk38(0),
      mFailThreadMsg(0), mNotifyThreadMsg(0), unk10c(0), unk110(0) {}

void Debug::RemoveExitCallback(ExitCallbackFunc *func) {
    if (!mExiting) {
        mExitCallbacks.remove(func);
    }
}

Debug::~Debug() { StopLog(); }

void Debug::Print(const char *msg) {
    if (mLog) {
        mLog->Print(msg);
        if (mAlwaysFlush) {
            mLog->File().Flush();
        }
    }
    if (MainThread() && mReflect) {
        mReflect->Print(msg);
    }
    if (!UsingCD()) {
        HolmesClientPrint(msg);
    }
    OutputDebugStringA(msg);
}

void Debug::Exit(int exitCode, bool call_exit) {
    if (!mExiting) {
        mExiting = true;
        MILO_LOG("APP EXITING\n");
        MILO_LOG("EXIT CODE %d call_exit %d\n", exitCode, call_exit);
        if (!gMemoryUsageTest) {
            FOREACH (it, mExitCallbacks) {
                (*it)();
            }
        }
        mExitCallbacks.clear();
        if (call_exit) {
            XLaunchNewImage("", 0);
        }
    }
}

void Debug::Warn(const char *msg) {
    ModalType type;
    if (!mNoDebug) {
        if (!MainThread()) {
            MILO_LOG("THREAD-NOTIFY: %s\n", msg);
            if (mModalCallback) {
                CritSecTracker tracker(&gNotifyThreadSec);
                mNotifyThreadMsg = msg;
                gNotifyThreadSync.Wait(200);
            }
        } else {
            type = kModalWarn;
            Modal(type, msg, nullptr);
        }
    }
}

void Debug::Notify(const char *msg) {
    ModalType type;
    if (!mNoDebug) {
        if (!MainThread()) {
            MILO_LOG("THREAD-NOTIFY: %s\n", msg);
            if (mModalCallback) {
                CritSecTracker tracker(&gNotifyThreadSec);
                mNotifyThreadMsg = msg;
                gNotifyThreadSync.Wait(200);
            }
        } else {
            type = kModalNotify;
            Modal(type, msg, nullptr);
        }
    }
}

void Debug::Fail(const char *msg, void *v) {
    if (!mNoDebug && !mFailing) {
        bool &failing = mFailing;
        failing = true;
        StackString<256> msgStr(msg);
        StackString<4096> stackTrace;
        DataAppendStackTrace(stackTrace);
        MILO_LOG(stackTrace.c_str());
        static int heap = MemFindHeap("main");
        {
            MemHeapTracker tracker(heap);
            if (!MainThread()) {
                CaptureStackTrace(DIM(mStackData.mFailThreadStack), &mStackData, v);
                mFailThreadMsg = msg;
                MILO_LOG("THREAD-FAIL: %s\n", msgStr);
                while (true) {
                    Timer::Sleep(200);
                    PlatformDebugBreak();
                }
            }
            if (mTry) {
                mTry--;
                throw msg;
            }
            FOREACH (it, mFailCallbacks) {
                (*it)();
            }
            mFailCallbacks.clear();
            ModalType t = kModalFail;
            Modal(t, msgStr.c_str(), v);
            if (t != kModalFail) {
                mFailing = false;
            }
        }
        mFailing = false;
    }
}

void Debug::Poll() {
    MILO_ASSERT(MainThread(), 0x1D4);
    if (mTry) {
        int oldTry = mTry;
        mTry = 0;
        MILO_FAIL("TRY conditional not exited %d", oldTry);
    }
    if (mFailThreadMsg) {
        Fail(mFailThreadMsg, nullptr);
    }
    if (mNotifyThreadMsg) {
        String notifyStr(mNotifyThreadMsg);
        mNotifyThreadMsg = nullptr;
        gNotifyThreadSync.Set();
        Notify(notifyStr.c_str());
    }
}

void Debug::SetTry(bool tryBool) {
    MILO_ASSERT(MainThread(), 0x1F5);
    if (!mNoTry) {
        if (tryBool) {
            mTry++;
        } else
            mTry--;
    }
}

void Debug::StartLog(const char *log, bool flush) {
    RELEASE(mLog);
    mLog = new TextFileStream(log, false);
    mAlwaysFlush = flush;
    if (mLog->File().Fail()) {
        MILO_NOTIFY("Couldn't open log %s", log);
        RELEASE(mLog);
    }
}

LONG HmxGlobalHandler(EXCEPTION_POINTERS *);

void Debug::Init() {
    mNoTry = OptionBool("no_try", false);
    const char *log = OptionStr("log", nullptr);
    if (log) {
        StartLog(log, true);
    }
    if (OptionBool("no_modal", false)) {
        SetModalCallback(nullptr);
        mNoModal = true;
    } else {
        SetModalCallback(DebugModal);
    }
    log = OptionStr("log", nullptr);
    if (log) {
        StartLog(log, true);
    }
    SetUnhandledExceptionFilter(HmxGlobalHandler);
    mFailing = false;
    DM_SYSTEM_INFO sysInfo;
    sysInfo.SizeOfStruct = 0x20;
    if (SUCCEEDED(DmGetSystemInfo(&sysInfo))) {
        unk11c = MakeString("%d.%d", sysInfo.XDKVersion.Build, sysInfo.XDKVersion.Qfe);
    }
    unk12c = NetworkSocket::GetHostName();
}

void Debug::Modal(Debug::ModalType &t, const char *msg, void *v) {
    String str = msg;
    DoCrucible(t, str.c_str(), nullptr);
    StackString<4096> outputStr(str.c_str());
    StackString<256> cheatsLog;
    StackString<512> dataStackTrace;
    StackString<2048> cStackTrace;
    if (t == kModalFail) {
        MILO_LOG("FAIL-MSG: %s\n", msg);
        if (mModalCallback) {
            mModalCallback(t, outputStr, false);
        }
        if (mFailThreadMsg) {
            AppendThreadStackTrace(outputStr, &mStackData);
        } else {
            String sysCfgFile;
            String versionStr;
            if (SystemConfig()) {
                sysCfgFile = SystemConfig()->File();
                SystemConfig()->FindData("version", versionStr, false);
            } else {
                sysCfgFile = "<unknown>";
            }
            outputStr += MakeString(
                "\n\nConsoleName: %s   %s   Plat: %s   ",
                NetworkSocket::GetHostName(),
                versionStr,
                PlatformSymbol(TheLoadMgr.GetPlatform())
            );
            outputStr +=
                MakeString("\nLang: %s   SystemConfig: %s", SystemLanguage(), sysCfgFile);

            outputStr += MakeString(
                "\nUptime: %.2f hrs   UsingCD: %s   SDK: %s",
                SystemMs() * 2.7777777777777778E-7,
                UsingCD() ? "true" : "false",
                unk11c
            );
            FOREACH (it, unk30) {
                (*it)(outputStr);
            }
            AppendCheatsLog(cheatsLog);
            outputStr += cheatsLog.c_str();
            DataAppendStackTrace(dataStackTrace);
            outputStr += dataStackTrace.c_str();
            AppendStackTrace(cStackTrace, v);
            outputStr += "\n";
            outputStr += cStackTrace.c_str();
        }
        if (t == kModalFail && TheAppChild) {
            TheAppChild->Sync(2);
        }
    }
    if (mModalCallback) {
        mModalCallback(t, outputStr, true);
    } else {
        const char *modalStrs[3] = { "WARN", "NOTIFY", "FAIL" };
        const char *myStr = modalStrs[t];
        MILO_LOG("%s: %s\n", myStr, outputStr);
    }
    if (t == kModalFail) {
        if (mModalCallback) {
            PlatformDebugBreak();
        }
        Exit(1, true);
    }
}

void Debug::DoCrucible(Debug::ModalType t, const char *msg, void *v) {
    if (!unk10c) {
        if (SystemConfig()) {
            DataArray *cfg = SystemConfig()->FindArray("crucible", false);
            if (cfg) {
                unk10c = cfg->FindStr("hostname");
                unk110 = cfg->FindStr("app");
                unk114 = cfg->FindStr("project");
            }
        }
        if (!unk10c) {
            unk10c = DevHostname("crucible");
        }
    }
    DataPoint pt40;
    DataPoint pt20;
    pt40.AddPair("message", msg);
    const char *severityMsg;
    if (t == kModalFail) {
        severityMsg = "crash";
    } else if (t == kModalNotify) {
        severityMsg = "notify";
    } else {
        severityMsg = "warn";
    }
    pt40.AddPair("severity", severityMsg);
    pt40.AddPair("project", unk114.c_str());
    pt40.AddPair("platform", PlatformSymbol(TheLoadMgr.GetPlatform()));
    pt40.AddPair("source", unk12c);
    {
        String sysCfgFile;
        String versionStr;
        if (SystemConfig()) {
            sysCfgFile = SystemConfig()->File();
            SystemConfig()->FindData("version", versionStr, false);
        } else {
            sysCfgFile = "<unknown>";
        }
        pt20.AddPair("config_name", sysCfgFile);
        pt40.AddPair("version", versionStr);
    }
    pt20.AddPair("uptime", SystemMs());
    {
        StackString<256> str1bf0(TheSystemArgs.empty() ? "" : TheSystemArgs.front());
        StackString<256> str1d00(str1bf0.c_str());
        str1d00 = FileGetBase(str1d00.c_str());
        if (str1d00.length() > 3) {
            if (str1d00[str1d00.length() - 2] == '_') {
                str1d00[str1d00.length() - 2] = '\0';
            }
        }
        str1bf0.ReplaceAll('\\', '/');
        pt20.AddPair("path", str1bf0.c_str());
        if (unk110) {
            pt40.AddPair("application", unk110);
        } else {
            pt40.AddPair("application", str1d00.c_str());
        }
    }
    {
        StackString<256> str1ae0;
        for (int i = 0; i < TheSystemArgs.size(); i++) {
            StackString<256> str19d0(TheSystemArgs[i]);
            str19d0.ReplaceAll('\\', '/');
            str1ae0 += str19d0.c_str();
            str1ae0 += "\r\n";
        }
        pt20.AddPair("args", str1ae0.c_str());
    }
    pt20.AddPair("opsys", unk11c);
    pt40.AddPair("extra", "");
    if (t == kModalFail) {
        StackString<512> str16a0;
        DataAppendStackTrace(str16a0);
        StackString<2048> str1490;
        AppendStackTrace(str1490, v);
        StackString<3096> strc80;
        strc80 += "\r\n";
        strc80 += str1490.c_str();
        strc80 += str16a0.c_str();
        pt20.AddPair("stack", strc80.c_str());
    }
    {
        StackString<256> str18c0;
        AppendCheatsLog(str18c0);
        if (!str18c0.empty()) {
            pt20.AddPair("history", str18c0.c_str());
        }
    }
    if (unk38) {
        unk38(t, pt20);
    }
    String json;
    pt20.ToJSON(json);
    pt40.AddPair("data", json);
}
