#include "rndobj/Font.h"
#include "os/Debug.h"
#include "os/System.h"
#include "rndobj/Bitmap.h"
#include "rndobj/FontBase.h"
#include "obj/Object.h"
#include "rndobj/Mat.h"
#include "rndobj/Tex.h"
#include "utl/BinStream.h"
#include "math/Rot.h"
#include "utl/FilePath.h"
#include "utl/MakeString.h"
#include "utl/UTF8.h"
#include <cmath>

KerningTable::KerningTable() : mNumEntries(0), mEntries(0) { memset(mTable, 0, 0x80); }
KerningTable::~KerningTable() { delete mEntries; }

KerningTable::Entry *KerningTable::Find(unsigned short us1, unsigned short us2) {
    if (mNumEntries == 0)
        return nullptr;
    else {
        Entry *entry = mTable[TableIndex(us1, us2)];
        int key = Key(us1, us2);
        for (; entry != nullptr && key != entry->key; entry = entry->next)
            ;
        return entry;
    }
}

float KerningTable::Kerning(unsigned short us1, unsigned short us2) {
    Entry *kerningEntry = Find(us1, us2);
    if (kerningEntry)
        return kerningEntry->kerning;
    else
        return 0;
}

bool KerningTable::Valid(const RndFont::KernInfo &info, RndFontBase *font) {
    return !font || (font->CharDefined(info.unk0) && font->CharDefined(info.unk2));
}

void KerningTable::Save(BinStream &bs) {
    bs << mNumEntries;
    for (int i = 0; i < mNumEntries; i++) {
        bs << mEntries[i].key;
        bs << mEntries[i].kerning;
    }
}

void KerningTable::SetKerning(
    const std::vector<RndFont::KernInfo> &info, RndFontBase *font
) {
    int validcount = 0;
    int i = 0;
    for (i = 0; i < info.size(); i++) {
        if (Valid(info[i], font)) {
            validcount++;
        }
    }
    if (validcount != mNumEntries) {
        mNumEntries = validcount;
        delete[] mEntries;
        mEntries = new Entry[mNumEntries];
    }
    memset(mTable, 0, 0x80);
    int entryIdx = 0;
    for (i = 0; i < info.size(); i++) {
        const RndFont::KernInfo &curInfo = info[i];
        if (Valid(curInfo, font)) {
            Entry &curEntry = mEntries[entryIdx++];
            curEntry.key = Key(curInfo.unk0, curInfo.unk2);
            curEntry.kerning = curInfo.kerning;
            int index = TableIndex(curInfo.unk0, curInfo.unk2);
            curEntry.next = mTable[index];
            mTable[index] = &curEntry;
        }
    }
}

void KerningTable::GetKerning(std::vector<RndFontBase::KernInfo> &info) const {
    info.resize(mNumEntries);
    for (int i = 0; i < mNumEntries; i++) {
        info[i].unk0 = mEntries[i].key;
        info[i].unk2 = (unsigned int)(mEntries[i].key) >> 16;
        info[i].kerning = mEntries[i].kerning;
    }
}

void KerningTable::Load(BinStreamRev &d, RndFontBase *f) {
    if (d.rev < 7) {
        std::vector<RndFontBase::KernInfo> info;
        d >> info;
        SetKerning(info, f);
    } else {
        int num;
        d >> num;
        if (num != mNumEntries) {
            mNumEntries = num;
            delete mEntries;
            mEntries = new Entry[mNumEntries];
        }
        memset(&mTable, 0, 0x80);
        for (int i = 0; i < mNumEntries; i++) {
            Entry &curEntry = mEntries[i];
            d >> curEntry.key;
            d >> curEntry.kerning;
            unsigned short us4, us3;
            if (d.rev < 0x11) {
                us4 = curEntry.key & 0xFF;
                us3 = curEntry.key >> 8 & 0xFF;
                curEntry.key = Key(us4, us3);
            } else {
                us4 = curEntry.key;
                us3 = curEntry.key >> 16;
            }
            int idx = TableIndex(us4, us3);
            curEntry.next = mTable[idx];
            mTable[idx] = &curEntry;
        }
    }
}

BitmapLocker::BitmapLocker(RndFont *font, int pageIdx) : mFont(font), mTex(0), unk8(0) {
    LoadPage(pageIdx);
}

