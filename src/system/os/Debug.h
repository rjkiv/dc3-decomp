#pragma once
#include "utl/TextStream.h"
#include "utl/TextFileStream.h"
#include <list>
#include <string.h>
#include "utl/Std.h"

typedef void ExitCallbackFunc(void);
typedef void FixedStringFunc(FixedString &);

struct StackData {
    unsigned int address; // 0x0
};

// size 0x134
class Debug : public TextStream {
public:
    enum ModalType {
        kModalWarn = 0,
        kModalNotify = 1,
        kModalFail = 2
    };

    typedef void ModalCallbackFunc(ModalType &, FixedString &, bool);
    typedef void DataPointFunc(ModalType, class DataPoint &);

    Debug();
    virtual ~Debug();
    virtual void Print(const char *msg);

    void Poll();
    void SetDisabled(bool b);
    void SetTry(bool b);
    void DoCrucible(ModalType mt, const char *msg, void *context);
    //   public: void AddFailCallback(void (*)());
    //   public: void RemoveFailCallback(void (*)());
    void AddExitCallback(ExitCallbackFunc *func) { mExitCallbacks.push_front(func); }
    void RemoveExitCallback(ExitCallbackFunc *func);
    void AddFailAppendCallback(FixedStringFunc *func) {
        mFailAppendCallbacks.push_front(func);
    }
    //   public: void RemoveFailAppendCallback(void (*)(FixedString&));
    ModalCallbackFunc *GetModalCallback() const { return mModalCallback; }
    ModalCallbackFunc *SetModalCallback(ModalCallbackFunc *func);

    void SetErrorCallback(DataPointFunc *func) { mErrorCallback = func; }

    bool NoModal() const { return mNoModal; }
    void SetNoModal(bool nomodal) { mNoModal = nomodal; }

    void StartLog(const char *file, bool always_flush);
    void StopLog();
    void Init();
    void Exit(int err, bool call_exit);
    void Warn(const char *msg);
    void Notify(const char *msg);
    void Fail(const char *msg, void *context);
    TextStream *SetReflect(TextStream *ts) {
        TextStream *ret = mReflect;
        mReflect = ts;
        return ret;
    }

    static const int STACK_MAX = 50;

private:
    void Modal(ModalType &mt, const char *msg, void *context);

    /** Whether or not to enable debug output. */
    volatile bool mNoDebug; // 0x4
    /** Whether or not we are in failure handling code. (aka the red screen of death) */
    bool mFailing; // 0x5
    /** Whether or not the app is currently exiting. */
    bool mExiting; // 0x6
    /** Whether or not we are allowed to enter try blocks in the code. */
    bool mNoTry; // 0x7
    bool mNoModal; // 0x8
    /** How many try blocks we are currently in. */
    int mTry; // 0xc
    TextFileStream *mLog; // 0x10
    bool mAlwaysFlush; // 0x14
    TextStream *mReflect; // 0x18
    ModalCallbackFunc *mModalCallback; // 0x1c
    std::list<ExitCallbackFunc *> mFailCallbacks; // 0x20
    std::list<ExitCallbackFunc *> mExitCallbacks; // 0x28
    std::list<FixedStringFunc *> mFailAppendCallbacks; // 0x30
    DataPointFunc *mErrorCallback; // 0x38
    /** Addresses in memory corresponding to called functions. */
    StackData mFailThreadStack[50]; // 0x3c
    const char *mFailThreadMsg; // 0x104
    const char *mNotifyThreadMsg; // 0x108
    const char *mCrucibleHostname; // 0x10c
    const char *mCrucibleApp; // 0x110
    String mCrucibleProject; // 0x114
    String mSDKVersion; // 0x11c
    String mWorkingDir; // 0x124
    String mUser; // 0x12c
};

typedef void ModalCallbackFunc(Debug::ModalType &, FixedString &, bool);

#include "utl/Str.h"
#include "utl/MakeString.h"
#include <list>

