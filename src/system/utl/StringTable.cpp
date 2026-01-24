#include "utl/StringTable.h"
#include "math/Utl.h"
#include "os/Debug.h"
#include "utl/Loader.h"
#include "utl/MemMgr.h"

int StringTable::UsedSize() const {
    int size = 0;
    for (int i = 0; i < mBuffers.size(); i++) {
        const Buf &buf = mBuffers[i];
        if (i == mCurBuf) {
            size += (int)(mCurChar - buf.chars);
            break;
        }
        size += buf.size;
    }
    return size;
}

int StringTable::Size() const {
    int size = 0;
    for (int i = 0; i < mBuffers.size(); i++) {
        size += mBuffers[i].size;
    }
    return size;
}

StringTable::~StringTable() {
    for (int i = 0; i < mBuffers.size(); i++) {
        MemFree(mBuffers[i].chars);
    }
}

StringTable::StringTable(int i) : mCurChar(0), mCurBuf(-1) {
    if (i != 0)
        AddBuf(i);
}

void StringTable::Reserve(int i) {
    int size = Size();
    if (size < i) {
        if (mCurBuf < 0) {
            AddBuf(i);
        } else {
            Buf buf;
            buf.size = (i - size) + 0x40;
            MemTemp tmp;
            buf.chars = (char *)MemAlloc(buf.size, __FILE__, 0x38, "string table");
            mBuffers.push_back(buf);
        }
    }
}

void StringTable::AddBuf(int i) {
    Buf buf;
    buf.size = i;
    MemTemp tmp;
    buf.chars = (char *)MemAlloc(i, __FILE__, 0x1D, "string table");
    mCurChar = buf.chars;
    mCurBuf = mBuffers.size();
    mBuffers.push_back(buf);
}

const char *StringTable::Add(const char *str) {
    int len = strlen(str) + 1;
    if (mCurBuf == -1) {
        AddBuf(Max(len, 0x100));
    } else {
        if (len + ((int)mCurChar - (int)mBuffers[mCurBuf].chars)
            > mBuffers[mCurBuf].size) {
            bool b4 = false;
            for (; mCurBuf < mBuffers.size() - 1;) {
                Buf &curBuf = mBuffers[++mCurBuf];
                mCurChar = curBuf.chars;
                if (curBuf.size >= len) {
                    b4 = true;
                    break;
                }
                MILO_WARN("Wasted string table (%d) adding %s\n", curBuf.size, str);
            }
            if (!b4) {
                int newSize = Max(Size(), len);
                AddBuf(newSize);
                if (!TheLoadMgr.EditMode()) {
                    MILO_WARN("Resizing string table (%d) adding %s", newSize * 2, str);
                }
            }
            MILO_ASSERT(mBuffers[mCurBuf].size >= len, 0x68);
        }
    }
    memcpy(mCurChar, str, len);
    str = mCurChar;
    mCurChar += len;
    return str;
}