BitmapLocker::~BitmapLocker() {
    if (mTex) {
        mTex->UnlockBitmap();
    }
}

void BitmapLocker::LoadPage(int pageIdx) {
    if (mTex) {
        mTex->UnlockBitmap();
    }
    unk8 = nullptr;
    mTex = mFont->ValidTexture(pageIdx);
    if (mTex) {
        const char *filename = mTex->File().c_str();
        int len = strlen(filename);
        if (UsingCD() || len < 4 || stricmp(filename + len - 4, ".bmp")) {
            mTex->LockBitmap(unkc, 3);
            if (unkc.Pixels()) {
                unk8 = &unkc;
            }
        } else {
            unkc.LoadBmp(filename, false, true);
            if (unkc.Pixels()) {
                unk8 = &unkc;
            }
            mTex = nullptr;
        }
    }
}

RndFont::RndFont()
    : mMats(this, (EraseMode)0, kObjListNoNull), mTextureOwner(this, this),
      mCellSize(1, 1), mDeprecatedSize(0), mPacked(0) {}

RndFont::~RndFont() { RELEASE(mKerningTable); }

bool RndFont::Replace(ObjRef *from, Hmx::Object *to) {
    if (&mTextureOwner == from) {
        if (mTextureOwner == this) {
            mTextureOwner = this;
        } else {
            RndFont *fontTo = dynamic_cast<RndFont *>(to);
            if (fontTo) {
                mTextureOwner = fontTo->mTextureOwner.Ptr();
            } else {
                mTextureOwner = this;
            }
        }
        return true;
    } else
        return Hmx::Object::Replace(from, to);
}

BEGIN_HANDLERS(RndFont)
    HANDLE_EXPR(texture_owner, mTextureOwner.Ptr())
    HANDLE_ACTION(bleed_test, BleedTest())
    HANDLE_SUPERCLASS(RndFontBase)
END_HANDLERS

BEGIN_PROPSYNCS(RndFont)
    SYNC_PROP_MODIFY(texture_owner, mTextureOwner, UpdateChars())
    SYNC_PROP_MODIFY(mats, mMats, UpdateChars())
    SYNC_PROP_MODIFY(monospace, mMonospace, UpdateChars())
    SYNC_PROP_MODIFY(packed, mPacked, UpdateChars())
    SYNC_PROP_SET(cell_width, (int)mCellSize.x, SetCellSize(_val.Int(), mCellSize.y))
    SYNC_PROP_SET(cell_height, (int)mCellSize.y, SetCellSize(mCellSize.x, _val.Int()))
    SYNC_PROP_SET(chars_in_map, GetASCIIChars(), SetASCIIChars(_val.Str()))
    SYNC_PROP_MODIFY(base_kerning, mBaseKerning, UpdateChars())
    SYNC_SUPERCLASS(RndFontBase)
END_PROPSYNCS

BEGIN_SAVES(RndFont)
    SAVE_REVS(0x11, 2)
    SAVE_SUPERCLASS(RndFontBase)
    bs << mMats;
    bs << mCellSize << mDeprecatedSize;
    bs << mTextureOwner;
    bs << mPacked;
    RndTex *validTex = ValidTexture(0);
    if (validTex) {
        bs << validTex->Width() << validTex->Height();
    } else {
        bs << 0 << 0;
    }
    bs << unk98;
    bs << mCharInfoMap.size();
    FOREACH (it, mCharInfoMap) {
        bs << it->first;
        CharInfo &info = it->second;
        bs << info.unk0;
        bs << info.unk4;
        bs << info.unk8;
        bs << info.charWidth;
        bs << info.charSpacing;
    }
END_SAVES

BEGIN_COPYS(RndFont)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY_AS(RndFont, f)
    MILO_ASSERT(f, 0x451);
    COPY_MEMBER_FROM(f, mMats)
    COPY_MEMBER_FROM(f, mCellSize)
    COPY_MEMBER_FROM(f, unk98)
    COPY_MEMBER_FROM(f, mDeprecatedSize)
    COPY_MEMBER_FROM(f, mPacked)
    COPY_MEMBER_FROM(f, mCharInfoMap)
    if (ty == kCopyShallow || (ty == kCopyFromMax && f->mTextureOwner != f)) {
        mTextureOwner = f->mTextureOwner.Ptr();
    } else {
        mTextureOwner = this;
    }
END_COPYS

