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
        MILO_ASSERT(nBuffRange < mByteSize, 0xAA);
        unsigned int numWritten = cacheFile->Write(mBuffer + mDirtyStart, nBuffRange);
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
    }
    mDirtyEnd = mDirtyStart;
    if (cacheFile != 0) {
        delete cacheFile;
    }
    mCurrLoadedIndex = newIndex;
}

void HiResScreen::BmpCache::GetPixelColor(
    int x, int y, unsigned char &r, unsigned char &g, unsigned char &b, unsigned char &a
) const {
    MILO_ASSERT(x >= 0 && x < mPixelsPerRow, 0xBC);
    unsigned int nLoadedStart = mCurrLoadedIndex * mRowsPerCacheLine;
    unsigned int nLoadedEnd = nLoadedStart + mRowsPerCacheLine - 1;
    MILO_ASSERT(y >= nLoadedStart && y <= nLoadedEnd, 0xC1);
    unsigned int yOffset = nLoadedEnd - y;
    unsigned int offset = (yOffset * mPixelsPerRow + x) * 4;
    a = mBuffer[offset + 3];
    r = mBuffer[offset + 2];
    g = mBuffer[offset + 1];
    b = mBuffer[offset];
}

void HiResScreen::BmpCache::SetPixelColor(
    int x, int y, unsigned char r, unsigned char g, unsigned char b, unsigned char a
) {
    MILO_ASSERT(x >= 0 && x < mPixelsPerRow, 0xD0);
    unsigned int nLoadedStart = mCurrLoadedIndex * mRowsPerCacheLine;
    unsigned int nLoadedEnd = nLoadedStart + mRowsPerCacheLine - 1;
    MILO_ASSERT(y >= nLoadedStart && y <= nLoadedEnd, 0xD5);
    unsigned int yOffset = nLoadedEnd - y;
    unsigned int offset = (yOffset * mPixelsPerRow + x) * 4;
    unsigned int newPixel = (a << 24) | (r << 16) | (g << 8) | b;
    unsigned int oldPixel = *(unsigned int *)(mBuffer + offset);
    if (newPixel != oldPixel) {
        *(unsigned int *)(mBuffer + offset) = newPixel;
        if (offset < mDirtyStart) {
            mDirtyStart = offset;
        }
        unsigned int offsetEnd = offset + 4;
        if (mDirtyEnd < offsetEnd) {
            mDirtyEnd = offsetEnd;
        }
    }
}

