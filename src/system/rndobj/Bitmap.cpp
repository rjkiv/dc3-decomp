#include "rndobj/Bitmap.h"
#include "utl/CRC.h"
#include "utl/ChunkStream.h"
#include "utl/FileStream.h"
#include "utl/MemMgr.h"

unsigned char BITMAP_REV = 2;

int RndBitmap::NumMips() const {
    const RndBitmap *x = this;
    int i;
    for (i = 0; x->mMip; i++)
        x = x->mMip;
    return i;
}

int RndBitmap::PixelBytes() const { return mRowBytes * mHeight; }
int RndBitmap::DxtRowBytes() const { return mOrder & 0x38 ? mRowBytes * 4 : mRowBytes; }

unsigned char RndBitmap::PixelIndex(int i1, int i2) const {
    bool bb;
    int offset = PixelOffset(i1, i2, bb);
    unsigned char pixel = mPixels[offset];
    if (mBpp == 8) {
        return pixel;
    } else if (bb) {
        return pixel >> 4;
    } else {
        return pixel & 0xF;
    }
}

void RndBitmap::SetName(const Hmx::CRC &crc) { mName = crc; }

BinStream &RndBitmap::LoadHeader(BinStream &bs, u8 &numMips) {
    u8 rev, h;
    u8 pad[32];
    bs.Tell();
    bs >> rev;
    if (rev > 1)
        bs >> mName;
    bs >> mBpp;
    if (rev > 0)
        bs >> mOrder;
    else {
        bs >> h;
        mOrder = h;
    }
    bs >> numMips;
    bs >> mWidth;
    bs >> mHeight;
    bs >> mRowBytes;

    int count;
    if (rev == 0) {
        count = 6;
    } else
        count = rev == 1 ? 0x13 : 0xF;
    bs.Read(pad, count);
    return bs;
}

BinStream &RndBitmap::SaveHeader(BinStream &bs) const {
    static u8 pad[0xf];
    bs << BITMAP_REV << mName << mBpp << (unsigned int)mOrder << (unsigned char)NumMips()
       << mWidth << mHeight;
    bs << mRowBytes;
    bs.Write(pad, 0xf);
    return bs;
}

BinStream &operator>>(BinStream &bs, tagBITMAPFILEHEADER &bmfh) {
    bs >> bmfh.bfSize;
    bs >> bmfh.bfReserved1;
    bs >> bmfh.bfReserved2;
    bs >> bmfh.bfOffBits;
    return bs;
}

BinStream &operator<<(BinStream &bs, const tagBITMAPFILEHEADER &bmfh) {
    bs << bmfh.bfSize << bmfh.bfReserved1 << bmfh.bfReserved2 << bmfh.bfOffBits;
    return bs;
}

BinStream &operator>>(BinStream &bs, tagBITMAPINFOHEADER &bmih) {
    bs >> bmih.biSize;
    bs >> bmih.biWidth;
    bs >> bmih.biHeight;
    bs >> bmih.biPlanes;
    bs >> bmih.biBitCount;
    bs >> bmih.biCompression;
    bs >> bmih.biSizeImage;
    bs >> bmih.biXPelsPerMeter;
    bs >> bmih.biYPelsPerMeter;
    bs >> bmih.biClrUsed;
    bs >> bmih.biClrImportant;
    return bs;
}

BinStream &operator<<(BinStream &bs, const tagBITMAPINFOHEADER &bmih) {
    bs << bmih.biSize << bmih.biWidth << bmih.biHeight << bmih.biPlanes << bmih.biBitCount
       << bmih.biCompression << bmih.biSizeImage << bmih.biXPelsPerMeter
       << bmih.biYPelsPerMeter << bmih.biClrUsed << bmih.biClrImportant;
    return bs;
}