struct MatChar {
    float width;
    float height;
};

__forceinline BinStream &operator>>(BinStream &bs, MatChar &mc) {
    char x[0x80];
    bs.ReadString(x, 0x80);
    bs >> mc.width;
    bs >> mc.height;
    return bs;
}

__forceinline BinStreamRev &operator>>(BinStreamRev &d, RndFontBase::KernInfo &info) {
    if (d.rev < 0x11) {
        char x;
        d >> x;
        info.unk0 = x;
        d >> x;
        info.unk2 = x;
    } else {
        d >> info.unk0 >> info.unk2;
    }
    if (d.rev < 6) {
        char x;
        d >> x >> x;
    }
    d >> info.kerning;
    return d;
}

INIT_REVS(0x11, 0)

BEGIN_LOADS(RndFont)
    LOAD_REVS(bs)
    ASSERT_REVS(0x11, 2)
    if (d.altRev < 2) {
        if (d.rev > 7) {
            LOAD_SUPERCLASS(Hmx::Object)
        }
    } else {
        LOAD_SUPERCLASS(RndFontBase)
    }
    char buf[0x80];
    if (d.rev < 3) {
        String str;
        int a, b, c, e;
        bool dd;
        d >> a >> b >> c >> dd >> e >> str;
    }
    if (d.rev < 1) {
        std::map<char, MatChar> charMap;
        d >> charMap;
    } else {
        if (d.altRev < 1) {
            ObjPtr<RndMat> mat(this);
            d >> mat;
            if (d.rev > 9 && d.rev < 0xc) {
                d.stream.ReadString(buf, 0x80);
                if (!mat && buf[0] != '\0') {
                    mat = LookupOrCreateMat(buf, Dir());
                }
            }
            mMats.clear();
            mMats.push_back(mat);
        } else {
            d >> mMats;
        }
        if (d.rev < 4) {
            float w, h;
            if (d.rev < 2) {
                int iW, iH;
                d >> iH >> iW;
                h = iH;
                w = iW;
            } else {
                d >> h;
                d >> w;
            }
            RndTex *validTex = ValidTexture(0);
            if (validTex) {
                RndBitmap bmap;
                validTex->LockBitmap(bmap, 3);
                mCellSize.x = std::floor((float)bmap.Width() / w + 0.5f);
                mCellSize.y = std::floor((float)bmap.Height() / h + 0.5f);
                validTex->UnlockBitmap();
            }
        } else {
            d >> mCellSize;
        }
        d >> mDeprecatedSize;
        if (d.altRev < 2) {
            d >> mBaseKerning;
        }
        if (d.rev < 4) {
            mBaseKerning /= mDeprecatedSize;
        }
    }
    if (d.rev > 1) {
        if (d.rev < 0x11) {
            String str;
            d >> str;
            ASCIItoWideVector(mChars, str.c_str());
        } else if (d.altRev < 2) {
            d >> mChars;
        }
    } else {
        const char theChars[] =
            " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
        for (const char *ptr = theChars; *ptr != '\0'; ptr++) {
            mChars.push_back(*ptr);
        }
    }
    if (d.rev > 4 && d.altRev < 2) {
        bool hasKerning;
        d >> hasKerning;
        if (hasKerning) {
            mKerningTable = new KerningTable();
            mKerningTable->Load(d, this);
        }
    }
    if (d.rev > 8) {
        d >> mTextureOwner;
    }
    if (!mTextureOwner) {
        mTextureOwner = this;
    }
    if (d.rev > 0xa && d.altRev < 2) {
        d >> mMonospace;
    }
    if (d.rev > 0xe) {
        d >> mPacked;
    }
    if (d.rev > 0xc) {
        int bw, bh;
        d >> bw >> bh;
        RndTex *validTex = ValidTexture(0);
        if (validTex) {
            if (bw && validTex->Width()) {
                mCellSize.x *= (float)validTex->Width() / (float)bw;
            }
            if (bh && validTex->Height()) {
                mCellSize.y *= (float)validTex->Height() / (float)bh;
            }
        }
    }
    unk98.resize(mMats.size());
    if (d.rev > 0xd) {
        if (d.altRev < 1) {
            d >> unk98[0];
        } else {
            d >> unk98;
        }
        if (d.rev < 0x11) {
            for (int i = 0; i < 0x100; i++) {
                CharInfo &info = mCharInfoMap[i];
                info.unk0 = 0;
                d >> info.unk4;
                d >> info.unk8;
                d >> info.charWidth;
                if (info.charWidth < 0) {
                    info.charWidth = 0;
                }
                if (d.rev > 0xe) {
                    d >> info.charSpacing;
                } else {
                    info.charSpacing = info.charWidth;
                }
                if (info.charSpacing < 0) {
                    info.charSpacing = 0;
                }
            }
        } else {
            unsigned int count;
            d >> count;
            for (unsigned int i = 0; i < count; i++) {
                unsigned short keyChar;
                d >> keyChar;
                CharInfo &info = mCharInfoMap[keyChar];
                if (d.altRev > 0) {
                    d >> info.unk0;
                } else {
                    info.unk0 = 0;
                }
                d >> info.unk4;
                d >> info.unk8;
                d >> info.charWidth;
                d >> info.charSpacing;
            }
        }
    } else {
        MILO_LOG("NOTIFY: %s is old version, please resave\n", PathName(this));
        UpdateChars();
    }
    mCharInfoMap[0x20];
    mCharInfoMap[0xa0];
    mCharInfoMap[0xa0] = mCharInfoMap[0x20];
    if (d.rev < 0x10) {
        std::vector<KernInfo> kernInfos;
        GetKerning(kernInfos);
        SetKerning(kernInfos);
        MILO_LOG("NOTIFY: %s is old version, resave file\n", PathName(this));
    }
    if (d.rev > 0x10 && d.altRev < 1) {
        ObjPtr<RndFont> nextFont(this);
        d >> nextFont;
    }