void HiResScreen::TakeShot(const char *c, int i) {
    mFileBase = c;
    mTiling = i;
    mActive = true;
    mCurrTile = 0;
    if (TheRnd.Width() <= 480 || TheRnd.Height() <= 270) {
        MILO_NOTIFY(MakeString("Padding exceeds screen size"));
        mActive = false;
    } else {
        mAccumWidth = i * (TheRnd.Width() - 480);
        mAccumHeight = i * TheRnd.Height() - i * 270;
        if ((int)mAccumWidth < TheRnd.Width() || (int)mAccumHeight < TheRnd.Height()) {
            MILO_NOTIFY(MakeString("HiResScreenshot requires more tiles (%d specified)", i));
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

void HiResScreen::GetBorderForTile(int x, int y, int &left, int &right, int &top, int &bottom)
    const {
    left = 0;
    right = 0;
    top = 0;
    bottom = 0;
    int xStep = TheRnd.Width() - 480;
    int xPos = xStep * x + TheRnd.Width();
    if (xPos < (int)mAccumWidth) {
        right = 480;
    } else if ((x + 1) * xStep > TheRnd.Width()) {
        left = 480;
    }
    int yStep = TheRnd.Height() - 270;
    int yPos = yStep * y + TheRnd.Height();
    if (yPos < (int)mAccumHeight) {
        bottom = 270;
    } else if ((y + 1) * yStep > TheRnd.Height()) {
        top = 270;
    }
}

void HiResScreen::Accumulate() {
    if (mCurrTile == 0) {
        mCurrTile = 1;
        return;
    }
    int prevTile = mCurrTile - 1;
    if (prevTile >= mTiling * mTiling) {
        return;
    }
    RndTex *tex = Hmx::Object::New<RndTex>();
    RndBitmap bm;
    tex->SetBitmap(0, 0, 0, RndTex::kFrontBuffer, false, 0);
    tex->LockBitmap(bm, true);
    delete tex;
    int tileX = prevTile % mTiling;
    int tileY = prevTile / mTiling;
    int left, right, top, bottom;
    GetBorderForTile(tileX, tileY, left, right, top, bottom);
    int xStep = TheRnd.Width() - 480;
    int yStep = TheRnd.Height() - 270;
    int xOff = xStep * tileX;
    int yOff = yStep * tileY;
    Merge(bm, left, top, bm.Width(), bm.Height(), xOff, yOff, left, top);
    TheRnd.ResetProcCounter();
    mCurrTile++;
    bm.Reset();
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
    RndBitmap bm;
    bm.Create(mAccumWidth, mAccumHeight, 32, 0, 0, 0, 0, 0);
    bm.SaveBmpHeader(fs);
    delete &bm;
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
        loResBm.Reset();
    }
    mActive = false;
    TheRnd.SetEvenOddDisabled(mEvenOddDisabled);
    TheRnd.SetShrinkToSafeArea(mShrinkToSafe);
    TheRnd.ShowConsole(mConsoleShowing);
    if (mCache) {
        delete mCache;
    }
    bm.Reset();
}

void HiResScreen::Merge(
    const RndBitmap &bm, int srcX, int srcY, int srcW, int srcH, int dstX, int dstY, int padX, int padY
) {
    if (srcW >= srcH) {
        return;
    }
    int xStart = dstX;
    int xEnd = srcH;
    int xRange = xEnd - srcX;
    for (; xStart < mAccumHeight && xStart >= 0; xStart++, xRange++) {
        if (xStart + xRange >= srcH) {
            break;
        }
        mCache->LoadCache(xStart);
        int yStart = srcY;
        int yOff = srcX - padX;
        int yRange = srcY - padY;
        for (; yStart < mAccumWidth && yStart >= 0; yStart++, yOff++, yRange++) {
            if (yStart + yRange >= srcW) {
                break;
            }
            int bmX = xRange + xStart;
            int bmY = yRange + yStart;
            unsigned char r, g, b, a;
            bm.PixelColor(bmY, bmX, r, g, b, a);
            unsigned char cr, cg, cb, ca;
            mCache->GetPixelColor(yStart, xStart, cr, cg, cb, ca);
            float blendX = 0.0f;
            float blendY = 0.0f;
            if (bmY > padX) {
                blendX = (float)yOff / (float)padX;
            }
            if (bmX > srcX) {
                blendY = (float)xRange / (float)srcX;
            }
            float blend = 0.0f;
            if (blendX > 0.0f || blendY > 0.0f) {
                blend = sqrtf(blendX * blendX + blendY * blendY);
                blend = (blend - 0.5f) * 2.0f;
                if (blend < 0.0f) blend = 0.0f;
                if (blend > 1.0f) blend = 1.0f;
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
    int tiling = mTiling;
    int newHeight = (tiling * 270 + mAccumHeight) / tiling;
    int newWidth = (tiling * 480 + mAccumWidth) / tiling;
    float scaleY = (float)mAccumHeight / (float)newHeight;
    float scaleX = (float)mAccumWidth / (float)newWidth;
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
    int tiling = mTiling;
    int tile = mCurrTile;
    int tileX = tile % tiling;
    int tileY = tile / tiling;
    float invTiling = 1.0f / (float)tiling;
    float tileXf = (float)tileX;
    float tileYf = (float)tileY;
    float tileXEnd = tileXf + 1.0f;
    float tileYEnd = tileYf + 1.0f;
    float x = inRect.x + inRect.w;
    float y = inRect.y + inRect.h;
    float x0 = (inRect.x - tileXf * invTiling) / (tileXEnd * invTiling - tileXf * invTiling);
    float x1 = (x - tileXf * invTiling) / (tileXEnd * invTiling - tileXf * invTiling);
    float y0 = (inRect.y - tileYf * invTiling) / (tileYEnd * invTiling - tileYf * invTiling);
    float y1 = (y - tileYf * invTiling) / (tileYEnd * invTiling - tileYf * invTiling);
    if (x0 < 0.0f) x0 = 0.0f;
    if (x0 > 1.0f) x0 = 1.0f;
    if (x1 < 0.0f) x1 = 0.0f;
    if (x1 > 1.0f) x1 = 1.0f;
    if (y0 < 0.0f) y0 = 0.0f;
    if (y0 > 1.0f) y0 = 1.0f;
    if (y1 < 0.0f) y1 = 0.0f;
    if (y1 > 1.0f) y1 = 1.0f;
    outTileRect.x = x0;
    outTileRect.w = x1;
    outTileRect.y = y0;
    outTileRect.h = y1;
    outAccumRect.x = x0 * invTiling + tileXf * invTiling;
    outAccumRect.y = y0 * invTiling + tileYf * invTiling;
    outAccumRect.w = x1 - x0;
    outAccumRect.h = y1 - y0;
}

Hmx::Rect HiResScreen::ScreenRect(const RndCam *cam, const Hmx::Rect &r) const {
    Hmx::Rect ret = r;
    if ((cam->TargetTex() != 0 && !mOverride) || !mActive || mCurrTile >= mTiling * mTiling) {
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
    Hmx::Rect ret;
    ret.w = 1.0f / r.w;
    ret.h = -1.0f / r.w;
    ret.x = -r.x * ret.w;
    ret.y = ret.h * r.y;
    return ret;
}
