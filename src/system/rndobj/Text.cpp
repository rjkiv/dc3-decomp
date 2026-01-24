#include "rndobj/Text.h"
#include "Text.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/System.h"
#include "rndobj/Draw.h"
#include "rndobj/Font.h"
#include "rndobj/FontBase.h"
#include "rndobj/Mat.h"
#include "rndobj/Mesh.h"
#include "rndobj/Trans.h"
#include "utl/BinStream.h"
#include "utl/MemMgr.h"
#include "utl/UTF8.h"
#include "wordwrap.h"

std::vector<RndText::BlacklightPacket> RndText::sBlacklightPacketPool;
std::list<RndText::FontMapBase *> RndText::sFontMapCache;
int TEXT_REV = 0;
float gSuperscriptScale = 0.7f;
float gGuitarScale = 0.7f;
float gGuitarZOffset = 0.2f;

RndText::RndText()
    : mWidth(0), mHeight(0), mCircle(0), mAlign(kMiddleCenter), mFitType(kFitWrap),
      mCapsMode(kCapsModeNone), mLeading(1), mFixedLength(0), mMarkup(true),
      mBasicMarkup(true), mScrollDelay(0), mScrollRate(1), mScrollPause(0), unk40(0),
      unk58(0), unk5c(0), unk60(0), mIndentation(0), unk78(nullptr), unk8c(0), unk90(-1),
      unk94(-1), mStyles(this), unkb4(0), unkb8(0), unkbc(0), unkc0(0), unkc4(0),
      unkc8(0) {
    mStyles.resize(1);
    mFontMaps.reserve(1);
}

RndText::~RndText() {
    FOREACH (it, mFontMaps) {
        delete *it;
    }
}

