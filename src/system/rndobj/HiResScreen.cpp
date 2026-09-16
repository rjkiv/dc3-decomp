#include "rndobj/HiResScreen.h"
#include "os/Debug.h"
#include "os/File.h"
#include "rndobj/Rnd.h"
#include "rndobj/Bitmap.h"
#include "rndobj/Tex.h"
#include "utl/MemMgr.h"
#include "utl/MakeString.h"
#include "utl/FileStream.h"

HiResScreen gHiResScreen;
HiResScreen &TheHiResScreen = gHiResScreen;

HiResScreen::BmpCache::BmpCache(unsigned int ui1, unsigned int ui2) {
    mRowsPerCacheLine = ui2 + 1;
    mPixelsPerRow = ui1;
    mTotalRows = ui2;
    mDirtyStart = 0;
    mDirtyEnd = 0;
    do {
        while (mTotalRows % --mRowsPerCacheLine != 0)
            ;
        mByteSize = mRowsPerCacheLine * mPixelsPerRow * 4;
    } while (7200000 < mByteSize);
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
    mFileNames = nullptr;
    delete mBuffer;
    mBuffer = nullptr;
}

void HiResScreen::BmpCache::DeleteCache() {
    for (unsigned int i = 0; i < mTotalNumCacheLines; i++) {
        FileDelete(mFileNames[i].c_str());
    }
}

void HiResScreen::BmpCache::GetLoadedRange(unsigned int &ui1, unsigned int &ui2) const {
    ui1 = mCurrLoadedIndex * mRowsPerCacheLine;
    ui2 = ui1 + mRowsPerCacheLine - 1;
}

void HiResScreen::BmpCache::FlushCache() {
    MILO_ASSERT(mCurrLoadedIndex < mTotalNumCacheLines, 0x9C);
    if (mDirtyEnd > mDirtyStart) {
        File *cacheFile = NewFile(mFileNames[mCurrLoadedIndex].c_str(), 1);
        MILO_ASSERT(cacheFile, 0xA2);
        cacheFile->Seek(mDirtyStart, 0);
        unsigned int nBuffRange = mDirtyEnd - mDirtyStart;
        unsigned char *bufStart = mBuffer + mDirtyStart;
        MILO_ASSERT(nBuffRange <= mByteSize, 0xAA);
        unsigned int numWritten = cacheFile->Write(bufStart, nBuffRange);
        MILO_ASSERT(numWritten == nBuffRange, 0xAE);
        cacheFile->Flush();
        delete cacheFile;
        mDirtyStart = 0;
        mDirtyEnd = 0;
    }
}

void HiResScreen::BmpCache::LoadCache(unsigned int y) {
    unsigned int nLoadedStart = mCurrLoadedIndex * mRowsPerCacheLine;
    unsigned int nLoadedEnd = nLoadedStart + mRowsPerCacheLine - 1;
    if (y >= nLoadedStart && y <= nLoadedEnd) {
        return;
    }
    if (mCurrLoadedIndex < mTotalNumCacheLines) {
        FlushCache();
    }
    unsigned int newIndex = y / mRowsPerCacheLine;
    File *cacheFile = NewFile(mFileNames[newIndex].c_str(), 2);
    if (cacheFile == 0) {
        memset(mBuffer, 0, mByteSize);
        cacheFile = NewFile(mFileNames[newIndex].c_str(), 0x101);
        MILO_ASSERT(cacheFile, 0x80);
        mDirtyStart = 0;
        mDirtyEnd = mByteSize;
    } else {
        unsigned int numRead = cacheFile->Read(mBuffer, mByteSize);
        MILO_ASSERT(numRead == mByteSize, 0x8A);
        mDirtyStart = 0;
        mDirtyEnd = 0;
    }
    if (cacheFile != 0) {
        delete cacheFile;
    }
    mCurrLoadedIndex = newIndex;
}

void HiResScreen::BmpCache::GetPixelColor(
    int x, int y, unsigned char &r, unsigned char &g, unsigned char &b, unsigned char &a
) const {
    MILO_ASSERT(x >= 0 && x < mPixelsPerRow, 0xBC);
    unsigned int nLoadedStart, nLoadedEnd;
    GetLoadedRange(nLoadedStart, nLoadedEnd);
    MILO_ASSERT(y >= nLoadedStart && y <= nLoadedEnd, 0xC1);
    unsigned int yOffset = nLoadedEnd - y;
    unsigned int offset = (yOffset * mPixelsPerRow + x) * 4;
    unsigned char *colorStart = &mBuffer[offset];
    a = colorStart[3];
    r = colorStart[2];
    g = colorStart[1];
    b = colorStart[0];
}

