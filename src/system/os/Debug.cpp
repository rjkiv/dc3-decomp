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
      mAlwaysFlush(0), mReflect(0), mModalCallback(DebugModal), mDataPointCallback(0),
      mFailThreadMsg(0), mNotifyThreadMsg(0), mHostname(0), mApp(0) {}

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

#define CHECK_CODE(code)                                                                 \
    case code:                                                                           \
        return #code;

const char *GetExpCode(int code) {
    switch (code) {
        CHECK_CODE(EXCEPTION_GUARD_PAGE);
        CHECK_CODE(EXCEPTION_DATATYPE_MISALIGNMENT);
        CHECK_CODE(EXCEPTION_BREAKPOINT);
        CHECK_CODE(EXCEPTION_SINGLE_STEP);
        CHECK_CODE(EXCEPTION_ACCESS_VIOLATION);
        CHECK_CODE(EXCEPTION_IN_PAGE_ERROR);
        CHECK_CODE(EXCEPTION_INVALID_HANDLE);
        CHECK_CODE(EXCEPTION_ILLEGAL_INSTRUCTION);
        CHECK_CODE(EXCEPTION_NONCONTINUABLE_EXCEPTION);
        CHECK_CODE(EXCEPTION_INVALID_DISPOSITION);
        CHECK_CODE(EXCEPTION_ARRAY_BOUNDS_EXCEEDED);
        CHECK_CODE(EXCEPTION_FLT_DENORMAL_OPERAND);
        CHECK_CODE(EXCEPTION_FLT_DIVIDE_BY_ZERO);
        CHECK_CODE(EXCEPTION_FLT_INEXACT_RESULT);
        CHECK_CODE(EXCEPTION_FLT_INVALID_OPERATION);
        CHECK_CODE(EXCEPTION_FLT_OVERFLOW);
        CHECK_CODE(EXCEPTION_FLT_STACK_CHECK);
        CHECK_CODE(EXCEPTION_FLT_UNDERFLOW);
        CHECK_CODE(EXCEPTION_INT_DIVIDE_BY_ZERO);
        CHECK_CODE(EXCEPTION_INT_OVERFLOW);
        CHECK_CODE(EXCEPTION_PRIV_INSTRUCTION);
        CHECK_CODE(EXCEPTION_STACK_OVERFLOW);
        CHECK_CODE(CONTROL_C_EXIT);

    default:
        return MakeString("Unhandled Exception %d", code);
    }
}

LONG HmxGlobalHandler(EXCEPTION_POINTERS *ptrs) {
    if (DmIsDebuggerPresent()) {
        return 1;
    } else {
        TheDebug.Fail(
            GetExpCode(ptrs->ExceptionRecord->ExceptionCode), ptrs->ContextRecord
        );
        return 0;
    }
}

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
        mSDK = MakeString("%d.%d", sysInfo.XDKVersion.Build, sysInfo.XDKVersion.Qfe);
    }
    mSource = NetworkSocket::GetHostName();
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
                mSDK
            );
            FOREACH (it, mFailAppendCallbacks) {
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
    if (!mHostname) {
        if (SystemConfig()) {
            DataArray *cfg = SystemConfig()->FindArray("crucible", false);
            if (cfg) {
                mHostname = cfg->FindStr("hostname");
                mApp = cfg->FindStr("app");
                mProject = cfg->FindStr("project");
            }
        }
        if (!mHostname) {
            mHostname = DevHostname("crucible");
        }
    }
    DataPoint dataPt;
    DataPoint dataPtJson;
    dataPt.AddPair("message", msg);
    const char *severityMsg;
    if (t == kModalFail) {
        severityMsg = "crash";
    } else if (t == kModalNotify) {
        severityMsg = "notify";
    } else {
        severityMsg = "warn";
    }
    dataPt.AddPair("severity", severityMsg);
    dataPt.AddPair("project", mProject.c_str());
    dataPt.AddPair("platform", PlatformSymbol(TheLoadMgr.GetPlatform()));
    dataPt.AddPair("source", mSource);
    {
        String sysCfgFile;
        String versionStr;
        if (SystemConfig()) {
            sysCfgFile = SystemConfig()->File();
            SystemConfig()->FindData("version", versionStr, false);
        } else {
            sysCfgFile = "<unknown>";
        }
        dataPtJson.AddPair("config_name", sysCfgFile);
        dataPt.AddPair("version", versionStr);
    }
    dataPtJson.AddPair("uptime", SystemMs());
    {
        StackString<256> pathStr(TheSystemArgs.empty() ? "" : TheSystemArgs.front());
        StackString<256> appStr(pathStr.c_str());
        appStr = FileGetBase(appStr.c_str());
        if (appStr.length() > 3) {
            if (appStr[appStr.length() - 2] == '_') {
                appStr[appStr.length() - 2] = '\0';
            }
        }
        pathStr.ReplaceAll('\\', '/');
        dataPtJson.AddPair("path", pathStr.c_str());
        if (mApp) {
            dataPt.AddPair("application", mApp);
        } else {
            dataPt.AddPair("application", appStr.c_str());
        }
    }
    {
        StackString<256> argsStr;
        for (int i = 0; i < TheSystemArgs.size(); i++) {
            StackString<256> curArg(TheSystemArgs[i]);
            curArg.ReplaceAll('\\', '/');
            argsStr += curArg.c_str();
            argsStr += "\r\n";
        }
        dataPtJson.AddPair("args", argsStr.c_str());
    }
    dataPtJson.AddPair("opsys", mSDK);
    dataPt.AddPair("extra", "");
    if (t == kModalFail) {
        StackString<512> dataStackTrace;
        DataAppendStackTrace(dataStackTrace);
        StackString<2048> cStackTrace;
        AppendStackTrace(cStackTrace, v);
        StackString<3096> stackOutput;
        stackOutput += "\r\n";
        stackOutput += cStackTrace.c_str();
        stackOutput += dataStackTrace.c_str();
        dataPtJson.AddPair("stack", stackOutput.c_str());
    }
    {
        StackString<256> cheatStr;
        AppendCheatsLog(cheatStr);
        if (!cheatStr.empty()) {
            dataPtJson.AddPair("history", cheatStr.c_str());
        }
    }
    if (mDataPointCallback) {
        mDataPointCallback(t, dataPtJson);
    }
    String json;
    dataPtJson.ToJSON(json);
    dataPt.AddPair("data", json);
}
