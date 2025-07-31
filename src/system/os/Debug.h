#pragma once
#include "utl/TextStream.h"
#include "utl/TextFileStream.h"
#include "xdk/xapilibi/getcurrentthreadid.h"
#include <list>
#include <string.h>
// #include <setjmp.h>

typedef void ExitCallbackFunc(void);
typedef void FixedStringFunc(FixedString &);

class Debug : public TextStream {
public:
    enum ModalType {
    };

    typedef void ModalCallbackFunc(ModalType &, FixedString &, bool);

    bool mNoDebug; // 0x4
    bool mFailing; // 0x5
    bool mExiting; // 0x6
    bool mNoTry; // 0x7
    bool mNoModal; // 0x8
    int mTry; // 0xc
    TextFileStream *mLog; // 0x10
    bool mAlwaysFlush; // 0x14
    TextStream *mReflect; // 0x18
    ModalCallbackFunc *mModalCallback; // 0x1c
    std::list<ExitCallbackFunc *> mFailCallbacks; // 0x20
    std::list<ExitCallbackFunc *> mExitCallbacks; // 0x28
    std::list<FixedStringFunc *> unk30; // 0x30
    int unk38; // 0x38
    unsigned int mFailThreadStack[50]; // starts at 0x3c
    const char *mFailThreadMsg; // 0x104
    const char *mNotifyThreadMsg; // 0x108
    int unk10c;
    int unk110;
    String unk114;
    String unk11c;
    String unk124;
    String unk12c;

    Debug();
    virtual ~Debug();
    virtual void Print(const char *);

    void Poll();
    void SetDisabled(bool);
    void SetTry(bool);
    void AddExitCallback(ExitCallbackFunc *func) { mExitCallbacks.push_front(func); }
    void RemoveExitCallback(ExitCallbackFunc *);
    void StartLog(const char *, bool);
    void StopLog();
    void Init();
    ModalCallbackFunc *SetModalCallback(ModalCallbackFunc *);
    void Exit(int, bool);
    void Modal(bool &, const char *);

    void Notify(const char *msg);
    void Fail(const char *msg, void *);
    TextStream *SetReflect(TextStream *ts) {
        TextStream *ret = mReflect;
        mReflect = ts;
        return ret;
    }
};

#include "utl/Str.h"
#include "utl/MakeString.h"

extern Debug TheDebug;
extern const char *kAssertStr;

#define MILO_ASSERT(cond, line)                                                          \
    ((cond) || (TheDebugFailer << (MakeString(kAssertStr, __FILE__, line, #cond)), 0))
#define MILO_ASSERT_FMT(cond, ...)                                                       \
    ((cond) || (TheDebugFailer << (MakeString(__VA_ARGS__)), 0))
#define MILO_FAIL(...) TheDebugFailer << MakeString(__VA_ARGS__)
#define MILO_WARN(...) TheDebugNotifier << MakeString(__VA_ARGS__)
#define MILO_NOTIFY_BETA(...) DebugBeta() << MakeString(__VA_ARGS__)
#define MILO_LOG(...) TheDebug << MakeString(__VA_ARGS__)

class DebugNotifier {
public:
    void operator<<(const char *c) { TheDebug.Notify(c); }
};

extern DebugNotifier TheDebugNotifier;

class DebugFailer {
public:
    void operator<<(const char *cc) { TheDebug.Fail(cc, nullptr); }
};

extern DebugFailer TheDebugFailer;

extern u32 gMainThreadID;

inline bool MainThread() {
    if (gMainThreadID == -1)
        return true;
    if (GetCurrentThreadId() == gMainThreadID)
        return true;
    return false;
}
