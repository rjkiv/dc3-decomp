#include "utl/MakeString.h"
#include "obj/Data.h"
#include "os/Debug.h"
#include "os/System.h"
#include "os/CritSec.h"
#include "os/OSFuncs.h"
#include "utl/MemMgr.h"
#include "utl/Str.h"
#include "xdk/XAPILIB.h"
#include "xdk/xapilibi/handleapi.h"
#include "xdk/xapilibi/processthreadsapi.h"

#define MAX_BUF_THREADS 6
#define MAX_BUF_SIZE 0x1000

bool bufExceeded = false;
static CriticalSection *gLock = nullptr;
static char ***gBuf = nullptr;
static int gNum[MAX_BUF_THREADS] = { 0 };
static int gThreadIds[MAX_BUF_THREADS] = { -1 };
static int gCurThread = 0;
static int gNumThreads = 0;

void InitMakeString() {
    if (!gLock) {
        gLock = new CriticalSection();
        gBuf = (char ***)MemAlloc(0x18, __FILE__, 0x93, "MakeString Buffer", 0);
        for (int i = 0; i < 6; i++) {
            gBuf[i] = (char **)MemAlloc(0x40, __FILE__, 0x97, "MakeString Buffer", 0);
            for (int j = 0; j < 16; j++) {
                gBuf[i][j] =
                    (char *)MemAlloc(0x1000, __FILE__, 0x9B, "MakeString Buffer", 0);
            }
        }
    }
}

bool ValidateThreadId(DWORD id) {
    HANDLE hThread = OpenThread(0x40, false, id);
    if (!hThread) {
        return false;
    } else {
        DWORD exitCode;
        GetExitCodeThread(hThread, &exitCode);
        CloseHandle(hThread);
        return exitCode == 0x103;
    }
}

char *NextBuf() {
    if (!gLock) {
        InitMakeString();
        MILO_LOG("MakeString before InitMakeString!\n");
        MILO_FAIL("MakeString before InitMakeString!");
    }
    CritSecTracker cst(gLock);
    if (gNumThreads == 0) {
        gThreadIds[0] = GetCurrentThreadId();
        gNumThreads = 1;
    } else {
        int tID;
        DWORD curThread = GetCurrentThreadId();
        if (gThreadIds[gCurThread] != curThread) {
            for (tID = 0; tID < gNumThreads; tID++) {
                DWORD curID = GetCurrentThreadId();
                if (gThreadIds[tID] == curID) {
                    break;
                }
            }
            if (tID == gNumThreads) {
                for (tID = 0; tID < gNumThreads; tID++) {
                    if (!ValidateThreadId(gThreadIds[tID])) {
                        gThreadIds[tID] = GetCurrentThreadId();
                        break;
                    }
                }
                if (tID == gNumThreads) {
                    char msg[256];
                    if (gNumThreads >= 6) {
                        Hx_snprintf(
                            msg,
                            256,
                            "Too many threads using MakeString; maybe increase MAX_BUF_THREADS in %s?",
                            __FILE__
                        );
                        TheDebugFailer << msg;
                        return nullptr;
                    }
                    gThreadIds[tID] = GetCurrentThreadId();
                    gNumThreads++;
                }
            }
            gCurThread = tID;
        }
    }

    char *buf = gBuf[gCurThread][gNum[gCurThread]];
    if (++gNum[gCurThread] == 0x10) {
        gNum[gCurThread] = 0;
    }

    return buf;
}

FormatString::FormatString()
    : mBuf(NextBuf()), mBufSize(MAX_BUF_SIZE), mFmtEnd(nullptr), mType(kNone) {}

FormatString &FormatString::operator<<(int i) {
    if (mType != kInt) {
        // for whatever reason, this has the FormatString expanded out
        FormatString str("FormatString: '%s' doesn't start with kInt.  Format: '%s'");
        str << mFmt << mFmtBuf;
        TheDebugNotifier << str.Str();
    }
    char tmp = *mFmtEnd;
    *mFmtEnd = '\0';
    int n = Hx_snprintf(mBuf + MAX_BUF_SIZE - mBufSize, mBufSize, mFmt, i);
    *mFmtEnd = tmp;
    if (n < 0 && !bufExceeded) {
        bufExceeded = true;
        MILO_NOTIFY("MakeString() buffer size %d exceeded", MAX_BUF_SIZE);
    }
    mBufSize -= n;
    UpdateType();
    return *this;
}

