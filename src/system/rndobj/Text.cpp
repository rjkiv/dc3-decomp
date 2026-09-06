#include "rndobj/Text.h"
#include "Text.h"
#include "math/Color.h"
#include "math/Geo.h"
#include "math/Mtx.h"
#include "math/Trig.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "os/Debug.h"
#include "os/System.h"
#include "rndobj/BaseMaterial.h"
#include "rndobj/Cam.h"
#include "rndobj/Draw.h"
#include "rndobj/Font.h"
#include "rndobj/FontBase.h"
#include "rndobj/Mat.h"
#include "rndobj/Mesh.h"
#include "rndobj/Rnd.h"
#include "rndobj/Tex.h"
#include "rndobj/Trans.h"
#include "ui/UI.h"
#include "utl/BinStream.h"
#include "utl/MemMgr.h"
#include "utl/UTF8.h"
#include "wordwrap.h"

std::vector<RndText::BlacklightPacket> RndText::sBlacklightPacketPool;
std::list<RndText::FontMapBase *> RndText::sFontMapCache;
int TEXT_REV = 0;
static float gSuperscriptScale = 0.7f;
static float gGuitarScale = 0.7f;
static float gGuitarZOffset = 0.2f;

float SegmentLength(int i1, int i2, const float *f3, const unsigned short *us4, float f5) {
    for (const unsigned short *p = us4 + i1; *p == 0x20 && i1 < i2; i1++, p++)
        ;
    for (const unsigned short *p = us4 + i2 - 1; *p == 0x20 && i1 < i2; i2--, p--)
        ;
    return (f3[i2] - f3[i1]) * f5;
}

void ResetFontMapPageMeshFaces(RndMesh *mesh, int i2) {
    MILO_ASSERT(mesh, 0x96);
    mesh->Faces().resize(i2);

    auto it = mesh->Faces().begin();
    auto itEnd = mesh->Faces().end();
    int i7 = 0;
    for (; it != itEnd; it += 2, i7 += 4) {
        it[0].Set(i7, i7 + 1, i7 + 2);
        it[1].Set(i7, i7 + 2, i7 + 3);
    }
}

Transform XfmOnCircleEdge(float f1, float f2) {
    Transform out;
    float f10 = f1 < 0 ? -1.0f : 1.0f;
    out.m.z.Set(0, 0, 1);
    float f9 = (f2 / f1) * 2 * PI + (f10 * (-PI / 2));
    out.v.Set(Cosine(f9), Sine(f9), 0);
    Scale(out.v, -f10, out.m.y);
    Cross(out.m.y, out.m.z, out.m.x);
    out.v *= f10 * f1 * 0.15915494f;
    return out;
}

bool CalcScreenHeight(float f1, RndMesh *mesh, float &fref) {
    if (!mesh->Showing()) {
        return false;
    } else {
        const Transform &xfm = mesh->WorldXfm();
        RndCam *cur = RndCam::Current();
        Vector2 v2[2];
        Vector3 v3[2];
        v3[0].Set(0, 0, -f1 / 2);
        v3[1].Set(0, 0, f1 / 2);
        for (int i = 0; i < 2; i++) {
            Vector3 vtmp;
            Multiply(v3[i], xfm, vtmp);
            cur->WorldToScreen(vtmp, v2[i]);
        }
        Vector2 v2res(v2[0].x - v2[1].x, v2[0].y - v2[1].y);
        v2res.x *= TheRnd.Width();
        v2res.y *= TheRnd.Height();
        fref = Length(v2res);
        return true;
    }
}

#pragma region FontMap

RndText::FontMap::~FontMap() {
    while (mPages.size() != 0) {
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
                int numFaces = page.displayableChars * 2;
                mesh->SetMutable(0);
                ResetFontMapPageMeshFaces(mesh, numFaces);
                page.unkc |= 0xA0;
                mesh->Verts().resize(numFaces * 2);
            } else if (
                (mesh->Mutable() & 0x1F) == 0 || mesh->Verts().size() != fixedLength * 4
            ) {
                int numFaces = fixedLength * 2;
                mesh->SetMutable(0x1F);
                ResetFontMapPageMeshFaces(mesh, numFaces);
                page.unkc |= 0xA0;
                mesh->Verts().resize(numFaces * 2);
            }
            page.unk8 = mesh->Verts().begin();
            MILO_ASSERT(mesh->Verts().size() >= page.displayableChars * 4, 0xD2);
        }
        MILO_ASSERT(!fixedLength || (page.displayableChars <= fixedLength), 0xD5);
    }
}

void RndText::FontMap::CleanupSyncMeshes() {
    for (int i = 0; i < mPages.size(); i++) {
        Page *page = mPages[i];
        RndMesh *mesh = page->mesh;
        if (mesh) {
            while (page->unk8 != mesh->Verts().end()) {
                page->unk8++->pos.Set(0, 0, 0);
            }
            mesh->Sync(page->unkc);
        }
    }
}