void HiResScreen::BmpCache::SetPixelColor(
    int x, int y, unsigned char r, unsigned char g, unsigned char b, unsigned char a
) {
    MILO_ASSERT(x >= 0 && x < mPixelsPerRow, 0xD0);
    unsigned int nLoadedStart, nLoadedEnd;
    GetLoadedRange(nLoadedStart, nLoadedEnd);
    MILO_ASSERT(y >= nLoadedStart && y <= nLoadedEnd, 0xD5);
    unsigned int byteOffset = ((nLoadedEnd - y) * mPixelsPerRow + x) * 4;
    unsigned char color[4] = { a, r, g, b };
    unsigned int colorWord = *reinterpret_cast<unsigned int *>(color);
    unsigned int *colorToSet = (unsigned int *)&mBuffer[byteOffset];
    if (colorWord != *colorToSet) {
        *colorToSet = colorWord;
        mDirtyStart = Min(mDirtyStart, byteOffset);
        mDirtyEnd = Max(mDirtyEnd, byteOffset + 4);
    }
}

int HiResScreen::GetPaddingX() const { return 480; }
int HiResScreen::GetPaddingY() const { return 270; }

void HiResScreen::TakeShot(const char *c, int i) {
    mFileBase = c;
    mTiling = i;
    mActive = true;
    mCurrTile = 0;
    if (TheRnd.Width() <= GetPaddingX() || TheRnd.Height() <= GetPaddingY()) {
        MILO_NOTIFY("Padding exceeds screen size");
        mActive = false;
    } else {
        int paddingXMult = i * GetPaddingX();
        int paddingYMult = i * GetPaddingY();
        mAccumWidth = TheRnd.Width() * i - paddingXMult;
        mAccumHeight = TheRnd.Height() * i - paddingYMult;
        if (mAccumWidth < TheRnd.Width() || mAccumHeight < TheRnd.Height()) {
            int specified = i;
            MILO_NOTIFY("HiResScreenshot requires more tiles (%d specified)", specified);
            mActive = false;
        } else {
            mCache = new BmpCache(mAccumWidth, mAccumHeight);
            mEvenOddDisabled = TheRnd.GetEvenOddDisabled();
            mShrinkToSafe = TheRnd.ShrinkToSafeArea();
            mConsoleShowing = TheRnd.ConsoleShowing();
            TheRnd.SetEvenOddDisabled(true);
            TheRnd.SetShrinkToSafeArea(false);
            TheRnd.ShowConsole(false);
        }
    }
}

void HiResScreen::GetBorderForTile(
    int x, int y, int &left, int &right, int &top, int &bottom
) const {
    left = 0;
    top = 0;
    right = 0;
    bottom = 0;
    int w = TheRnd.Width() - GetPaddingX();
    if (w * x + TheRnd.Width() < mAccumWidth) {
        top = GetPaddingX();
    } else if (w * (x + 1) - TheRnd.Width() > 0) {
        left = GetPaddingX();
    }
    int h = TheRnd.Height() - GetPaddingY();
    if (h * y + TheRnd.Height() < mAccumHeight) {
        bottom = GetPaddingY();
    } else if (h * (y + 1) - TheRnd.Height() > 0) {
        right = GetPaddingY();
    }
}

void HiResScreen::Accumulate() {
    if (mCurrTile == 0) {
        mCurrTile = 1;
    } else {
        int prevTile = mCurrTile - 1;
        if (prevTile < mTiling * mTiling) {
            RndTex *tex = Hmx::Object::New<RndTex>();
            RndBitmap bm;
            tex->SetBitmap(0, 0, 0, RndTex::kFrontBuffer, false, 0);
            tex->LockBitmap(bm, true);
            delete tex;
            int tileX = prevTile % mTiling;
            int tileY = prevTile / mTiling;
            int left, right, top, bottom;
            GetBorderForTile(tileX, tileY, left, right, top, bottom);
            int x = (TheRnd.Width() - GetPaddingX()) * tileX;
            int y = (TheRnd.Height() - GetPaddingY()) * tileY;
            Merge(bm, x, y, left, right, bm.Width(), bm.Height(), top, bottom);
            TheRnd.ResetProcCounter();
            mCurrTile++;
        }
    }
}