const char *FormatString::Str() {
    if (mType != kNone) {
        // for whatever reason, this has the FormatString expanded out
        FormatString str("FormatString: '%s' doesn't start with kNone.  Format: '%s'");
        str << mFmt << mFmtBuf;
        TheDebugNotifier << str.Str();
    }
    if (*mFmt != '\0') {
        MILO_ASSERT(mFmtEnd - mFmt < mBufSize, 0x16F);
        strcpy(mBuf + MAX_BUF_SIZE - mBufSize, mFmt);
    }
    return mBuf;
}

FormatString &FormatString::operator<<(const char *cc) {
    if (mType != kStr)
        MILO_NOTIFY(
            "FormatString: '%s' doesn't start with kStr.  Format: '%s'", mFmt, mFmtBuf
        );
    if (cc >= mBuf && cc < mBuf + sizeof(mFmtBuf)) {
        FormatString str("FormatString: arg in buffer");
        TheDebugFailer << str.Str();
    }
    char tmp = *mFmtEnd;
    *mFmtEnd = '\0';
    int n = Hx_snprintf(mBuf + sizeof(mFmtBuf) - mBufSize, mBufSize, mFmt, cc);
    *mFmtEnd = tmp;
    if (n < 0 && !bufExceeded) {
        bufExceeded = true;
        MILO_NOTIFY("MakeString() buffer size %d exceeded", MAX_BUF_SIZE);
    }
    mBufSize -= n;
    UpdateType();
    return *this;
}

void FormatString::UpdateType() {
    if (mFmtEnd != 0) {
        mFmt = mFmtEnd;
        mFmtEnd = 0;
    }

    int val = 0;
    char *ptr = mFmt;
    for (; *ptr != '\0'; ptr++) {
        if (*ptr == '%') {
            if (ptr[1] == '%')
                ptr++; // '%' escape
            else if (val != 0)
                break;
            else
                val = 1;
        } else if (val == 1) {
            if (*ptr < 'A' || *ptr > 'z')
                continue;
            if (*ptr == 'f' || *ptr == 'g') {
                mType = kFloat;
            } else if (*ptr == 's') {
                mType = kStr;
            } else {
                mType = kInt;
            }
            val = 2;
        }
    }
    if (val == 1) {
        MILO_NOTIFY("FormatString: unable to identify %s", mFmt);
    }
    if (val == 0)
        mType = kNone;
    mFmtEnd = ptr;
}

FormatString &FormatString::operator<<(void *v) {
    if (mType != kInt)
        MILO_NOTIFY(
            "FormatString: '%s' doesn't start with kInt.  Format: '%s'", mFmt, mFmtBuf
        );
    char tmp = *mFmtEnd;
    *mFmtEnd = '\0';
    int n = Hx_snprintf(mBuf + MAX_BUF_SIZE - mBufSize, mBufSize, mFmt, v);
    *mFmtEnd = tmp;
    if (n < 0 && !bufExceeded) {
        bufExceeded = true;
        MILO_NOTIFY("MakeString() buffer size %d exceeded", MAX_BUF_SIZE);
    }
    mBufSize -= n;
    UpdateType();
    return *this;
}

FormatString &FormatString::operator<<(unsigned int ui) {
    if (mType != kInt)
        MILO_NOTIFY(
            "FormatString: '%s' doesn't start with kInt.  Format: '%s'", mFmt, mFmtBuf
        );
    char tmp = *mFmtEnd;
    *mFmtEnd = '\0';
    int n = Hx_snprintf(mBuf + MAX_BUF_SIZE - mBufSize, mBufSize, mFmt, ui);
    *mFmtEnd = tmp;
    if (n < 0 && !bufExceeded) {
        bufExceeded = true;
        MILO_NOTIFY("MakeString() buffer size %d exceeded", MAX_BUF_SIZE);
    }
    mBufSize -= n;
    UpdateType();
    return *this;
}