extern Debug TheDebug;
extern const char *kAssertStr;

#define MILO_ASSERT(cond, line)                                                          \
    do {                                                                                 \
        if (!(cond)) {                                                                   \
            TheDebugFailer << MakeString(kAssertStr, __FILE__, line, #cond);             \
        }                                                                                \
    } while (0)

#define MILO_ASSERT_FMT(cond, ...)                                                       \
    do {                                                                                 \
        if (!(cond)) {                                                                   \
            TheDebugFailer << MakeString(__VA_ARGS__);                                   \
        }                                                                                \
    } while (0)

#define MILO_FAIL(...) TheDebugFailer << MakeString(__VA_ARGS__)
#define MILO_WARN(...) TheDebugWarner << MakeString(__VA_ARGS__)
#define MILO_NOTIFY(...) TheDebugNotifier << MakeString(__VA_ARGS__)
#define MILO_NOTIFY_BETA(...) DebugBeta() << MakeString(__VA_ARGS__)
#define MILO_LOG(...) TheDebug << MakeString(__VA_ARGS__)

// Usage:
// MILO_TRY {
//     // The code to try
// } MILO_CATCH(errMsg) {
//     // Use errMsg here, e.g.:
//     MILO_NOTIFY("An unexpected thing happened: %s", errMsg);
// }
#define MILO_TRY                                                                         \
    TheDebug.SetTry(true);                                                               \
    try {
#define MILO_CATCH(name)                                                                 \
    TheDebug.SetTry(false);                                                              \
    }                                                                                    \
    catch (const char *name)

// (min) <= (value) && (value) < (max)
#define MILO_ASSERT_RANGE(value, min, max, line)                                         \
    MILO_ASSERT((min) <= (value) && (value) < (max), line)

// (min) <= (value) && (value) <= (max)
#define MILO_ASSERT_RANGE_EQ(value, min, max, line)                                      \
    MILO_ASSERT((min) <= (value) && (value) <= (max), line)

class DebugWarner {
public:
    void operator<<(const char *c) { TheDebug.Warn(c); }
};

extern DebugWarner TheDebugWarner;

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

class DebugNotifyOncePrinter {
    char mStr[256];

public:
    void operator<<(const char *cc) {
        if (strcmp(mStr, cc)) {
            strncpy(mStr, cc, 0xFF);
            TheDebug.Print(cc);
        }
    }
};

extern DebugNotifyOncePrinter TheDebugNotifyOncePrinter;

#define MILO_PRINT_ONCE(...) TheDebugNotifyOncePrinter << MakeString(__VA_ARGS__)

namespace {
    bool AddToStrings(const char *name, std::list<String> &strings) {
        if (strings.size() > 16) {
            return false;
        }

        FOREACH (it, strings) {
            if (streq(it->c_str(), name)) {
                return false;
            }
        }

        strings.push_back(name);
        return true;
    }
}

class DebugNotifyOncer {
private:
    std::list<String> mStrs;

public:
    DebugNotifyOncer() {}
    ~DebugNotifyOncer() {}

    void operator<<(const char *cc) {
        if (AddToStrings(cc, mStrs)) {
            TheDebugNotifier << cc;
        }
    }
};

#define MILO_NOTIFY_ONCE(...)                                                            \
    {                                                                                    \
        static DebugNotifyOncer _dw;                                                     \
        _dw << MakeString(__VA_ARGS__);                                                  \
    }

class DebugWarnOncer {
private:
    std::list<String> mStrs;

public:
    DebugWarnOncer() {}
    ~DebugWarnOncer() {}

    void operator<<(const char *cc) {
        if (AddToStrings(cc, mStrs)) {
            TheDebugWarner << cc;
        }
    }
};

#define MILO_WARN_ONCE(...)                                                              \
    {                                                                                    \
        static DebugWarnOncer _dw;                                                       \
        _dw << MakeString(__VA_ARGS__);                                                  \
    }