void RndBitmap::Reset() {
    mName.Reset();
    mRowBytes = 0;
    mHeight = 0;
    mWidth = 0;
    mBpp = 32;
    mOrder = 1;
    mPalette = nullptr;
    mPixels = nullptr;
    if (mBuffer) {
        MemFree(mBuffer, __FILE__, 0x16C);
        mBuffer = nullptr;
    }
    RELEASE(mMip);
}

bool RndBitmap::LoadBmp(BinStream *bs) {
    unsigned short us;
    *bs >> us;
    if (us != 0x4D42) {
        MILO_NOTIFY("%s not BMP format", bs->Name());
        return false;
    } else {
        tagBITMAPFILEHEADER header;
        *bs >> header;
        return LoadDIB(bs, header.bfOffBits);
    }
}

void RndBitmap::SaveBmp(BinStream *file) const {
    MILO_ASSERT(file, 0x515);
    if (mOrder & 1) {
        MILO_NOTIFY("Order isn't kARGB");
    } else {
        SaveBmpHeader(file);
        SaveBmpPixels(file);
    }
}

void RndBitmap::SaveBmp(const char *cc) const {
    FileStream *file = new FileStream(cc, FileStream::kWrite, true);
    SaveBmp(file);
    delete file;
}

int RndBitmap::PaletteBytes() const {
    if (mBpp <= 8) {
        if ((mOrder & 0x38) == 0) {
            return (1 << mBpp) * 4;
        }
    }
    return 0;
}

void RndBitmap::AllocateBuffer() {
    int paletteBytes;
    if (mPalette)
        paletteBytes = 0;
    else
        paletteBytes = PaletteBytes();
    int sum = paletteBytes + PixelBytes();
    if (sum)
        mBuffer = (u8 *)MemAlloc(sum, __FILE__, 0x1A9, "Bitmap_buf");
    if (paletteBytes)
        mPalette = mBuffer;
    mPixels = mBuffer + paletteBytes;
}

void RndBitmap::SetPixelIndex(int i1, int i2, unsigned char uc) {
    bool bb;
    int offset = PixelOffset(i1, i2, bb);
    u8 *pixels = mPixels;
    if (mBpp == 8) {
        *(pixels + offset) = uc;
    } else if (bb) {
        *(pixels + offset) = uc << 4 | *(pixels + offset) & 0xF;
    } else {
        *(pixels + offset) = *(pixels + offset) & 0xF0 | uc;
    }
}

unsigned char RndBitmap::NearestColor(
    unsigned char r, unsigned char g, unsigned char b, unsigned char a
) const {
    int paletteColorIdx = -1;
    int minDiff = 0x40000;
    unsigned char pr, pg, pb, pa;
    for (int i = (1 << mBpp) - 1; i >= 0; i--) {
        PaletteColor(i, pr, pg, pb, pa);
        int dr = pr - r;
        int dg = pg - g;
        int db = pb - b;
        int da = pa - a;
        int diff = dr * dr + dg * dg + db * db + da * da;
        if (diff < minDiff) {
            minDiff = diff;
            paletteColorIdx = i;
        }
    }
    return paletteColorIdx;
}

void RndBitmap::PaletteColor(
    int i, unsigned char &r, unsigned char &g, unsigned char &b, unsigned char &a
) const {
    ConvertColor(mPalette + PaletteOffset(i) * 4, r, g, b, a);
}

int RndBitmap::PaletteOffset(int i) const {
    if ((mOrder & 2) && mBpp == 8) {
        if ((i & 0x18) == 8) {
            i += 8;
        } else if ((i & 0x18) == 0x10) {
            i -= 8;
        }
    }
    return i;
}

unsigned char RndBitmap::RowNonTransparent(int x, int y, int z, int *iptr) {
    for (int i = x; i < y; i++) {
        unsigned char r, g, b, a;
        PixelColor(i, z, r, g, b, a);
        if (a != 0) {
            *iptr = i;
            return a;
        }
    }
    return 0;
}

