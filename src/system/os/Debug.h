#pragma once
#include "utl/TextStream.h"
#include "utl/TextFileStream.h"
#include <list>
#include <string.h>

typedef void ExitCallbackFunc(void);
typedef void FixedStringFunc(FixedString &);

// size 0x134
class Debug : public TextStream {
public:
    enum ModalType {
        kModalWarn = 0,
        kModalNotify = 1,
        kModalFail = 2
    };

    typedef void ModalCallbackFunc(ModalType &, FixedString &, bool);

private:
    void Modal(ModalType &, const char *, void *);

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
    // 0x3c is a struct, StackData
    unsigned int mFailThreadStack[50]; // starts at 0x3c
    const char *mFailThreadMsg; // 0x104
    const char *mNotifyThreadMsg; // 0x108
    int unk10c;
    int unk110;
    String unk114;
    String unk11c;
    String unk124;
    String unk12c;

public:
    Debug();
    virtual ~Debug();
    virtual void Print(const char *);

    void Poll();
    void SetDisabled(bool);
    void SetTry(bool);
    void AddExitCallback(ExitCallbackFunc *func) { mExitCallbacks.push_front(func); }
    void RemoveExitCallback(ExitCallbackFunc *);
    bool CheckModalCallback(ModalCallbackFunc *func) { return mModalCallback == func; }
    ModalCallbackFunc *ModalCallback() const { return mModalCallback; }
    bool NoModal() const { return mNoModal; }
    void SetNoModal(bool nomodal) { mNoModal = nomodal; }

    void StartLog(const char *, bool);
    void StopLog();
    void Init();
    ModalCallbackFunc *SetModalCallback(ModalCallbackFunc *);
    void Exit(int, bool);
    void Warn(const char *msg);
    void Notify(const char *msg);
    void Fail(const char *msg, void *);
    TextStream *Reflect() const { return mReflect; }
    TextStream *SetReflect(TextStream *ts) {
        TextStream *ret = mReflect;
        mReflect = ts;
        return ret;
    }
};

typedef void ModalCallbackFunc(Debug::ModalType &, FixedString &, bool);

#include "utl/Str.h"
#include "utl/MakeString.h"
#include <list>

extern Debug TheDebug;
extern const char *kAssertStr;

// #define MILO_ASSERT(cond, line) \
//     ((cond) || (TheDebugFailer << (MakeString(kAssertStr, __FILE__, line, #cond)), 0))

#define MILO_ASSERT(cond, line)                                                          \
    do {                                                                                 \
        if (!(cond)) {                                                                   \
            TheDebugFailer << MakeString(kAssertStr, __FILE__, line, #cond);             \
        }                                                                                \
    } while (0)

#define MILO_ASSERT_FMT(cond, ...)                                                       \
    ((cond) || (TheDebugFailer << (MakeString(__VA_ARGS__)), 0))
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
    try {                                                                                \
        TheDebug.SetTry(true);                                                           \
        do

#define MILO_CATCH(name)                                                                 \
    while (false)                                                                        \
        ;                                                                                \
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
    char msg[0x100];

public:
    void operator<<(const char *cc) {
        if (strcmp(msg, cc)) {
            strncpy(msg, cc, 0xFF);
            TheDebug.Print(cc);
        }
    }
};

extern DebugNotifyOncePrinter TheDebugNotifyOncePrinter;

#define MILO_PRINT_ONCE(...) TheDebugNotifyOncePrinter << MakeString(__VA_ARGS__)

namespace {
    bool AddToStrings(const char *name, std::list<String> &strings);
}

class DebugNotifyOncer {
private:
    std::list<String> mStrings;

public:
    DebugNotifyOncer() {}
    ~DebugNotifyOncer() {}

    void operator<<(const char *cc) {
        if (AddToStrings(cc, mStrings)) {
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
    std::list<String> mStrings;

public:
    DebugWarnOncer() {}
    ~DebugWarnOncer() {}

    void operator<<(const char *cc) {
        if (AddToStrings(cc, mStrings)) {
            TheDebugWarner << cc;
        }
    }
};

#define MILO_WARN_ONCE(...)                                                              \
    {                                                                                    \
        static DebugWarnOncer _dw;                                                       \
        _dw << MakeString(__VA_ARGS__);                                                  \
    }