END_LOADS

float RndFont::CharWidth(unsigned short c) const {
    MILO_ASSERT(HasChar(c), 0x143);
    CharInfo &info = mTextureOwner->mCharInfoMap[c];
    float w = info.charWidth;
    MILO_ASSERT(w >= 0, 0x146);
    return w;
}

bool RndFont::CharAdvance(unsigned short u1, unsigned short c, float &f3) const {
    if (mTextureOwner != this) {
        return mTextureOwner->CharAdvance(u1, c, f3);
    } else {
        auto it = mCharInfoMap.find(c);
        if (it != mCharInfoMap.end()
            && (it->second.unk4 != 0 || it->second.unk8 != 0
                || it->second.charSpacing != 0)) {
            f3 = mMonospace ? 1 : it->second.charSpacing;
            f3 += Kerning(u1, c);
            return true;
        }
        return false;
    }
}

float RndFont::CharAdvance(unsigned short c) const {
    MILO_ASSERT(HasChar(c), 0x14E);
    if (mMonospace) {
        return 1;
    } else {
        return mTextureOwner->mCharInfoMap[c].charSpacing;
    }
}

bool RndFont::CharDefined(unsigned short c) const {
    if (HasChar(c)) {
        auto it = mCharInfoMap.find(c);
        const CharInfo &info = it->second;
        return info.unk4 != 0 || info.unk8 != 0 || info.charSpacing != 0;
    } else {
        return false;
    }
}

void RndFont::Print() const {
    TheDebug << "   pages: " << mMats.size() << "\n";
    TheDebug << "   mats: \n";
    FOREACH (it, mMats) {
        TheDebug << "         " << *it << "\n";
    }
    TheDebug << "   cellSize: " << mCellSize << "\n";
    TheDebug << "   deprecated size: " << mDeprecatedSize << "\n";
    TheDebug << "   space: " << mBaseKerning << "\n";
    TheDebug << "   chars: ";
    for (int i = 0; i < mChars.size(); i++) {
        unsigned short us = mChars[i];
        TheDebug << WideCharToChar(&us);
    }
    TheDebug << "\n";
    TheDebug << "   kerning: TODO\n";
}

bool RndFont::HasChar(unsigned short c) const {
    return mCharInfoMap.find(c) != mCharInfoMap.end();
}

void RndFont::SetASCIIChars(String str) {
    RndFontBase::SetASCIIChars(str);
    UpdateChars();
}

RndMat *RndFont::Mat(int idx) const {
    if (idx >= 0 && idx < mMats.size()) {
        return (RndMat *)mMats[idx];
    } else
        return nullptr;
}

RndTex *RndFont::ValidTexture(int idx) const {
    if (Mat(idx)) {
        return Mat(idx)->GetDiffuseTex();
    } else
        return nullptr;
}