unsigned char RndBitmap::ColumnNonTransparent(int x, int y, int z, int *iptr) {
    for (int i = y; i < z; i++) {
        unsigned char r, g, b, a;
        PixelColor(x, i, r, g, b, a);
        if (a != 0) {
            *iptr = i;
            return a;
        }
    }
    return 0;
}

bool RndBitmap::IsTranslucent() const {
    if (mBpp == 24)
        return false;
    for (int i = 0; i < mHeight; i++) {
        for (int j = 0; j < mWidth; j++) {
            unsigned char r, g, b, a;
            PixelColor(j, i, r, g, b, a);
            if (a < 253)
                return true;
        }
    }
    return false;
}

void RndBitmap::GenerateMips() {
    int dim = Min(mWidth, mHeight);
    if (dim > 16U) {
        RELEASE(mMip);
        mMip = new RndBitmap();
        mMip->Create(mWidth >> 1, mHeight >> 1, 0, mBpp, mOrder, mPalette, 0, 0);
        int i18 = 0;
        for (int i = 0; i < mMip->mHeight; i++) {
            int i181 = i18 + 1;
            int i17 = 0;
            for (int j = 0; j < mMip->mWidth; j++) {
                int i171 = i17 + 1;
                unsigned char r, g, b, a;
                PixelColor(i17, i18, r, g, b, a);
                unsigned short rsum = r;
                unsigned short gsum = g;
                unsigned short bsum = b;
                unsigned short asum = a;
                PixelColor(i171, i18, r, g, b, a);
                rsum += r;
                gsum += g;
                bsum += b;
                asum += a;
                PixelColor(i17, i181, r, g, b, a);
                rsum += r;
                gsum += g;
                bsum += b;
                asum += a;
                PixelColor(i171, i181, r, g, b, a);
                rsum += r;
                gsum += g;
                bsum += b;
                asum += a;
                mMip->SetPixelColor(j, i, rsum >> 2, gsum >> 2, bsum >> 2, asum >> 2);
                i17 += 2;
            }
            i18 += 2;
        }
        mMip->GenerateMips();
    }
}

bool ModifierFound(const char *filename, const char *key) {
    return strstr(filename, MakeString("%s.", key))
        || strstr(filename, MakeString("%s_", key));
}

bool RndBitmap::ProcessFlags(const char *filename, bool wantMips) {
    if (ModifierFound(filename, "_tb")) {
        SetAlpha(kTransparentBlack);
    } else if (ModifierFound(filename, "_gw")) {
        SetAlpha(kGrayscaleWhite);
    } else if (ModifierFound(filename, "_ga")) {
        SetAlpha(kGrayscaleAlpha);
    }

    if (ModifierFound(filename, "_pma")) {
        SetPreMultipliedAlpha();
    }
    if (ModifierFound(filename, "_selfmip")) {
        SelfMip();
    } else if (wantMips) {
        if (!ModifierFound(filename, "_nomip")) {
            GenerateMips();
        }
    } else if (ModifierFound(filename, "_mip")) {
        GenerateMips();
    }
    return true;
}