FormatString &FormatString::operator<<(long l) {
    if (mType != kInt)
        MILO_NOTIFY(
            "FormatString: '%s' doesn't start with kInt.  Format: '%s'", mFmt, mFmtBuf
        );
    char tmp = *mFmtEnd;
    *mFmtEnd = '\0';
    int n = Hx_snprintf(mBuf + MAX_BUF_SIZE - mBufSize, mBufSize, mFmt, l);
    *mFmtEnd = tmp;
    if (n < 0 && !bufExceeded) {
        bufExceeded = true;
        MILO_NOTIFY("MakeString() buffer size %d exceeded", MAX_BUF_SIZE);
    }
    mBufSize -= n;
    UpdateType();
    return *this;
}

FormatString &FormatString::operator<<(unsigned long ul) {
    if (mType != kInt)
        MILO_NOTIFY(
            "FormatString: '%s' doesn't start with kInt.  Format: '%s'", mFmt, mFmtBuf
        );
    char tmp = *mFmtEnd;
    *mFmtEnd = '\0';
    int n = Hx_snprintf(mBuf + MAX_BUF_SIZE - mBufSize, mBufSize, mFmt, ul);
    *mFmtEnd = tmp;
    if (n < 0 && !bufExceeded) {
        bufExceeded = true;
        MILO_NOTIFY("MakeString() buffer size %d exceeded", MAX_BUF_SIZE);
    }
    mBufSize -= n;
    UpdateType();
    return *this;
}

FormatString &FormatString::operator<<(long long ll) {
    if (mType != kInt)
        MILO_NOTIFY(
            "FormatString: '%s' doesn't start with kInt.  Format: '%s'", mFmt, mFmtBuf
        );
    char tmp = *mFmtEnd;
    *mFmtEnd = '\0';
    int n = Hx_snprintf(mBuf + MAX_BUF_SIZE - mBufSize, mBufSize, mFmt, ll);
    *mFmtEnd = tmp;
    if (n < 0 && !bufExceeded) {
        bufExceeded = true;
        MILO_NOTIFY("MakeString() buffer size %d exceeded", MAX_BUF_SIZE);
    }
    mBufSize -= n;
    UpdateType();
    return *this;
}

FormatString &FormatString::operator<<(unsigned long long ull) {
    if (mType != kInt)
        MILO_NOTIFY(
            "FormatString: '%s' doesn't start with kInt.  Format: '%s'", mFmt, mFmtBuf
        );
    char tmp = *mFmtEnd;
    *mFmtEnd = '\0';
    int n = Hx_snprintf(mBuf + MAX_BUF_SIZE - mBufSize, mBufSize, mFmt, ull);
    *mFmtEnd = tmp;
    if (n < 0 && !bufExceeded) {
        bufExceeded = true;
        MILO_NOTIFY("MakeString() buffer size %d exceeded", MAX_BUF_SIZE);
    }
    mBufSize -= n;
    UpdateType();
    return *this;
}

FormatString &FormatString::operator<<(float f) {
    if (mType != kFloat)
        MILO_NOTIFY(
            "FormatString: '%s' doesn't start with kFloat.  Format: '%s'", mFmt, mFmtBuf
        );
    char tmp = *mFmtEnd;
    *mFmtEnd = '\0';
    int n = Hx_snprintf(mBuf + MAX_BUF_SIZE - mBufSize, mBufSize, mFmt, f);
    *mFmtEnd = tmp;
    if (n < 0 && !bufExceeded) {
        bufExceeded = true;
        MILO_NOTIFY("MakeString() buffer size %d exceeded", MAX_BUF_SIZE);
    }
    mBufSize -= n;
    UpdateType();
    return *this;
}

FormatString &FormatString::operator<<(double f) {
    if (mType != kFloat)
        MILO_NOTIFY(
            "FormatString: '%s' doesn't start with kFloat.  Format: '%s'", mFmt, mFmtBuf
        );
    char tmp = *mFmtEnd;
    *mFmtEnd = '\0';
    int n = Hx_snprintf(mBuf + MAX_BUF_SIZE - mBufSize, mBufSize, mFmt, f);
    *mFmtEnd = tmp;
    if (n < 0 && !bufExceeded) {
        bufExceeded = true;
        MILO_NOTIFY("MakeString() buffer size %d exceeded", MAX_BUF_SIZE);
    }
    mBufSize -= n;
    UpdateType();
    return *this;
}