void RndText::FontMap::SetupCharacter(
    unsigned short us2,
    float &f3,
    float f4,
    const StyleState &state,
    unsigned short us6,
    float circle,
    FitType fitType,
    float indentation
) {
    if (fitType == 7 && us2 == 10) {
        f3 += indentation;
    } else {
        int i5 = mFont->CharPage(us2);
        if (i5 < 0) {
            return;
        }
        Page *page = mPages[i5];
        float fc0, fbc;
        if (!mFont->CharWidthAdvanceCoords(
                us2, fc0, fbc, page->unk8[0].tex, page->unk8[2].tex
            )) {
            return;
        }

        float f11 = mFont->Kerning(us6, us2);
        f3 += f11 + state.mInfo.mKerning * state.mInfo.mSize;
        if (fc0 > 0) {
            f11 = fc0;
        } else {
            f11 = fbc;
        }
        float f8 = 0;
        if (mFont->Monospace()) {
            f8 = Min((fbc - f11) / 2.0f, 0.0f);
        }
        float fvar9 = state.mInfo.mSize;
        fc0 = fvar9 * f11;
        f11 = fvar9 * f8;
        if (fc0 <= 0) {
            return;
        }
        float f13 = state.mInfo.mZOffset * fvar9 + f4;
        float aspect8 = mFont->AspectRatio();
        float f10 = state.mInfo.mItalics * fvar9;
        page->unk8->pos.Set(f10 + f11 + f3, 0, f13);
        float fvar1 = f13 - (aspect8 * indentation);
        reinterpret_cast<Vector3 &>(page->unk8->tangent).Set((f11 + f3) - f10, 0, fvar1);
        reinterpret_cast<Vector3 &>(page->unk8[1].tangent)
            .Set(((f11 + f3) - f10) + fc0, fvar1, 0);
        reinterpret_cast<Vector3 &>(page->unk8[2].tangent)
            .Set(((f11 + f3) + fc0) + fvar1, 0, f13);
        if (circle != 0) {
            float fvar10 = (page->unk8[2].tangent.x - page->unk8[0].tangent.x) / 2
                + page->unk8[0].tangent.x;
            Transform xfm = XfmOnCircleEdge(circle, fvar10);
            Vector3 tmp;
            Scale(xfm.m.x, fvar10, tmp);
            xfm.v -= tmp;
            Multiply(page->unk8[0].pos, xfm, page->unk8[0].pos);
            Multiply(page->unk8[1].pos, xfm, page->unk8[1].pos);
            Multiply(page->unk8[2].pos, xfm, page->unk8[2].pos);
            Multiply(page->unk8[3].pos, xfm, page->unk8[3].pos);
        }
        page->unk8[1].tex.Set(page->unk8[0].tex.x, page->unk8[2].tex.y);
        page->unk8[3].tex.Set(page->unk8[2].tex.x, page->unk8[0].tex.y);
        page->unk8[0].norm.Set(0, -1, 0);
        page->unk8[1].norm = page->unk8[2].norm = page->unk8[3].norm = page->unk8[0].norm;
        page->unk8[0].color = page->unk8[1].color = page->unk8[2].color =
            page->unk8[3].color = state.mInfo.mTextColor;
        // advance to the next set of 4 verts
        page->unk8 = &page->unk8[4];
        f3 += state.mInfo.mSize * fbc;
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
            Vector3 pos = mesh->LocalXfm().v;
            pos.x = f1;
            mesh->SetLocalPos(pos);
        }
    }
}

RndText::FontMap3d::~FontMap3d() {
    for (int i = 0; i < mMeshes.size(); i++) {
        if (mMeshes[i]) {
            delete mMeshes[i];
        }
    }
}

void RndText::FontMap3d::SetFont(RndFontBase *f) {
    MILO_ASSERT(f->ClassName() == RndFont3d::StaticClassName(), 0x17D);
    mFont = static_cast<RndFont3d *>(f);
}

void RndText::FontMap3d::IncrementDisplayableChars(unsigned short us1) {
    RndFont3d::CharInfo *info = mFont->GetCharInfo(us1);
    if (info && info->unk24) {
        mDisplayableChars++;
    }
}

void RndText::FontMap3d::AllocateMeshes(RndText *text, int i2) {
    int i9 = 0;
    if (mFont) {
        i9 = i2 ? i2 : mDisplayableChars;
    }
    int numMeshes = mMeshes.size();
    for (int i = i9; i < mMeshes.size(); i++) {
        delete mMeshes[i];
    }
    mMeshes.resize(numMeshes);
    for (int i = 0; i < mMeshes.size(); i++) {
        if (i >= numMeshes) {
            mMeshes[i] = Hmx::Object::New<RndMesh>();
        }
        RndMesh *cur = mMeshes[i];
        cur->SetTransParent(text, false);
        cur->SetTransConstraint(kConstraintNone, nullptr, false);
        cur->SetMat(mFont->Mat());
        cur->SetShowing(true);
    }
    mMeshItr = mMeshes.begin();
}

void RndText::FontMap3d::CleanupSyncMeshes() {
    for (; mMeshItr != mMeshes.end(); ++mMeshItr) {
        (*mMeshItr)->SetShowing(false);
    }
}