void RndBitmap::Blt(
    const RndBitmap &bm, int dX, int dY, int sX, int sY, int width, int height
) {
    MILO_ASSERT(dX + width <= mWidth, 1728);
    MILO_ASSERT(dY + height <= mHeight, 1729);
    MILO_ASSERT(sX + width <= bm.Width(), 1730);
    MILO_ASSERT(sY + height <= bm.Height(), 1731);
    if (SamePixelFormat(bm)) {
        if (mOrder & 0x38) {
            MILO_ASSERT(!((dX | dY | sX | sY | width | height) & 0x3), 0x6CC);
        }
        int count = width * mBpp >> 3;
        for (; height > 0; height--, dY++, sY++) {
            void *dst = mPixels + (dY * mRowBytes) + (dX * mBpp >> 3);
            void *src = bm.mPixels + (sY * bm.RowBytes()) + (sX * bm.Bpp() >> 3);
            memcpy(dst, src, count);
        }
    } else {
        if (mOrder & 0x38) {
            MILO_NOTIFY(
                "RndBitmap::Blt: Can't blt to DXT formatted textures, changing to rgba."
            );
            Create(mWidth, mHeight, 0, 0x20, 0, 0, 0, 0);
        }
        if (mPalette && bm.Palette()) {
            unsigned char colorBuffer[256];
            int i = bm.NumPaletteColors() - 1;
            unsigned char *idx = colorBuffer + i;
            for (; i >= 0; i--, idx--) {
                unsigned char r, g, b, a;
                bm.PaletteColor(i, r, g, b, a);
                *idx = NearestColor(r, g, b, a);
            }
            for (int h = height, dy = dY, sy = sY; h > 0; h--, dy++, sy++) {
                for (int w = width, sx = sX, dx = dX; w > 0; w--, sx++, dx++) {
                    SetPixelIndex(dx, dy, colorBuffer[bm.PixelIndex(sx, sy)]);
                }
            }
        } else {
            for (int h = height, dy = dY, sy = sY; h > 0; h--, dy++, sy++) {
                for (int w = width, sx = sX, dx = dX; w > 0; w--, sx++, dx++) {
                    unsigned char r, g, b, a;
                    bm.PixelColor(sx, sy, r, g, b, a);
                    SetPixelColor(dx, dy, r, g, b, a);
                }
            }
        }
    }
}

void RndBitmap::SaveBmpPixels(BinStream *file) const {
    for (int i = mHeight - 1; i >= 0; i--) {
        u8 *pixels = mPixels + mRowBytes * i;
        if (mBpp == 4) {
            u8 *pixelIt = pixels;
            for (; pixelIt != pixels + mRowBytes; pixelIt++) {
                unsigned char pix = ((*pixelIt & 0xF0) >> 4) | ((*pixelIt & 0x0F) << 4);
                *file << pix;
            }
        } else {
            file->Write(pixels, mRowBytes);
        }
    }
}

void RndBitmap::SetMip(RndBitmap *bm) {
    RndBitmap *mip = mMip;
    delete mip;
    mMip = 0;
    if (bm) {
        MILO_ASSERT(mWidth / 2 == bm->Width(), 0x435);
        MILO_ASSERT(mHeight / 2 == bm->Height(), 0x436);
        MILO_ASSERT(mOrder == bm->Order(), 0x437);
        MILO_ASSERT(mBpp == bm->Bpp(), 0x438);
        mMip = bm;
    }
}

void RndBitmap::SaveBmpHeader(BinStream *file) const {
    tagBITMAPFILEHEADER fileheader;
    tagBITMAPINFOHEADER infoheader;

    MILO_ASSERT(file, 0x524);
    unsigned short us = 0x4D42; // "BM" in ASCII, used to identify that this is a bmp file
    *file << us;
    fileheader.bfOffBits = PaletteBytes() + 54;
    fileheader.bfSize = fileheader.bfOffBits + PixelBytes();
    fileheader.bfReserved1 = 0;
    fileheader.bfReserved2 = 0;
    *file << fileheader;

    infoheader.biSize = 40;
    infoheader.biWidth = mWidth;
    infoheader.biHeight = mHeight;
    infoheader.biPlanes = 1;
    infoheader.biBitCount = mBpp;
    infoheader.biCompression = 0;
    infoheader.biSizeImage = 0;
    infoheader.biXPelsPerMeter = 0xb11;
    infoheader.biYPelsPerMeter = 0xb11;
    infoheader.biClrUsed = 0;
    infoheader.biClrImportant = 0;
    *file << infoheader;
    if (mPalette) {
        file->Write(mPalette, (1 << mBpp) << 2);
    }
}

void RndBitmap::SetPaletteColor(
    int idx, unsigned char r, unsigned char g, unsigned char b, unsigned char a
) {
    ConvertColor(r, g, b, a, mPalette + PaletteOffset(idx) * 4);
}