void RndFont::SetCellSize(float x, float y) {
    mCellSize.Set(x, y);
    UpdateChars();
}

int RndFont::CharPage(unsigned short c) const {
    if (HasChar(c)) {
        return mCharInfoMap.find(c)->second.unk0;
    } else {
        return -1;
    }
}

void RndFont::SetBitmapSize(const Vector2 &size) {
    mCellSize = size;
    if (unk98.size() != mMats.size()) {
        unk98.resize(mMats.size());
    }
    for (int i = 0; i < mMats.size(); i++) {
        RndMat *curMat = mMats[i];
        RndTex *curTex = curMat ? curMat->GetDiffuseTex() : nullptr;
        if (curTex && curTex->Width() && curTex->Height()) {
            unk98[i].x = mCellSize.x / (float)curTex->Width();
            unk98[i].y = mCellSize.y / (float)curTex->Height();
        }
    }
}

bool RndFont::CharWidthAdvanceCoords(
    unsigned short key, float &f1, float &f2, Vector2 &v1, Vector2 &v2
) const {
    const RndFont *font;
    for (font = this; font->mTextureOwner != font; font = font->mTextureOwner)
        ;
    auto it = font->mCharInfoMap.find(key);
    if (it != font->mCharInfoMap.end()) {
        const CharInfo &cur = it->second;
        if (cur.unk4 || cur.unk8 || cur.charSpacing) {
            f1 = cur.charWidth;
            f2 = mMonospace ? 1 : cur.charSpacing;
            v1.x = cur.unk4;
            v2.x = unk98[cur.unk0].x * cur.charWidth + cur.unk4;
            v1.y = cur.unk8;
            v2.y = unk98[cur.unk0].y + cur.unk8;
            return true;
        }
    }
    return false;
}

void RndFont::SetCharInfo(CharInfo *info, RndBitmap &bmap, const Vector2 &v2, int i4) {
    info->unk0 = i4;
    if (mMonospace) {
        info->charSpacing = 1;
        info->charWidth = 1;
        info->unk4 = v2.x / (float)bmap.Width();
    } else {
        int vx = v2.x;
        int vy = v2.y;
        int addx = mCellSize.x + v2.x;
        int addy = mCellSize.y + v2.y;
        int yPtr = addy;
        int i9 = vx;
        while (i9 != addx && !bmap.ColumnNonTransparent(i9, vy, addy, &yPtr)) {
            if (addx > vx) {
                i9++;
            } else {
                i9--;
            }
        }
        yPtr = i9;
        float f9 = i9;
        i9 = addx - 1;
        while (i9 != vx - 1 && !bmap.ColumnNonTransparent(i9, vy, addy, &yPtr)) {
            if (vx - 1 > addx - 1) {
                i9++;
            } else {
                i9--;
            }
        }
        float f10 = ((float)i9 + 1.0f) - f9;
        if (f10 <= 0) {
            float bW = bmap.Width();
            info->charSpacing = 0.25f;
            info->charWidth = 0.25f;
            info->unk4 = v2.x / bW;
        } else {
            float bW = bmap.Width();
            info->unk4 = f9 / bW;
            float w = f10 / mCellSize.x;
            info->charWidth = w;
            info->charSpacing = w;
        }
    }
    info->unk8 = v2.y / (float)bmap.Height();
    MILO_ASSERT(info->charWidth >= 0, 0x1A6);
}

void RndFont::UpdateChars() {
    if (mPacked) {
        SetBitmapSize(mCellSize);
    } else {
        if (!mChars.empty()) {
            if (mChars[0] == 160) {
                MILO_NOTIFY(
                    "%s: first character is ascii 160, converting to the space character.",
                    mChars[0]
                );
                mChars[0] = L' ';
            }
        }
        mCharInfoMap.clear();
        int i12 = 0;
        BitmapLocker locker(this, 0);
        RndBitmap *bmap = locker.Unk8();
        if (bmap) {
            if (unk98.size() != mMats.size()) {
                unk98.resize(mMats.size());
            }
            Vector2 v120(0, 0);
            unk98[0].x = mCellSize.x / (float)bmap->Width();
            unk98[0].y = mCellSize.y / (float)bmap->Height();
            for (int i = 0; i < mChars.size(); i++) {
            }
        }
    }
}
