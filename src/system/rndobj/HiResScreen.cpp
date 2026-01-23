#include "rndobj/HiResScreen.h"
#include "utl/MemMgr.h"

HiResScreen gHiResScreen;
HiResScreen &TheHiResScreen = gHiResScreen;

void HiResScreen::BmpCache::DeleteCache() {
    for (unsigned int i = 0; i < mTotalNumCacheLines; i++) {
        FileDelete(mFileNames[i].c_str());
    }
}

int HiResScreen::GetPaddingX() const { return 480; }
int HiResScreen::GetPaddingY() const { return 270; }

HiResScreen::BmpCache::BmpCache(unsigned int ui1, unsigned int ui2) {
    mRowsPerCacheLine = ui2 + 1;
    mPixelsPerRow = ui1;
    mTotalRows = ui2;
    mDirtyStart = 0;
    mDirtyEnd = 0;
    mByteSize = mTotalRows % mRowsPerCacheLine;
    MILO_ASSERT(mTotalRows % mRowsPerCacheLine == 0, 0x3B);
    mTotalNumCacheLines = mTotalRows / mRowsPerCacheLine;
    mFileNames = new String[mTotalNumCacheLines];
    for (uint i = 0; i < mTotalNumCacheLines; i++) {
        mFileNames[i] = MakeString("_hires_cache_%.2d.dat", i);
    }
    mBuffer = (unsigned char *)MemAlloc(mByteSize, __FILE__, 0x44, "HiResScreenCache");
    mCurrLoadedIndex = ui2;
    DeleteCache();
}

HiResScreen::BmpCache::~BmpCache() {
    DeleteCache();
    delete[] mFileNames;
    mFileNames = 0;
    delete mBuffer;
    mBuffer = 0;
}

Hmx::Rect HiResScreen::InvScreenRect() const {
    // 20 = x
    // 1c = y
    // 18 = w
    // 14 = h
    auto screenRect = ScreenRect();
    float rH = 1 / screenRect.h;
    float rW = 1 / screenRect.w;
    float rY = (-screenRect.h * screenRect.y);
    float rX = (-screenRect.w * screenRect.x);
    Hmx::Rect rect(rX, rY, rW, rH);
    return rect;
}

void HiResScreen::GetBorderForTile(int i1, int i2, int &i3, int &i4, int &i5, int &i6) const {
    i3 = 0;
    i4 = 0;
    i5 = 0;
    i6 = 0;
    int width = TheRnd.Width();
    if (((width - 480) * i1 + width) < mAccumWidth) {
        i5 = 480;
    }
    else if (0 < (i1 + 480) * (width - 1) - width) {
        i3 = 480;
    }
    int height = TheRnd.Height();
    if ((height - 270) * i2 + height < mAccumHeight) {
        i6 = 270;
    }
    else if (0 < (i2 + 2) * (height - 270) - height) {
        i4 = 270;
    }
}

void HiResScreen::BmpCache::FlushCache() {
    MILO_ASSERT(mCurrLoadedIndex < mTotalNumCacheLines, 0x9c);
    if (mDirtyEnd > mDirtyStart) {
        auto cacheFile = NewFile(mFileNames->c_str(), 1);
        MILO_ASSERT(cacheFile, 0xa2);
        cacheFile->Seek(mDirtyStart, 0);
        unsigned int nBuffRange = mDirtyEnd - mDirtyStart;
        MILO_ASSERT(nBuffRange < mByteSize, 0xaa);
        int numWritten = cacheFile->Write((void *)mBuffer[mDirtyStart], nBuffRange);
        MILO_ASSERT(numWritten == nBuffRange, 0xae);
        cacheFile->Flush();
        cacheFile->Truncate(1);
        mDirtyStart = 0;
        mDirtyEnd = 0;
    }
}

void HiResScreen::TakeShot(const char *c, int i) {
    mFileBase = c;
    mTiling = i;
    mActive = true;
    mAccumHeight = 0;
    //BmpCache bmpCache;
    if (TheRnd.Width() < 481 || TheRnd.Height() < 271) {
        MILO_NOTIFY("Padding exceeds screen size");
    }
    else {
        mAccumWidth = TheRnd.Width() * i + i * -480;
        mAccumHeight = TheRnd.Height() * i + i * -270;
        if (TheRnd.Width() < mAccumWidth && TheRnd.Height() < mAccumHeight) {
            //bmpCache = MemAlloc(0x2c, __FILE__, 0x6b, "BmpCache", 0);
        }
    }
}