#include "AsyncTask.h"
#include "HolmesClient.h"
#include "os/ArkFile_p.h"
#include "os/Archive.h"
#include "os/Block.h"
#include "os/Debug.h"
#include "os/File.h"
#include "os/System.h"
#include "utl/Loader.h"
#include "utl/Str.h"

int ArkFile::Read(void *iBuff, int iBytes) {
    if (ReadAsync(iBuff, iBytes) == 0)
        return 0;
    int ret = -1;
    while (ReadDone(ret) == 0)
        ;
    return ret;
}

int ArkFile::Seek(int iOffset, int iMode) {
    switch (iMode) {
    case 0:
        mTell = iOffset;
        break;
    case 1:
        mTell += iOffset;
        break;
    case 2:
        mTell = mSize + iOffset;
        break;
    default:
        break;
    }
    return mTell;
}

void ArkFile::TaskDone(int iBytes) {
    mNumOutstandingTasks--;
    mBytesRead += iBytes;
    mTell += iBytes;
}

bool ArkFile::ReadDone(int &oBytes) {
    TheBlockMgr.Poll();
    oBytes = mBytesRead;
    return mNumOutstandingTasks == 0;
}

ArkFile::ArkFile(const char *iFilename, int iMode)
    : mArkfileNum(0), mByteStart(0), mReadStartTime(0), mSize(0), mUCSize(0),
      mNumOutstandingTasks(0), mBytesRead(0), mTell(0), mFail(false), mReadAhead(true),
      mFilename(iFilename) {
    if (!TheArchive->GetFileInfo(
            FileMakePath(".", iFilename), mArkfileNum, mByteStart, mSize, mUCSize
        )
        || (iMode & 1)) {
        mFail = true;
    }
}

ArkFile::~ArkFile() {
    if (mNumOutstandingTasks > 0) {
        TheBlockMgr.KillBlockRequests(this);
    }
}

int ArkFile::Write(const void *iBuff, int iBytes) {
    MILO_FAIL("ERROR: Cannot write to a file in an archive!");
    return 0;
}

bool ArkFile::ReadAsync(void *iData, int iBytes) {
    MILO_ASSERT(iBytes >= 0, 0x5D);
    if (mTell == mSize || mNumOutstandingTasks != 0)
        return false;
    else {
        mBytesRead = 0;
        if (iBytes == 0)
            return true;
        if (mReadAhead) {
            unsigned int last = mFilename.find_last_of('_');
            bool b6 = last != String::npos
                && PlatformSymbol(TheLoadMgr.GetPlatform())
                    == mFilename.c_str() + last + 1;
            String filename(b6 ? mFilename.substr(0, last) : mFilename);
            TheArchive->HasArchivePermission(mArkfileNum);
            if (Archive::DebugArkOrder() != 0) {
                MILO_ASSERT(filename.length() < 255, 0x7D);
                char buf[256];
                SearchReplace(filename.c_str(), "/eng/", "/*/", buf);
                MILO_LOG("ArkFile%d:   '%s'\n", mArkfileNum, buf);
                HolmesClientPrint(filename.c_str());
            }
        }
        mReadAhead = false;
        if (mTell + iBytes > mSize) {
            iBytes = mSize - mTell;
        }
        MILO_ASSERT(iBytes >= 0, 0x8B);
        int ib8 = 0;
        int ib4 = 0;
        int ib0 = 0;
        u64 curPos = mTell + mByteStart;
        TheBlockMgr.GetAssociatedBlocks(curPos, iBytes, ib8, ib4, ib0);
        bool first = true;
        int idiff = ib4 + ib8 - 1;
        for (int i = ib8; i <= idiff; i++) {
            int ivar8;
            if (i == ib8) {
                ivar8 = curPos % ib0;
            } else {
                ivar8 = 0;
            }
            int ivar12 = ib0;
            if (i == idiff && iBytes != 0) {
                int mod = (curPos + iBytes) % ib0;
                if (mod != 0) {
                    ivar12 = mod;
                }
            }
            AsyncTask task(this, iData, mArkfileNum, i, ivar8, ivar12, mFilename.c_str());
            iData = (void *)((int)iData - ivar8 + ivar12);
            mNumOutstandingTasks++;
            if (!first || !task.FillData()) {
                first = false;
                TheBlockMgr.AddTask(task);
            }
        }
    }
    TheBlockMgr.Poll();
    return true;
}