void RndText::FontMap3d::SetupCharacter(
    unsigned short us2,
    float &f3,
    float f4,
    const StyleState &state,
    unsigned short us6,
    float circle,
    FitType fitType,
    float indentation
) {
    float f100, ffc;
    RndMesh *localf8;
    if (mFont->CharWidthAdvanceMesh(us2, f100, ffc, &localf8)) {
        float kerning = mFont->Kerning(us6, us2);
        f3 += (kerning + state.mInfo.mKerning) * state.mInfo.mSize;
        kerning = f100 > 0 ? f100 : ffc;
        float f5 = 0;
        if (mFont->Monospace()) {
            f5 = Min((ffc - kerning) / 2, 0.0f);
        }
        float fvar6 = state.mInfo.mSize;
        f100 = fvar6 * kerning;
        kerning = fvar6 * f5;
        if (f100 > 0) {
            f4 += state.mInfo.mZOffset * fvar6;
            if (localf8 && mMeshItr != mMeshes.end()) {
                RndMesh *it = *mMeshItr++;
                it->SetGeomOwner(localf8);
                Transform tff0;
                tff0.v = mFont->CharOriginOffset();
                tff0.v *= state.mInfo.mSize;
                tff0.v.x += kerning + f3;
                tff0.v.z += f4;
                tff0.m.x.x = mFont->FontUnitInverse() * state.mInfo.mSize;
                tff0.m.x.y = tff0.m.x.x * 0;
                tff0.m.x.z = tff0.m.x.y;
                tff0.m.y.x = tff0.m.x.y;
                tff0.m.y.y = tff0.m.x.x;
                tff0.m.y.z = tff0.m.x.y;
                tff0.m.z.x = tff0.m.x.y;
                tff0.m.z.y = tff0.m.x.y;
                tff0.m.z.z = tff0.m.x.x;
                if (circle != 0) {
                    float fvar = f100 / 2 + tff0.v.x;
                    Transform tfa0 = XfmOnCircleEdge(circle, fvar);
                    tff0.v.x -= fvar;
                    Multiply(tff0, tfa0, tff0);
                }
                it->SetLocalXfm(tff0);
            }
            f3 += state.mInfo.mSize * ffc;
        }
    }
}

#pragma endregion

#pragma region RndText

RndText::Style::Style(Hmx::Object *owner) : mFont(owner), mBlacklight(false) {}

RndText::StyleState::StyleState(RndText *text, float f2)
    : mInfo(text->mStyles[0].mInfo), unk34(&text->mStyles[0]) {
    unk38 = text->FontMapIndex(unk34->mFont, unk34->mBlacklight);
    unk3c = f2;
    mInfo.mSize *= f2;
    unk40 = true;
}

RndText::RndText()
    : mWidth(0), mHeight(0), mCircle(0), mAlignment(kMiddleCenter), mFitType(kFitWrap),
      mCapsMode(kCapsModeNone), mLeading(1), mFixedLength(0), mMarkup(true),
      mBasicMarkup(true), mScrollDelay(0), mScrollRate(1), mScrollPause(0), unk40(0),
      unk58(0), unk5c(0), unk60(0), mIndentation(0), unk78(nullptr), unk8c(0), unk90(-1),
      unk94(-1), mStyles(this), mDrawRect(0, 0, 0, 0), unkc4(0), unkc8(0) {
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
    SYNC_PROP(size, o.mInfo.mSize)
    SYNC_PROP_SET(
        text_color, o.mInfo.mTextColor.Pack(), o.mInfo.mTextColor.Unpack(_val.Int())
    )
    SYNC_PROP_SET(
        text_alpha, o.mInfo.mTextColor.alpha, o.mInfo.mTextColor.alpha = _val.Float()
    )
    SYNC_PROP(font_color_override, o.mInfo.mFontColorOverride)
    SYNC_PROP_SET(
        font_color, o.mInfo.mFontColor.Pack(), o.mInfo.mFontColor.Unpack(_val.Int())
    )
    SYNC_PROP_SET(
        font_alpha, o.mInfo.mFontColor.alpha, o.mInfo.mFontColor.alpha = _val.Float()
    )
    SYNC_PROP(italics, o.mInfo.mItalics)
    SYNC_PROP(kerning, o.mInfo.mKerning)
    SYNC_PROP(z_offset, o.mInfo.mZOffset)
    SYNC_PROP(blacklight, o.mBlacklight)
END_CUSTOM_PROPSYNC

BEGIN_PROPSYNCS(RndText)
    SYNC_PROP_SET(text, TextASCII(), SetTextASCII(_val.Str()))
    SYNC_PROP_SET(fixed_length, mFixedLength, SetFixedLength(_val.Int()))
    SYNC_PROP(align, (int &)mAlignment)
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
    bs << s.mInfo.mSize;
    bs << s.mInfo.mTextColor;
    bs << s.mInfo.mFontColorOverride;
    bs << s.mInfo.mFontColor;
    bs << s.mInfo.mItalics;
    bs << s.mInfo.mKerning;
    bs << s.mInfo.mZOffset;
    bs << s.mBlacklight;
    return bs;
}