FormatString &FormatString::operator<<(const String &str) {
    if (mType != kStr)
        MILO_NOTIFY(
            "FormatString: '%s' doesn't start with kStr.  Format: '%s'", mFmt, mFmtBuf
        );
    char tmp = *mFmtEnd;
    *mFmtEnd = '\0';
    int n = Hx_snprintf(mBuf + MAX_BUF_SIZE - mBufSize, mBufSize, mFmt, str.c_str());
    *mFmtEnd = tmp;
    if (n < 0 && !bufExceeded) {
        bufExceeded = true;
        MILO_NOTIFY("MakeString() buffer size %d exceeded", MAX_BUF_SIZE);
    }
    mBufSize -= n;
    UpdateType();
    return *this;
}

FormatString &FormatString::operator<<(const FixedString &str) {
    if (mType != kStr)
        MILO_NOTIFY(
            "FormatString: '%s' doesn't start with kStr.  Format: '%s'", mFmt, mFmtBuf
        );
    char tmp = *mFmtEnd;
    *mFmtEnd = '\0';
    int n = Hx_snprintf(mBuf + MAX_BUF_SIZE - mBufSize, mBufSize, mFmt, str.c_str());
    *mFmtEnd = tmp;
    if (n < 0 && !bufExceeded) {
        bufExceeded = true;
        MILO_NOTIFY("MakeString() buffer size %d exceeded", MAX_BUF_SIZE);
    }
    mBufSize -= n;
    UpdateType();
    return *this;
}

FormatString &FormatString::operator<<(Symbol s) {
    if (mType != kStr)
        MILO_NOTIFY(
            "FormatString: '%s' doesn't start with kStr.  Format: '%s'", mFmt, mFmtBuf
        );
    char tmp = *mFmtEnd;
    *mFmtEnd = '\0';
    int n = Hx_snprintf(mBuf + MAX_BUF_SIZE - mBufSize, mBufSize, mFmt, s.Str());
    *mFmtEnd = tmp;
    if (n < 0 && !bufExceeded) {
        bufExceeded = true;
        MILO_NOTIFY("MakeString() buffer size %d exceeded", MAX_BUF_SIZE);
    }
    mBufSize -= n;
    UpdateType();
    return *this;
}

void FormatString::InitializeWithFmt(const char *fmt, bool b2) {
    MILO_ASSERT(fmt, 0xC2);
    if (!StrNCopy(mFmtBuf, fmt, MAX_BUF_SIZE)) {
        MILO_NOTIFY("Increase mFmtBuf size to %d", strlen(fmt));
    }
    mFmt = mFmtBuf;
    if (b2)
        UpdateType();
}

FormatString::FormatString(const char *str)
    : mBuf(NextBuf()), mBufSize(MAX_BUF_SIZE), mFmtEnd(nullptr), mType(kNone) {
    InitializeWithFmt(str, true);
}

FormatString &FormatString::operator<<(const DataNode &node) {
    char tmp = *mFmtEnd;
    *mFmtEnd = '\0';

    int n;
    if (mType == kInt && node.Type() == kDataFloat) {
        n = Hx_snprintf(
            mBuf + MAX_BUF_SIZE - mBufSize, mBufSize, mFmt, (int)node.LiteralFloat()
        );
    } else if (mType == kInt) {
        n = Hx_snprintf(mBuf + MAX_BUF_SIZE - mBufSize, mBufSize, mFmt, node.LiteralInt());
    } else if (mType == kFloat) {
        n = Hx_snprintf(
            mBuf + MAX_BUF_SIZE - mBufSize, mBufSize, mFmt, node.LiteralFloat()
        );
    } else if (mType == kStr) {
        n = Hx_snprintf(mBuf + MAX_BUF_SIZE - mBufSize, mBufSize, mFmt, node.LiteralStr());
    } else {
        MILO_NOTIFY("FormatString: Couldn't convert DataNode to '%s'", mFmt);
        n = 0;
    }

    *mFmtEnd = tmp;
    MILO_ASSERT(n >= 0, 0x13E);
    mBufSize -= n;
    UpdateType();
    return *this;
}