BEGIN_HANDLERS(RndText)
    HANDLE_EXPR(get_text_size, GetTextSize())
    HANDLE_ACTION(update_text, UpdateText())
    HANDLE_SUPERCLASS(RndDrawable)
    HANDLE_SUPERCLASS(RndTransformable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_CUSTOM_PROPSYNC(RndText::Style)
    SYNC_PROP(font, o.mFont)
    SYNC_PROP(size, o.mSize)
    SYNC_PROP_SET(text_color, o.mTextColor.Pack(), o.mTextColor.Unpack(_val.Int()))
    SYNC_PROP_SET(text_alpha, o.mTextColor.alpha, o.mTextColor.alpha = _val.Float())
    SYNC_PROP(font_color_override, o.mFontColorOverride)
    SYNC_PROP_SET(font_color, o.mFontColor.Pack(), o.mFontColor.Unpack(_val.Int()))
    SYNC_PROP_SET(font_alpha, o.mFontColor.alpha, o.mFontColor.alpha = _val.Float())
    SYNC_PROP(italics, o.mItalics)
    SYNC_PROP(kerning, o.mKerning)
    SYNC_PROP(z_offset, o.mZOffset)
    SYNC_PROP(blacklight, o.mBlacklight)
END_CUSTOM_PROPSYNC

BEGIN_PROPSYNCS(RndText)
    SYNC_PROP_SET(text, TextASCII(), SetTextASCII(_val.Str()))
    SYNC_PROP_SET(fixed_length, mFixedLength, SetFixedLength(_val.Int()))
    SYNC_PROP(align, (int &)mAlign)
    SYNC_PROP(caps_mode, (int &)mCapsMode)
    SYNC_PROP(width, mWidth)
    SYNC_PROP(height, mHeight)
    SYNC_PROP(circle, mCircle)
    SYNC_PROP(fit_type, (int &)mFitType)
    SYNC_PROP(leading, mLeading)
    SYNC_PROP(indentation, mIndentation)
    SYNC_PROP(basic_markup, mBasicMarkup)
    SYNC_PROP(markup, mMarkup)
    SYNC_PROP(scroll_delay, mScrollDelay)
    SYNC_PROP(scroll_rate, mScrollRate)
    SYNC_PROP(scroll_pause, mScrollPause)
    SYNC_PROP(styles, mStyles)
    SYNC_SUPERCLASS(RndDrawable)
    SYNC_SUPERCLASS(RndTransformable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BinStream &operator<<(BinStream &bs, const RndText::Style &s) {
    bs << s.mFont;
    bs << s.mSize;
    bs << s.mTextColor;
    bs << s.mFontColorOverride;
    bs << s.mFontColor;
    bs << s.mItalics;
    bs << s.mKerning;
    bs << s.mZOffset;
    bs << s.mBlacklight;
    return bs;
}

BEGIN_SAVES(RndText)
    SAVE_REVS(0x1C, 1)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(RndDrawable)
    SAVE_SUPERCLASS(RndTransformable)
    bs << mAlign;
    bs << mText;
    bs << mWidth;
    bs << mLeading;
    bs << mFixedLength;
    bs << mMarkup;
    bs << mCapsMode;
    bs << mHeight;
    bs << mCircle;
    bs << mFitType;
    bs << mStyles;
    bs << mScrollDelay;
    bs << mScrollRate;
    bs << mScrollPause;
    bs << mIndentation;
    bs << mBasicMarkup;
END_SAVES

BEGIN_COPYS(RndText)
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(RndDrawable)
    COPY_SUPERCLASS(RndTransformable)
    if (ty != kCopyFromMax) {
        CREATE_COPY(RndText)
        BEGIN_COPYING_MEMBERS
            COPY_MEMBER(mAlign)
            COPY_MEMBER(mCapsMode)
            COPY_MEMBER(mFitType)
            COPY_MEMBER(mWidth)
            COPY_MEMBER(mHeight)
            COPY_MEMBER(mCircle)
            COPY_MEMBER(mLeading)
            COPY_MEMBER(mMarkup)
            SetFixedLength(c->mFixedLength);
            SetText(c->mText.c_str());
            COPY_MEMBER(mStyles)
            COPY_MEMBER(mScrollDelay)
            COPY_MEMBER(mScrollRate)
            COPY_MEMBER(mScrollPause)
            COPY_MEMBER(mIndentation)
        END_COPYING_MEMBERS
        UpdateText();
    }
END_COPYS

BinStream &operator>>(BinStream &bs, RndText::Style &s) {
    bs >> s.mFont;
    bs >> s.mSize;
    bs >> s.mTextColor;
    bs >> s.mFontColorOverride;
    bs >> s.mFontColor;
    bs >> s.mItalics;
    bs >> s.mKerning;
    bs >> s.mZOffset;
    if (TEXT_REV >= 0x19) {
        bs >> s.mBlacklight;
    }
    return bs;
}

BEGIN_LOADS(RndText)
    LOAD_REVS(bs)
    ASSERT_REVS(0x1C, 1)
    Style style(this);
    TEXT_REV = d.rev;
    if (d.rev > 0xF) {
        Hmx::Object::Load(bs);
    }
    RndDrawable::Load(bs);
    if (d.rev < 7) {
        ObjPtrList<Hmx::Object> objects(this);
        int x;
        bs >> x;
        bs >> objects;
    }
    if (d.rev > 1) {
        RndTransformable::Load(bs);
    }
    if (d.rev < 0x16) {
        bs >> style.mFont;
    }
    if (d.rev < 3) {
        int idx;
        bs >> idx;
        Alignment align_choices[6] = { kTopLeft,    kTopCenter,    kTopRight,
                                       kBottomLeft, kBottomCenter, kBottomRight };
        mAlign = align_choices[idx];
    } else {
        bs >> (int &)mAlign;
    }
    if (d.rev < 2) {
        Vector2 v2;
        bs >> v2;
        SetLocalPos(Vector3(v2.x, 0, -v2.y * 0.75f));
    }
    bs >> mText;
    if (d.rev < 0x14) {
        std::vector<unsigned short> vec;
        ASCIItoWideVector(vec, mText.c_str());
        WideVectorToUTF8(vec, mText);
    }
    if (d.rev > 0 && d.rev < 0x16) {
        bs >> style.mTextColor;
    }
    if (d.rev > 0xC) {
        bs >> mWidth;
    } else if (d.rev > 3) {
        bool b;
        d >> b;
        bs >> mWidth;
        if (!b)
            mWidth = 0.0f;
        if (d.rev < 5 && (mWidth < 0.0f || mWidth > 1000.0f))
            mWidth = 0.0f;
    }
    if (d.rev == 5) {
        String str;
        bs >> str;
    }
    if (d.rev > 4 && d.rev < 11) {
        bool b;
        d >> b;
        if (style.mFont) {
            RndFont *oldfont2d = dynamic_cast<RndFont *>(style.mFont.Ptr());
            MILO_ASSERT(oldfont2d, 0xBC1);
            if (oldfont2d->NumMats() > 0 && oldfont2d->Mat(0)) {
                int zMode = !mMarkup ? 2 : 0;
                oldfont2d->Mat(0)->SetZMode((ZMode)zMode);
            }
        }
    }
    if (d.rev > 7) {
        bs >> mLeading;
    }
    if (d.rev > 0xB) {
        int len;
        bs >> len;
        SetFixedLength(len);
    } else if (d.rev > 8) {
        bool b;
        d >> b;
        if (b) {
            SetFixedLength(mText.length());
        } else if (mFixedLength != 0) {
            mFixedLength = 0;
        }
    }
    if (d.rev > 9 && d.rev < 0x16) {
        bs >> style.mItalics;
    }
    if (d.rev < 0x16) {
        if (d.rev > 0xB) {
            bs >> style.mSize;
        } else if (style.mFont) {
            RndFont *oldfont2d = dynamic_cast<RndFont *>(style.mFont.Ptr());
            MILO_ASSERT(oldfont2d, 0xBE9);
            style.mSize = oldfont2d->DeprecatedSize();
        }
        if (d.rev < 0xD) {
            style.mItalics /= style.mSize;
        }
    }
    if (d.rev > 0xD) {
        LOAD_BITFIELD(bool, mMarkup)
    }
    if (d.rev > 0xE) {
        bs >> (int &)mCapsMode;
    } else {
        mCapsMode = kCapsModeNone;
    }
    if (d.rev > 0xF) {
        bs >> mHeight;
        bs >> mCircle;
        bs >> (int &)mFitType;
    }
    if (d.rev >= 0x12 && d.rev < 0x15) {
        bool b;
        d >> b;
    }
    if (d.rev >= 0x13 && d.rev < 0x15) {
        int i, j, k;
        bs >> i;
        bs >> j;
        bs >> k;
    }
    if (d.rev >= 0x16) {
        if (d.rev == 0x17) {
            TheDebug.Notify(MakeString("%s was bad version 23, suggest resave", PathName(this)));
        }
        bs >> (int &)mFitType;
        if (d.rev < 0x18) {
            String str;
            bs >> str;
        }
        if (d.altRev > 0) {
            bs >> unk90;
            bs >> unk94;
        }
        d >> mStyles;
    } else {
        mStyles.resize(1);
        memcpy(&mStyles[0], &style, 0x34);
        mStyles[0].mFont = style.mFont;
    }
    if (d.rev >= 0x1A) {
        bs >> mScrollDelay;
        bs >> mScrollRate;
        bs >> mScrollPause;
    }
    if (d.rev >= 0x1B) {
        bs >> mIndentation;
    }
    if (d.rev >= 0x1C) {
        d >> mBasicMarkup;
    }
    UpdateText();
END_LOADS

void RndText::UpdateSphere() {
    Sphere s;
    s.Zero();
    FOREACH (it, mFontMaps) {
        for (int i = 0; i < (*it)->NumMeshes(); i++) {
            RndMesh *mesh = (*it)->Mesh(i);
            if (mesh) {
                mesh->UpdateSphere();
                s.GrowToContain(mesh->GetSphere());
            }
        }
    }
    SetSphere(s);
}

void RndText::Mats(std::list<class RndMat *> &mats, bool) {
    FOREACH (it, mFontMaps) {
        for (int i = 0; i < (*it)->NumMaterials(); i++) {
            RndMat *mat = (*it)->Material(i);
            if (mat) {
                mats.push_back(mat);
            }
        }
    }
}

RndDrawable *RndText::CollideShowing(const Segment &s, float &f, Plane &p) {
    FOREACH (it, mFontMaps) {
        for (int i = 0; i < (*it)->NumMeshes(); i++) {
            RndMesh *mesh = (*it)->Mesh(i);
            if (mesh && mesh->CollideShowing(s, f, p)) {
                return this;
            }
        }
    }
    return nullptr;
}

int RndText::CollidePlane(const Plane &p) {
    int ret = 0;
    FOREACH (it, mFontMaps) {
        for (int i = 0; i < (*it)->NumMeshes(); i++) {
            RndMesh *mesh = (*it)->Mesh(i);
            if (mesh) {
                int meshCol = mesh->CollidePlane(p);
                if (meshCol == 0) {
                    return 0;
                }
                if (meshCol > 0) {
                    if (ret < 0) {
                        return 0;
                    } else {
                        ret = meshCol;
                    }
                } else if (ret > 0) {
                    return 0;
                } else {
                    ret = meshCol;
                }
            }
        }
    }
    return ret;
}

void RndText::Init() {
    REGISTER_OBJ_FACTORY(RndText)
    SystemConfig("rnd")->FindData("text_superscript_scale", gSuperscriptScale, false);
    SystemConfig("rnd")->FindData("text_guitar_scale", gGuitarScale, false);
    SystemConfig("rnd")->FindData("text_guitar_z_offset", gGuitarZOffset, false);
    unsigned int ui = 1;
    static Symbol kor("kor");
    if (SystemLanguage() == kor)
        ui = 5;
    WordWrap_SetOption(ui);
}

RndText::FontMap::~FontMap() {
    while (!mPages.empty()) {
        delete mPages.back();
        mPages.pop_back();
    }
}

void RndText::FontMap::SetFont(RndFontBase *f) {
    MILO_ASSERT(f->ClassName() == RndFont::StaticClassName(), 0x75);
    mFont = static_cast<RndFont *>(f);
    while (mPages.size() > mFont->NumMats()) {
        delete mPages.back();
        mPages.pop_back();
    }
    mPages.reserve(mFont->NumMats());
    while (mPages.size() < mFont->NumMats()) {
        mPages.push_back(new Page());
    }
}

void RndText::FontMap::ResetDisplayableChars() {
    for (int i = 0; i < mPages.size(); i++) {
        mPages[i]->displayableChars = 0;
    }
}

void RndText::FontMap::IncrementDisplayableChars(unsigned short num) {
    int page = mFont->CharPage(num);
    if (page >= 0) {
        mPages[page]->displayableChars++;
    }
}

void ResetFontMapPageMeshFaces(RndMesh *, int);

void RndText::FontMap::AllocateMeshes(RndText *text, int fixedLength) {
    for (int i = 0; i < mPages.size(); i++) {
        Page &page = *(mPages[i]);
        if (!page.mesh && mFont && page.displayableChars > 0) {
            page.mesh = Hmx::Object::New<RndMesh>();
        }
        RndMesh *mesh = page.mesh;
        page.unkc = 0x1F;
        page.unk8 = 0;
        if (mesh) {
            mesh->SetTransParent(text, false);
            mesh->SetTransConstraint(
                RndTransformable::kConstraintParentWorld, nullptr, false
            );
            if (mFont) {
                mesh->SetMat(mFont->Mat(i));
            }
            mesh->SetShowing(page.displayableChars > 0);
            if (fixedLength == 0) {
                mesh->SetMutable(0);
                ResetFontMapPageMeshFaces(mesh, page.displayableChars * 2);
                page.unkc |= 0xA0;
                mesh->Verts().resize(page.displayableChars * 4);
            } else if (mesh->Mutable() == 0 || mesh->Verts().size() != fixedLength * 4) {
                mesh->SetMutable(0x1F);
                ResetFontMapPageMeshFaces(mesh, page.displayableChars * 2);
                page.unkc |= 0xA0;
                mesh->Verts().resize(page.displayableChars * 4);
            }
            MILO_ASSERT(mesh->Verts().size() >= page.displayableChars * 4, 0xD2);
        }
        MILO_ASSERT(!fixedLength || (page.displayableChars <= fixedLength), 0xD5);
    }
}

void RndText::FontMap::CleanupSyncMeshes() {
    for (int i = 0; i < mPages.size(); i++) {
        Page &page = *(mPages[i]);
        RndMesh *mesh = page.mesh;
        if (mesh) {
            for (RndMesh::Vert *it = mesh->Verts().begin(); page.unk8 != it; ++it) {
                RndMesh::Vert *old = page.unk8;
                page.unk8++;
                old->pos.Zero();
            }
            mesh->Sync(page.unkc);
        }
    }
}

void RndText::FontMap::SetupScrolling() {
    for (int i = 0; i < NumMeshes(); i++) {
        RndMesh *mesh = Mesh(i);
        if (mesh) {
            mesh->SetTransConstraint(RndTransformable::kConstraintNone, nullptr, false);
        }
    }
}

void RndText::FontMap::UpdateScrolling(float f1) {
    for (int i = 0; i < NumMeshes(); i++) {
        RndMesh *mesh = Mesh(i);
        if (mesh) {
        }
    }
}

RndText::FontMap3d::~FontMap3d() {
    for (int i = 0; i < mMeshes.size(); i++) {
        RndMesh *mesh = mMeshes[i];
        if (mesh) {
            delete mesh;
        }
    }
}

void RndText::FontMap3d::SetFont(RndFontBase *f) {
    MILO_ASSERT(f->ClassName() == RndFont3d::StaticClassName(), 0x17D);
    mFont = static_cast<RndFont3d *>(f);
}

void RndText::SetFixedLength(int len) {
    if (mFixedLength != len) {
        mFixedLength = len;
        if (mFixedLength != 0) {
            const char *p = mText.c_str();
            int newLen;
            for (newLen = 0; *p != '\0' && newLen < mFixedLength; newLen++) {
                unsigned short us;
                p += DecodeUTF8(us, p);
            }
            mText.resize((int)p + mFixedLength - newLen - (int)mText.c_str());
        }
    }
}

void RndText::DoBasicMarkup() {
    while (mText.contains("\\q")) {
        mText.replace(mText.find("\\q"), 2, "\"");
    }
}

int RndText::FontMapIndex(RndFontBase *f, bool b) {
    for (int i = 0; i < mFontMaps.size(); i++) {
        if (mFontMaps[i]->Font() == f && mFontMaps[i]->mBlacklight == b) {
            return i;
        }
    }
    return -1;
}

float RndText::ComputeHeight(int i1, float f2, float &f3) {
    float f1;
    if (mStyles[0].mFont) {
        f1 = mStyles[0].mFont->AspectRatio() * mStyles[0].mSize * f2;
    } else {
        f1 = 0;
    }
    f3 = mLeading * f1;
    return ((i1 - 1) * mLeading + 1.0f) * f1;
}

void RndText::SetText(const char *str) {
    if (mFixedLength != 0) {
        MILO_ASSERT(mText.capacity() >= mFixedLength, 0x75E);
        const char *p = str;
        for (int newLen = 0; *p != '\0' && newLen < mFixedLength; newLen++) {
            unsigned short us;
            p += DecodeUTF8(us, p);
        }
        int newLen = p - str;
        if (mText.capacity() < newLen) {
            mText.resize(newLen);
        }
        strncpy((char *)mText.c_str(), str, newLen);
        char *last = (char *)mText.c_str() + newLen;
        *last = '\0';
    } else {
        mText = str;
    }
    if (mBasicMarkup) {
        DoBasicMarkup();
    }
}

String RndText::TextASCII() const {
    String str;
    {
        MemTemp tmp;
        str.resize(UTF8StrLen(mText.c_str()) + 1);
    }
    UTF8toASCIIs((char *)str.c_str(), str.capacity(), mText.c_str(), '*');
    return str;
}

void RndText::BuildFontMaps(bool b1) {
    if (b1) {
        for (auto it = mFontMaps.begin(); it != mFontMaps.end();
             it = mFontMaps.erase(it)) {
            sFontMapCache.push_back(*it);
        }
    }
    if (mFontMaps.empty()) {
        for (int i = 0; i < mStyles.size(); i++) {
            RndFontBase *font = mStyles[i].mFont;
            if (font) {
                if (FontMapIndex(font, mStyles[i].mBlacklight) == -1) {
                    FontMapBase *map = AcquireFontMap(font);
                    map->mBlacklight = mStyles[i].mBlacklight;
                    mFontMaps.push_back(map);
                }
            }
        }
    }
}

void RndText::SetTextASCII(const char *cstr) {
    String str;
    {
        MemTemp tmp;
        std::vector<unsigned short> vec;
        ASCIItoWideVector(vec, cstr);
        WideVectorToUTF8(vec, str);
    }
    SetText(str.c_str());
}

void RndText::QueueBlacklightPacket(RndMesh *mesh, float f2, int i3) {
    int cursize = sBlacklightPacketPool.size();
    if (sBlacklightPacketCount >= cursize) {
        int newsize = 8;
        if (cursize != 0) {
            newsize = cursize * 2;
        }
        sBlacklightPacketPool.resize(newsize);
    }
    sBlacklightPacketCount++;
}