BEGIN_SAVES(RndText)
    SAVE_REVS(0x1C, 1)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(RndDrawable)
    SAVE_SUPERCLASS(RndTransformable)
    bs << mAlignment;
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
            COPY_MEMBER(mAlignment)
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
    bs >> s.mInfo.mSize;
    bs >> s.mInfo.mTextColor;
    bs >> s.mInfo.mFontColorOverride;
    bs >> s.mInfo.mFontColor;
    bs >> s.mInfo.mItalics;
    bs >> s.mInfo.mKerning;
    bs >> s.mInfo.mZOffset;
    if (TEXT_REV >= 0x19) {
        bs >> s.mBlacklight;
    }
    return bs;
}

INIT_REVS(0x1C, 1)

BEGIN_LOADS(RndText)
    LOAD_REVS(bs)
    ASSERT_REVS(0x1C, 1)
    TEXT_REV = d.rev;
    StyleInfo info;
    ObjPtr<RndFontBase> fontBase(this);
    if (d.rev > 0xF) {
        LOAD_SUPERCLASS(Hmx::Object)
    }
    LOAD_SUPERCLASS(RndDrawable)
    if (d.rev < 7) {
        ObjPtrList<Hmx::Object> objects(this);
        int x;
        d >> x >> objects;
    }
    if (d.rev > 1) {
        LOAD_SUPERCLASS(RndTransformable)
    }
    if (d.rev < 0x16) {
        d >> fontBase;
    }
    if (d.rev < 3) {
        int idx;
        d >> idx;
        Alignment align_choices[6] = { kTopLeft,    kTopCenter,    kTopRight,
                                       kBottomLeft, kBottomCenter, kBottomRight };
        mAlignment = align_choices[idx];
    } else {
        d >> (int &)mAlignment;
    }
    if (d.rev < 2) {
        Vector2 v2;
        d >> v2;
        SetLocalPos(Vector3(v2.x, 0, -v2.y * 0.75f));
    }
    d >> mText;
    if (d.rev < 0x14) {
        std::vector<unsigned short> vec;
        ASCIItoWideVector(vec, mText.c_str());
        WideVectorToUTF8(vec, mText);
    }
    if (d.rev > 0 && d.rev < 0x16) {
        d >> info.mTextColor;
    }
    if (d.rev > 0xC) {
        d >> mWidth;
    } else if (d.rev > 3) {
        bool b;
        d >> b;
        d >> mWidth;
        if (!b)
            mWidth = 0.0f;
        if (d.rev < 5 && (mWidth < 0.0f || mWidth > 1000.0f))
            mWidth = 0.0f;
    }
    if (d.rev == 5) {
        String str;
        d >> str;
    }
    if (d.rev > 4 && d.rev < 11) {
        bool b;
        d >> b;
        if (fontBase) {
            RndFont *oldfont2d = dynamic_cast<RndFont *>(fontBase.Ptr());
            MILO_ASSERT(oldfont2d, 0xBC1);
            if (oldfont2d->NumMats() && oldfont2d->Mat(0)) {
                fontBase->Mat()->SetZMode(b ? kZModeTransparent : kZModeDisable);
            }
        }
    }
    if (d.rev > 7) {
        d >> mLeading;
    }
    if (d.rev > 0xB) {
        int len;
        d >> len;
        SetFixedLength(len);
    } else if (d.rev > 8) {
        bool b;
        d >> b;
        if (b) {
            SetFixedLength(mText.length());
        } else {
            ClearFixedLength();
        }
    }

    if (d.rev > 9 && d.rev < 0x16) {
        d >> info.mItalics;
    }
    if (d.rev < 0x16) {
        if (d.rev > 0xC) {
            d >> info.mSize;
        } else if (fontBase) {
            RndFont *oldfont2d = dynamic_cast<RndFont *>(fontBase.Ptr());
            MILO_ASSERT(oldfont2d, 0xBE9);
            info.mSize = oldfont2d->DeprecatedSize();
        }
    }
    if (d.rev < 0xD) {
        info.mItalics /= info.mSize;
    }
    if (d.rev > 0xD) {
        d >> mMarkup;
    }
    if (d.rev > 0xE) {
        d >> (int &)mCapsMode;
    } else {
        mCapsMode = kCapsModeNone;
    }
    if (d.rev >= 0x12 && d.rev < 0x15) {
        bool b;
        d >> b;
    }
    if (d.rev >= 0x13 && d.rev < 0x15) {
        int i, j, k;
        d >> i >> j >> k;
    }
    if (d.rev >= 0x16) {
        if (d.rev > 0x16) {
            if (d.rev == 0x17) {
                MILO_NOTIFY(
                    "%s was bad version 23, suggest reverting and resaving, lost [height] and [fit_type]",
                    PathName(this)
                );
            } else {
                d >> mHeight;
                if (d.rev < 0x18) {
                    String str;
                    d >> str;
                }
                if (d.altRev > 0) {
                    d >> mCircle;
                }
                d >> (int &)mFitType;
            }
        }
        d >> mStyles;
    } else if (d.rev < 0x16) {
        mStyles.resize(1);
        info.mZOffset = 0;
        mStyles[0].mInfo = info;
        mStyles[0].mFont = fontBase;
    }
    if (d.rev >= 0x1A) {
        d >> mScrollDelay;
        d >> mScrollRate;
        d >> mScrollPause;
    }
    if (d.rev >= 0x1B) {
        d >> mIndentation;
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

float RndText::GetDistanceToPlane(const Plane &pl, Vector3 &v) {
    if (mFontMaps.empty()) {
        return 0;
    } else {
        float f6 = 0;
        bool b1 = true;
        FOREACH (it, mFontMaps) {
            for (int i = 0; i < (*it)->NumMeshes(); i++) {
                RndMesh *mesh = (*it)->Mesh(i);
                if (mesh) {
                    Vector3 locVec;
                    float f7 = mesh->GetDistanceToPlane(pl, locVec);
                    if (b1 || (fabs(f7) < fabs(f6))) {
                        b1 = false;
                        v = locVec;
                        f6 = f7;
                    }
                }
            }
        }
        return f6;
    }
}

bool RndText::MakeWorldSphere(Sphere &s, bool zero) {
    s.Zero();
    FOREACH (it, mFontMaps) {
        for (int i = 0; i < (*it)->NumMeshes(); i++) {
            RndMesh *mesh = (*it)->Mesh(i);
            if (mesh) {
                Sphere locSphere;
                if (zero) {
                    mesh->MakeWorldSphere(locSphere, true);
                } else if (GetSphere().radius != 0) {
                    Multiply(GetSphere(), WorldXfm(), locSphere);
                }
                s.GrowToContain(locSphere);
            }
        }
    }
    return s.radius;
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

void RndText::DrawShowing() {
    SizeCheck();
    int i11 = 0;
    FOREACH (it, mFontMaps) {
        i11 += (*it)->NumMaterials();
    }
    // Hmx::Color *colors = (Hmx::Color *)_alloca(i11 * sizeof(Hmx::Color));
    float *colors = (float *)_alloca(i11 * sizeof(Hmx::Color));
    FOREACH (it, mFontMaps) {
        for (int i = 0; i < (*it)->NumMaterials(); i++) {
            RndMat *mat = (*it)->Material(i);
            Hmx::Color *curColor = (Hmx::Color *)&colors[i * 4];
            *curColor = mat->GetColor();
        }
    }
    bool b3 = false;
    FOREACH (it, mStyles) {
        if (it->mFont && it->mInfo.mFontColorOverride) {
            int idx = FontMapIndex(it->mFont, it->mBlacklight);
            if (idx != -1) {
                b3 = true;
                FontMapBase *font = mFontMaps[idx];
                for (int i = 0; i < font->NumMaterials(); i++) {
                    RndMat *mat = font->Material(i);
                    Hmx::Color *curColor = (Hmx::Color *)&colors[i * 4];
                    mat->SetColor(*curColor);
                }
            }
        }
    }
    if (unk40) {
        UpdateScrollOffsets();
    }

    FOREACH (it, mFontMaps) {
        for (int i = 0; i < (*it)->NumMeshes(); i++) {
            RndMesh *curMesh = (*it)->Mesh(i);
            if (curMesh) {
                if (sBlacklightModeEnabled && (*it)->mBlacklight
                    && TheUI->BlacklightScreenEnabled()) {
                    QueueBlacklightPacket(curMesh, unk58, unk5c);
                } else {
                    DrawMesh(curMesh, unk58, unk5c);
                }
            }
        }
    }
    if (b3) {
        FOREACH (it, mFontMaps) {
            for (int i = 0; i < (*it)->NumMaterials(); i++) {
                RndMat *mat = (*it)->Material(i);
                Hmx::Color *curColor = (Hmx::Color *)&colors[i * 4];
                mat->SetColor(curColor->red, curColor->green, curColor->blue);
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

void RndText::Highlight() { RndDrawable::Highlight(); }

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
        f1 = mStyles[0].mFont->AspectRatio() * mStyles[0].mInfo.mSize * f2;
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
        MemDoTempAllocations tmp;
        str.resize(UTF8StrLen(mText.c_str()) + 1);
    }
    UTF8toASCIIs((char *)str.c_str(), str.capacity(), mText.c_str(), '*');
    return str;
}

void RndText::BuildFontMaps(bool b1) {
    if (b1) {
        for (auto it = mFontMaps.begin(); it != mFontMaps.end();
             it = mFontMaps.erase(it)) {
            FontMapBase *cur = *it;
            sFontMapCache.push_back(cur);
        }
    }
    if (mFontMaps.empty()) {
        for (int i = 0; i < mStyles.size(); i++) {
            RndFontBase *font = mStyles[i].mFont ? mStyles[i].mFont : mStyles[0].mFont;
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
        MemDoTempAllocations tmp;
        std::vector<unsigned short> vec;
        ASCIItoWideVector(vec, cstr);
        WideVectorToUTF8(vec, str);
    }
    SetText(str.c_str());
}

void RndText::ConstructMeshes(
    const std::vector<RndText::Line> &lines, const Hmx::Rect &rect, float f3
) {
    unkc8 = f3;
    unkc4 = lines.size();
    mDrawRect = rect;
    FOREACH (it, mFontMaps) {
        (*it)->AllocateMeshes(this, mFixedLength);
    }
    if (mStyles[0].mFont) {
        StyleState state(this, f3);
        for (int i = 0; i < lines.size(); i++) {
            const Line &curLine = lines[i];
            unsigned short usIt = 0;
            float f10 = curLine.unk10;
            float fc = curLine.unkc;
            for (const unsigned short *it = curLine.unk0; it != curLine.unk4;) {
                unsigned short cur = *it;
                if (cur == 0x3C && mMarkup) {
                    it = ParseMarkup(it, state, cur);
                    if (cur != 0) {
                        it--;
                    }
                }
                if (cur != 0) {
                    mFontMaps[state.unk38]->SetupCharacter(
                        cur, fc, f10, state, usIt, mCircle, mFitType, mIndentation
                    );
                    usIt = cur;
                    it++;
                }
            }
        }
    }
    FOREACH (it, mFontMaps) {
        (*it)->CleanupSyncMeshes();
    }
}

void RndText::SizeCheck() {
    static float s6590;
    static RndText *s6594;
    StyleState state(this, unkc8);
    FOREACH (it, mFontMaps) {
        RndFontBase *curFontBase = (*it)->Font();
        if (curFontBase && curFontBase->BitmapFont()) {
            for (int i = 0; i < (*it)->NumMeshes(); i++) {
                RndMesh *curMesh = (*it)->Mesh(i);
                if (curMesh) {
                    float fref;
                    if (CalcScreenHeight(
                            curFontBase->AspectRatio() * state.mInfo.mSize, curMesh, fref
                        )) {
                        float f11 = curFontBase->FontUnit() * curFontBase->AspectRatio();
                        float f10 = fref < 127.5f ? fref : 127.5f;
                        if ((f11 * 1.25f < f10 && (s6594 != this || s6590 < fref))) {
                            MILO_NOTIFY(
                                "oversized: %s font: %s token:'%s' text:'%s' %d < %d",
                                PathName(this),
                                curFontBase->Name(),
                                TextToken(),
                                mText,
                                (int)(f11),
                                (int)fref
                            );
                            s6590 = fref;
                            s6594 = this;
                        }
                    }

                    return;
                }
            }
        }
    }
}

void RndText::ReFitTextScroll(String str) {
    if (mFitType != kFitScrollMarqueeWrapAlways) {
        return;
    } else {
        SetText(str.c_str());
        FitTextScroll();
        unk50 = 0;
        unk8c = 0;
        while (mWidth - unk8c >= unk68.front()) {
            unk90++;
            if (unk90 >= unk60) {
                unk90 = 0;
            }
            if (unk68.front() == unk54) {
                unk8c += mWidth;
            }
            if (unk60 == unk68.size()) {
                unk68.push_back(unk68.front());
            }
            unk68.pop_front();
        }
        unk44 = unk48;
    }
}

float RndText::ComputeCharWidthsForText(String str) {
    BuildFontMaps(false);
    std::vector<unsigned short> wide;
    int ret = ConvertTextToWide(str.c_str(), wide);
    float *floats = (float *)_alloca((ret + 2) * sizeof(float));
    OnComputeCharWidths(wide.begin(), floats, true);
    return floats[ret];
}

void RndText::GetWidthHeightBox(Box &box) const {
    if (mAlignment & 1) {
        box.mMin.x = 0;
    } else if (mAlignment & 2) {
        box.mMin.x = -mWidth / 2;
    } else {
        box.mMin.x = -mWidth;
    }
    if (mAlignment & 0x10) {
        box.mMin.z = -mHeight;
    } else if (mAlignment & 0x20) {
        box.mMin.z = -mHeight / 2;
    } else {
        box.mMin.z = 0;
    }
    box.mMax.x = mWidth + box.mMin.x;
    box.mMax.z = mHeight + box.mMin.z;
    box.mMax.y = 0;
    box.mMin.y = 0;
}

void RndText::UpdateText() {
    if (mFitType == kFitJust) {
        FitTextJust();
        return;
    }
    if (mStyles[0].mInfo.mSize > 0 && mWidth > 0) {
        if (mFitType == kFitEllipsis) {
            FitTextEllipsis();
            return;
        }
        if (mFitType == kFitScrollMarqueeReset || mFitType == kFitScrollMarqueeWrap
            || mFitType == kFitScrollPingPong
            || mFitType == kFitScrollMarqueeWrapAlways) {
            for (int i = 0; i < mStyles.size(); i++) {
                RndFontBase *font =
                    mStyles[i].mFont ? mStyles[i].mFont : mStyles[0].mFont;

                if (font && font->ClassName() == RndFont::StaticClassName()) {
                    continue;
                } else {
                    const char *name = font ? font->Name() : "NULL";
                    MILO_NOTIFY(
                        "%s %s requests scrolling, but uses a font that does not support it (%s)",
                        PathName(this),
                        Name(),
                        name
                    );
                    mFitType = kFitEllipsis;
                    FitTextEllipsis();
                    return;
                }
            }
            FitTextScroll();
            return;
        }
    }

    std::vector<RndText::Line> lines;
    BuildFontMaps(true);
    std::vector<unsigned short> wide;
    int ret = ConvertTextToWide(mText.c_str(), wide);
    float *floats = (float *)_alloca((ret + 2) * sizeof(float));
    OnComputeCharWidths(wide.begin(), floats, false);
    Hmx::Rect rect;
    WrapText(wide.begin(), ret, floats, lines, rect, 1);
    ConstructMeshes(lines, rect, 1);
}

void RndText::FitTextJust() {
    BuildFontMaps(true);
    std::vector<RndText::Line> lines;
    std::vector<unsigned short> wide;
    float f9 = 1;
    int ret = ConvertTextToWide(mText.c_str(), wide);
    float *floats = (float *)_alloca((ret + 2) * sizeof(float));
    OnComputeCharWidths(wide.begin(), floats, false);
    Hmx::Rect rect;
    WrapText(wide.begin(), ret, floats, lines, rect, 1);
    float f10 = mStyles[0].mInfo.mSize;
    float f7 = 0.2f;
    if ((mWidth && rect.w > mWidth) || (mHeight && rect.h > mHeight)) {
        float f11 = mStyles[0].mInfo.mSize;
        float f6 = 0.2f;
        while (f10 - f7 > 0.2f) {
            f11 = (f6 + f10) / 2;
            f9 = f11 / mStyles[0].mInfo.mSize;
            WrapText(wide.begin(), ret, floats, lines, rect, f9);
            if ((mWidth && rect.w > mWidth) || (mHeight && rect.h > mHeight)) {
                f7 = f6;
                f10 = f11;
            }
            f6 = f7;
        }
        if (f10 == f11) {
            f9 = f7 / mStyles[0].mInfo.mSize;
            WrapText(wide.begin(), ret, floats, lines, rect, f9);
        }
    }
    ConstructMeshes(lines, rect, f9);
}

void RndText::FitTextScroll() {
    BuildFontMaps(true);
    std::vector<RndText::Line> lines;
    std::vector<unsigned short> wide;
    int ret = ConvertTextToWide(mText.c_str(), wide);
    float *floats = (float *)_alloca((ret + 2) * sizeof(float));
    unk60 = 0;
    unk68.clear();
    unk70.clear();
    OnComputeCharWidths(wide.begin(), floats, false);
    unk40 = false;
    float f12 = 0;
    float f11 = mWidth;
    float f13 = f12;
    Hmx::Rect rect;
    if (floats[ret] > f11 || mFitType == kFitScrollMarqueeWrapAlways) {
        mWidth = 0;
        unk40 = true;
        RndFontBase *font = mStyles[0].mFont;
        MILO_ASSERT(font, 0xA9E);
        if (font) {
            unsigned short us;
            DecodeUTF8(us, "8");
            float fref;
            font->CharAdvance(us, us, fref);
            rect.x = mStyles[0].mInfo.mSize;
            f12 = (rect.x + 0x2c + fref) * rect.x;
        }
    }
    WrapText(wide.begin(), ret, floats, lines, rect, 1);
    ConstructMeshes(lines, rect, 1);
    if (unk40) {
        mWidth = f11;
        unk5c = 1;
        unk44 = f13;
        unk4c = mScrollRate * f12 * -0.001f;
        if (mFitType == kFitScrollMarqueeWrapAlways) {
            unk50 = f11;
            unk8c = f11;
            unk60++;
            unk54 = mIndentation * (float)unk60 + floats[ret];
            unk68.push_back(unk54);
            unk68.push_back(f13);
            unk70.push_back(unk54);
            unk58 = unk54;
            f11 = f12;
            if (f13 < f12) {
                while (f13 < unk54) {
                    if (mWidth < f11)
                        break;
                    f11 += f12;
                    unk5c++;
                }
            }
            unk90 = -1;
            unk94 = -1;
        } else {
            unk50 = f13;
            unk54 = floats[ret];
            unk58 = f13;
        }
        unk48 = mScrollDelay;
        for (int i = 0; i < mFontMaps.size(); i++) {
            mFontMaps[i]->SetupScrolling();
        }
    }
}

void RndText::UpdateScrollOffsets() {
    float delta = TheTaskMgr.DeltaUISeconds();
    unk44 += delta;
    if (unk44 >= unk48) {
        bool b11 = false;
        float widthDiff = unk54 - mWidth;
        float f13 = unk4c * delta * 1000;
        unk50 += f13;
        switch (mFitType) {
        case kFitScrollMarqueeWrap:
            if (unk50 < -unk54) {
                unk50 = 0;
            }
            unk58 = unk54;
            break;
        case kFitScrollMarqueeReset:
            if (unk50 < -(unk54 + 20)) {
                unk50 = 0;
                b11 = true;
            }
            break;
        case kFitScrollPingPong:
            if (unk4c >= 0) {
                if (unk4c > 0 && unk50 >= 0) {
                    unk4c = -unk4c;
                    unk50 = 0;
                    b11 = true;
                }
            } else if (unk50 < -widthDiff) {
                unk50 = -widthDiff;
                unk4c = -unk4c;
                b11 = true;
            }
            break;
        case kFitScrollMarqueeWrapAlways: {
            static Message textScrolledIn("text_scrolled_in", -1);
            static Message textScrolledOut("text_scrolled_out", -1);
            float old8c = unk8c;
            unk8c += f13;
            float f14 = unk68.front();
            if (mWidth - (old8c + f13) >= f14) {
                unk90++;
                if (unk90 >= unk60) {
                    unk90 = 0;
                }
                textScrolledIn[0] = unk90;
                if (f14 == unk54) {
                    unk8c = mWidth;
                }
                if (unk60 == unk68.size()) {
                    unk68.push_back(unk54);
                }
                unk68.pop_front();
                if (unk78) {
                    unk78->Handle(textScrolledIn, false);
                }
            }
            float f15 = unk70.front();
            if (unk50 <= -f15) {
                unk94++;
                textScrolledOut[0] = unk94;
                if (f15 == unk54) {
                    unk50 = 0;
                    unk94 = -1;
                }
                unk70.push_back(unk50);
                unk70.pop_front();
                if (unk78) {
                    unk78->Handle(textScrolledOut, false);
                }
            }
            break;
        }
        default:
            unk50 = 0;
            break;
        }

        if (b11) {
            unk44 = 0;
        }
    }
    FOREACH (it, mFontMaps) {
        (*it)->UpdateScrolling(unk50);
    }
}

void RndText::QueueBlacklightPacket(RndMesh *mesh, float f2, int i3) {
    unsigned int cursize = sBlacklightPacketPool.capacity();
    if (sBlacklightPacketCount >= cursize) {
        unsigned int newsize = 8;
        if (cursize != 0) {
            newsize = cursize * 2;
        }
        sBlacklightPacketPool.resize(newsize);
    }
    BlacklightPacket &packet = sBlacklightPacketPool[sBlacklightPacketCount++];
    packet.unk0 = mesh;
    packet.unk4 = mesh->Mat()->GetColor();
    packet.unk14 = f2;
    packet.unk18 = i3;
    packet.unk1c = RndCam::Current();
}

void RndText::ClearBlacklight() { sBlacklightPacketCount = 0; }

void RndText::DrawBlacklight() {
    RndCam *cur = RndCam::Current();
    for (int i = 0; i < sBlacklightPacketCount; i++) {
        BlacklightPacket &packet = sBlacklightPacketPool[i];
        if (packet.unk1c && packet.unk1c != RndCam::Current()) {
            packet.unk1c->Select();
        }
        packet.unk0->Mat()->SetColor(packet.unk4.red, packet.unk4.green, packet.unk4.blue);
        DrawMesh(packet.unk0, packet.unk14, packet.unk18);
    }
    if (cur && cur != RndCam::Current()) {
        cur->Select();
    }
}

void RndText::DrawMesh(RndMesh *mesh, float f2, int i3) {
    mesh->DrawShowing();
    if (f2) {
        float f7 = f2;
        for (int i = 0; i < i3; i++) {
            Vector3 pos = mesh->LocalXfm().v;
            pos.x += f7;
            mesh->SetLocalPos(pos);
            mesh->DrawShowing();
            pos.x -= f7;
            mesh->SetLocalPos(pos);
            f7 += f2;
        }
    }
}

RndText::FontMapBase *RndText::AcquireFontMap(RndFontBase *fontBase) {
    Symbol classToUse;
    if (fontBase->ClassName() == RndFont::StaticClassName()) {
        classToUse = FontMap::StaticClassName();
    } else if (fontBase->ClassName() == RndFont3d::StaticClassName()) {
        classToUse = FontMap3d::StaticClassName();
    } else {
        MILO_FAIL("Unknown Font type: %s", fontBase->ClassName());
        classToUse = FontMap::StaticClassName();
    }

    FOREACH (it, sFontMapCache) {
        if ((*it)->ClassName() == classToUse) {
            FontMapBase *found = *it;
            sFontMapCache.erase(it);
            if (found) {
                found->SetFont(fontBase);
                found->ResetDisplayableChars();
                return found;
            } else {
                break;
            }
        }
    }

    FontMapBase *found;
    if (classToUse == FontMap::StaticClassName()) {
        found = new FontMap();
    } else if (classToUse == FontMap3d::StaticClassName()) {
        found = new FontMap3d();
    } else {
        MILO_FAIL("Unknown FontMap type: %s", classToUse);
        found = new FontMap();
    }
    found->SetFont(fontBase);
    found->ResetDisplayableChars();
    return found;
}

#pragma endregion