void HiResScreen::Finish() {
    int fileNum = 0;
    String filename;
    File *existFile = 0;
    do {
        fileNum++;
        filename = MakeString("%s_%d.bmp", mFileBase, fileNum);
        if (existFile) {
            delete existFile;
        }
        existFile = NewFile(filename.c_str(), 1);
    } while (existFile);
    mCache->FlushCache();
    FileStream *fs = new FileStream(filename.c_str(), FileStream::kWrite, true);
    void *pixels = MemAlloc(0x40, __FILE__, 0x250, "TmpRndBitmap");
    RndBitmap bm;
    bm.Create(mAccumWidth, mAccumHeight, 0, 32, 0, 0, pixels, 0);
    bm.SaveBmpHeader(fs);
    delete pixels;
    for (int i = mCache->mTotalNumCacheLines - 1; i >= 0; i--) {
        mCache->LoadCache(i * mCache->mRowsPerCacheLine);
        fs->Write(mCache->mBuffer, mCache->mByteSize);
    }
    if (fs) {
        delete fs;
    }
    FileMkDir("lo_res");
    filename = MakeString("lo_res/%s_%d.bmp", mFileBase, fileNum);
    File *loResFile = NewFile(filename.c_str(), 0x101);
    if (loResFile) {
        delete loResFile;
        RndBitmap loResBm;
        DownSample(loResBm);
        loResBm.SaveBmp(filename.c_str());
    }
    mActive = false;
    TheRnd.SetEvenOddDisabled(mEvenOddDisabled);
    TheRnd.SetShrinkToSafeArea(mShrinkToSafe);
    TheRnd.ShowConsole(mConsoleShowing);
    delete mCache;
}

void HiResScreen::Merge(
    const RndBitmap &src,
    int dstX,
    int dstY,
    int srcX,
    int srcXEnd,
    int srcY,
    int srcYEnd,
    int blendX,
    int blendY
) {
    if (srcX >= srcXEnd) {
        return;
    }
    int xStart = srcY;
    int xEnd = srcXEnd;
    int xRange = xEnd - dstX;
    for (; xStart < mAccumHeight && xStart >= 0; xStart++, xRange++) {
        if (xStart + xRange >= srcXEnd) {
            break;
        }
        mCache->LoadCache(xStart);
        int yStart = dstY;
        int yOff = dstX - blendX;
        int yRange = dstY - blendY;
        for (; yStart < mAccumWidth && yStart >= 0; yStart++, yOff++, yRange++) {
            if (yStart + yRange >= srcX) {
                break;
            }
            int bmX = xRange + xStart;
            int bmY = yRange + yStart;
            unsigned char r, g, b, a;
            src.PixelColor(bmY, bmX, r, g, b, a);
            unsigned char cr, cg, cb, ca;
            mCache->GetPixelColor(yStart, xStart, cr, cg, cb, ca);
            float blendX = 0.0f;
            float blendY = 0.0f;
            if (bmY > blendX) {
                blendX = (float)yOff / (float)blendX;
            }
            if (bmX > dstX) {
                blendY = (float)xRange / (float)dstX;
            }
            float blend = 0.0f;
            if (blendX > 0.0f || blendY > 0.0f) {
                blend = sqrtf(blendX * blendX + blendY * blendY);
                blend = (blend - 0.5f) * 2.0f;
                if (blend < 0.0f)
                    blend = 0.0f;
                if (blend > 1.0f)
                    blend = 1.0f;
            }
            float invBlend = (1.0f - blend) * 255.0f;
            unsigned char newA = (unsigned char)invBlend;
            if (ca != 0) {
                float t = ca / 255.0f;
                int dr = cr - r;
                int dg = cg - g;
                int db = cb - b;
                r += (unsigned char)(dr * t + 0.5f);
                g += (unsigned char)(dg * t + 0.5f);
                b += (unsigned char)(db * t + 0.5f);
                if (newA < ca) {
                    newA = ca;
                }
            }
            mCache->SetPixelColor(yStart, xStart, r, g, b, newA);
        }
    }
}