void RndBitmap::Save(BinStream &bs) const {
    SaveHeader(bs);
    if (mPalette) {
        bs.Write(mPalette, PaletteBytes());
    }
    const RndBitmap *m = this;
    while (m) {
        WriteChunks(bs, m->Pixels(), m->PixelBytes(), 0x8000);
        m = m->mMip;
    }
}

void RndBitmap::PixelColor(
    int x, int y, unsigned char &r, unsigned char &g, unsigned char &b, unsigned char &a
) const {
    if (mPalette) {
        PaletteColor(PixelIndex(x, y), r, g, b, a);
    } else if (mOrder & 0x38) {
        DxtColor(x, y, r, g, b, a);
    } else {
        bool boolbool;
        const unsigned char *p = mPixels + PixelOffset(x, y, boolbool);
        ConvertColor(p, r, g, b, a);
    }
}

void RndBitmap::SetPixelColor(
    int x, int y, unsigned char r, unsigned char g, unsigned char b, unsigned char a
) {
    if (mPalette) {
        SetPixelIndex(x, y, NearestColor(r, g, b, a));
    } else {
        bool boolbool;
        unsigned char *p = mPixels + PixelOffset(x, y, boolbool);
        ConvertColor(r, g, b, a, p);
    }
}

void RndBitmap::ConvertToAlpha() {
    if (mBpp == 24) {
        RndBitmap bmap;
        bmap.Create(*this, 32, mOrder, 0);
        if (mBuffer) {
            MemFree(mBuffer, __FILE__, 0x328);
            mBuffer = nullptr;
        }
        mPalette = bmap.mPalette;
        mPixels = bmap.mPixels;
        mBuffer = bmap.mBuffer;
        mBpp = bmap.mBpp;
        mRowBytes = bmap.mRowBytes;
        bmap.mBuffer = nullptr;
    }
}

int RndBitmap::NumPaletteColors() const {
    if (mPalette)
        return 1 << mBpp;
    else
        return 0;
}

void RndBitmap::SetAlpha(AlphaFlag flag) {
    ConvertToAlpha();
    if (mBpp <= 8) {
        int max = 255;
        int min = 0;
        for (int i = NumPaletteColors() - 1; i >= 0; i--) {
            unsigned char r, g, b, a;
            PaletteColor(i, r, g, b, a);
            switch (flag) {
            case kTransparentBlack:
                if (b == 0 && r == 0 && g == 0) {
                    a = min;
                }
                break;
            case kGrayscaleWhite:
                a = r;
                b = max;
                g = max;
                r = max;
                break;
            case kGrayscaleAlpha:
                a = r;
                break;
            default:
                break;
            }
            SetPaletteColor(i, r, g, b, a);
        }
    } else {
        for (int i = 0; i < mHeight; i++) {
            for (int j = 0; j < mWidth; j++) {
                unsigned char r, g, b, a;
                PixelColor(j, i, r, g, b, a);
                switch (flag) {
                case kTransparentBlack:
                    if (b == 0 && r == 0 && g == 0) {
                        a = 0;
                    }
                    break;
                case kGrayscaleWhite:
                    a = r;
                    b = 255;
                    g = 255;
                    r = 255;
                    break;
                case kGrayscaleAlpha:
                    a = r;
                    break;
                default:
                    break;
                }
                SetPixelColor(j, i, r, g, b, a);
            }
        }
    }
}

bool RndBitmap::LoadBmp(const char *filename, bool wantMips, bool noAlpha) {
    FileStream *stream = new FileStream(filename, FileStream::kRead, true);
    if (stream->Fail()) {
        delete stream;
        return false;
    } else {
        if (!LoadBmp(stream)) {
            delete stream;
            return false;
        } else {
            delete stream;
            if (!noAlpha) {
                ProcessFlags(filename, wantMips);
            }
            return true;
        }
    }
}