void HiResScreen::DownSample(RndBitmap &outBm) {
    int newWidth = (mTiling * GetPaddingX() + mAccumWidth) / mTiling;
    int newHeight = (mTiling * GetPaddingY() + mAccumHeight) / mTiling;
    float scaleX = (float)mAccumWidth / (float)newWidth;
    float scaleY = (float)mAccumHeight / (float)newHeight;
    outBm.Create(newWidth, newHeight, 32, 0, 0, 0, 0, 0);
    memset(outBm.Buffer(), 0, outBm.PixelBytes());
    for (int y = 0; y < newHeight; y++) {
        int srcY = (int)(y * scaleY);
        mCache->LoadCache(srcY);
        for (int x = 0; x < newWidth; x++) {
            int srcX = (int)(x * scaleX);
            unsigned char r, g, b, a;
            mCache->GetPixelColor(srcX, srcY, r, g, b, a);
            outBm.SetPixelColor(x, y, r, g, b, a);
        }
    }
}

void HiResScreen::CurrentTileRect(
    const Hmx::Rect &inRect, Hmx::Rect &outTileRect, Hmx::Rect &outAccumRect
) const {
    float invTiling = 1.0f / mTiling;
    float mod = mCurrTile % mTiling;
    float div = mCurrTile / mTiling;
    float tileXf = mod * invTiling;
    float tileYf = div * invTiling;

    float midX = (mod + 1.0f) * invTiling;
    float midY = (div + 1.0f) * invTiling;

    float rectX = (inRect.x - tileXf) / (midX - tileXf);
    float rectY = (inRect.y - tileYf) / (midY - tileYf);
    float rectW = ((inRect.w + inRect.x) - tileXf) / (midX - tileXf);
    float rectH = ((inRect.h + inRect.y) - tileYf) / (midY - tileYf);

    rectX = Clamp(0.0f, 1.0f, rectX);
    rectY = Clamp(0.0f, 1.0f, rectY);
    rectW = Clamp(0.0f, 1.0f, rectW);
    rectH = Clamp(0.0f, 1.0f, rectH);
    outTileRect.Set(rectX, rectY, rectW, rectH);

    rectX = rectX * invTiling + tileXf;
    rectY = rectY * invTiling + tileYf;
    rectW = rectW * invTiling + tileXf;
    rectH = rectH * invTiling + tileYf;

    outAccumRect.Set(rectX, rectY, rectW - rectX, rectH - rectY);
}

Hmx::Rect HiResScreen::ScreenRect(const RndCam *cam, const Hmx::Rect &r) const {
    Hmx::Rect ret = r;
    if ((cam->TargetTex() != 0 && !mOverride) || !mActive
        || mCurrTile >= mTiling * mTiling) {
        return r;
    }
    int tiling = mTiling;
    float invTiling = 1.0f / (float)tiling;
    Hmx::Rect tileRect, accumRect;
    CurrentTileRect(r, tileRect, accumRect);
    int tileX = mCurrTile % tiling;
    int tileY = mCurrTile / tiling;
    int left, right, top, bottom;
    GetBorderForTile(tileX, tileY, left, right, top, bottom);
    float screenH = (float)TheRnd.Height();
    float screenW = (float)TheRnd.Width();
    float leftF = (float)left;
    float topF = (float)top;
    float rightF = (float)right;
    float bottomF = (float)bottom;
    float xScale = screenH / (screenH - leftF);
    float yScale = screenW / (screenW - topF);
    float xOffset = xScale - invTiling;
    float yOffset = yScale - invTiling;
    xOffset = xOffset - invTiling;
    float xShift = screenH / (screenH - rightF);
    float yShift = screenW / (screenW - bottomF);
    xShift = xShift - invTiling;
    yShift = yShift - invTiling;
    ret.x = accumRect.x - xOffset;
    ret.w = accumRect.w + xOffset + xShift;
    ret.y = accumRect.y - yOffset;
    ret.h = accumRect.h + yOffset + yShift;
    return ret;
}

Hmx::Rect HiResScreen::ScreenRect() const {
    const RndCam *cam = RndCam::Current();
    Hmx::Rect r = cam->GetScreenRect();
    return ScreenRect(cam, r);
}

Hmx::Rect HiResScreen::InvScreenRect() const {
    Hmx::Rect r = ScreenRect();
    return Hmx::Rect(-r.x / r.w, -r.y / r.h, 1 / r.w, 1 / r.h);
}
