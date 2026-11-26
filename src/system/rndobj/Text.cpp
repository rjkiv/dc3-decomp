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
#include <cwchar>
#include <set>

bool RndText::sBlacklightModeEnabled = false;
int RndText::sBlacklightPacketCount = 0;

std::vector<RndText::BlacklightPacket> RndText::sBlacklightPacketPool;
std::list<RndText::FontMapBase *> RndText::sFontMapCache;
int TEXT_REV = 0;
static float gSuperscriptScale = 0.7f;
static float gGuitarScale = 0.7f;
static float gGuitarZOffset = 0.2f;

float SegmentLength(
    int startOff,
    int endOff,
    const float *charWidths,
    const unsigned short *in,
    float scale
) {
    for (const unsigned short *p = in + startOff; *p == ' ' && startOff < endOff;
         startOff++, p++)
        ;
    for (const unsigned short *p = in + endOff - 1; *p == ' ' && startOff < endOff;
         endOff--, p--)
        ;
    return (charWidths[endOff] - charWidths[startOff]) * scale;
}

void ResetFontMapPageMeshFaces(RndMesh *mesh, int numFaces) {
    MILO_ASSERT(mesh, 0x96);
    mesh->Faces().resize(numFaces);

    auto it = mesh->Faces().begin();
    auto itEnd = mesh->Faces().end();
    int i7 = 0;
    for (; it != itEnd; it += 2, i7 += 4) {
        it[0].Set(i7, i7 + 1, i7 + 2);
        it[1].Set(i7, i7 + 2, i7 + 3);
    }
}

Transform XfmOnCircleEdge(float circleCircumference, float distAround) {
    Transform out;
    float f10 = circleCircumference < 0 ? -1.0f : 1.0f;
    out.m.z.Set(0, 0, 1);
    float f9 = (distAround / circleCircumference) * (2 * PI) + (f10 * (-PI / 2));
    out.v.Set(Cosine(f9), Sine(f9), 0);
    Scale(out.v, -f10, out.m.y);
    Cross(out.m.y, out.m.z, out.m.x);
    out.v *= f10 * circleCircumference / (2 * PI);
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
    while (pages.size() != 0) {
        delete pages.back();
        pages.pop_back();
    }
}

void RndText::FontMap::SetFont(RndFontBase *f) {
    MILO_ASSERT(f->ClassName() == RndFont::StaticClassName(), 0x75);
    font = static_cast<RndFont *>(f);
    while (pages.size() > font->NumMats()) {
        delete pages.back();
        pages.pop_back();
    }
    pages.reserve(font->NumMats());
    while (pages.size() < font->NumMats()) {
        pages.push_back(new Page());
    }
}

void RndText::FontMap::ResetDisplayableChars() {
    for (int i = 0; i < pages.size(); i++) {
        pages[i]->displayableChars = 0;
    }
}

void RndText::FontMap::IncrementDisplayableChars(unsigned short num) {
    int page = font->CharPage(num);
    if (page >= 0) {
        pages[page]->displayableChars++;
    }
}

void RndText::FontMap::AllocateMeshes(RndText *text, int fixedLength) {
    for (int i = 0; i < pages.size(); i++) {
        Page &page = *(pages[i]);
        if (!page.mesh && font && page.displayableChars > 0) {
            page.mesh = Hmx::Object::New<RndMesh>();
        }
        RndMesh *mesh = page.mesh;
        page.syncFlags = 0x1F;
        page.vertIt = 0;
        if (mesh) {
            mesh->SetTransParent(text, false);
            mesh->SetTransConstraint(RndTransformable::kParentWorld, nullptr, false);
            if (font) {
                mesh->SetMat(font->Mat(i));
            }
            mesh->SetShowing(page.displayableChars > 0);
            if (fixedLength == 0) {
                int numFaces = page.displayableChars * 2;
                mesh->SetMutable(0);
                ResetFontMapPageMeshFaces(mesh, numFaces);
                page.syncFlags |= 0xA0;
                mesh->Verts().resize(numFaces * 2);
            } else if (
                (mesh->Mutable() & 0x1F) == 0 || mesh->Verts().size() != fixedLength * 4
            ) {
                int numFaces = fixedLength * 2;
                mesh->SetMutable(0x1F);
                ResetFontMapPageMeshFaces(mesh, numFaces);
                page.syncFlags |= 0xA0;
                mesh->Verts().resize(numFaces * 2);
            }
            page.vertIt = mesh->Verts().begin();
            MILO_ASSERT(mesh->Verts().size() >= page.displayableChars * 4, 0xD2);
        }
        MILO_ASSERT(!fixedLength || (page.displayableChars <= fixedLength), 0xD5);
    }
}

void RndText::FontMap::CleanupSyncMeshes() {
    for (int i = 0; i < pages.size(); i++) {
        Page *page = pages[i];
        RndMesh *mesh = page->mesh;
        if (mesh) {
            while (page->vertIt != mesh->Verts().end()) {
                page->vertIt++->pos.Set(0, 0, 0);
            }
            mesh->Sync(page->syncFlags);
        }
    }
}

void RndText::FontMap::SetupCharacter(
    unsigned short c,
    float &x,
    float baseZ,
    const StyleState &state,
    unsigned short prevChar,
    float circleCircumference,
    FitType fitType,
    float indentation
) {
    if (fitType == kFitScrollMarqueeWrapAlways && c == 10) {
        x += indentation;
    } else {
        int i5 = font->CharPage(c);
        if (i5 < 0) {
            return;
        }
        Page *page = pages[i5];
        float width, advance;
        if (!font->CharWidthAdvanceCoords(
                c, width, advance, page->vertIt[0].tex, page->vertIt[2].tex
            )) {
            return;
        }

        x += font->Kerning(prevChar, c) + state.kerning * state.size;
        width = width > 0 ? width : advance;

        float f8 = 0;
        if (font->Monospace()) {
            f8 = Max((advance - width) / 2.0f, 0.0f);
        }
        width = state.size * width;
        float f11 = state.size * f8;
        if (width <= 0) {
            return;
        }
        float f13 = state.zOffset * state.size + baseZ;
        float aspect8 = font->AspectRatio();
        float f10 = state.italics * state.size;
        float fvar1 = f13 - (aspect8 * state.size);
        page->vertIt[0].pos.Set(f10 + f11 + x, 0, f13);
        page->vertIt[0].tangent.AsVector3().Set((f11 + x) - f10, 0, fvar1);
        page->vertIt[1].tangent.AsVector3().Set(((f11 + x) - f10) + width, 0, fvar1);
        page->vertIt[2].tangent.AsVector3().Set(((f11 + x) + width) + f10, 0, f13);
        if (circleCircumference != 0) {
            float fvar10 = (page->vertIt[2].tangent.x - page->vertIt[0].tangent.x) / 2
                + page->vertIt[0].tangent.x;
            Transform xfm = XfmOnCircleEdge(circleCircumference, fvar10);
            Vector3 tmp;
            Scale(xfm.m.x, fvar10, tmp);
            Subtract(xfm.v, tmp, xfm.v);
            Multiply(page->vertIt[0].pos, xfm, page->vertIt[0].pos);
            Multiply(page->vertIt[1].pos, xfm, page->vertIt[1].pos);
            Multiply(page->vertIt[2].pos, xfm, page->vertIt[2].pos);
            Multiply(page->vertIt[3].pos, xfm, page->vertIt[3].pos);
        }
        page->vertIt[1].tex.Set(page->vertIt[0].tex.x, page->vertIt[2].tex.y);
        page->vertIt[3].tex.Set(page->vertIt[2].tex.x, page->vertIt[0].tex.y);
        page->vertIt[0].norm.Set(0, -1, 0);
        page->vertIt[1].norm = page->vertIt[2].norm = page->vertIt[3].norm =
            page->vertIt[0].norm;
        page->vertIt[0].color = page->vertIt[1].color = page->vertIt[2].color =
            page->vertIt[3].color = state.textColor;
        // advance to the next set of 4 verts
        page->vertIt = &page->vertIt[4];
        x += state.size * advance;
    }
}

void RndText::FontMap::SetupScrolling() {
    for (int i = 0; i < NumMeshes(); i++) {
        RndMesh *mesh = Mesh(i);
        if (mesh) {
            mesh->SetTransConstraint(RndTransformable::kNone, nullptr, false);
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
    for (int i = 0; i < meshes.size(); i++) {
        if (meshes[i]) {
            delete meshes[i];
        }
    }
}

void RndText::FontMap3d::SetFont(RndFontBase *f) {
    MILO_ASSERT(f->ClassName() == RndFont3d::StaticClassName(), 0x17D);
    font = static_cast<RndFont3d *>(f);
}

void RndText::FontMap3d::IncrementDisplayableChars(unsigned short us1) {
    RndFont3d::CharInfo *info = font->GetCharInfo(us1);
    if (info && info->mesh) {
        displayableChars++;
    }
}

void RndText::FontMap3d::AllocateMeshes(RndText *text, int i2) {
    int i9 = 0;
    if (font) {
        i9 = i2 ? i2 : displayableChars;
    }
    for (int i = i9; i < meshes.size(); i++) {
        delete meshes[i];
    }
    int numMeshes = meshes.size();
    meshes.resize(i9);
    for (int i = 0; i < meshes.size(); i++) {
        if (i >= numMeshes) {
            meshes[i] = Hmx::Object::New<RndMesh>();
        }
        RndMesh *cur = meshes[i];
        cur->SetTransParent(text, false);
        cur->SetTransConstraint(kNone, nullptr, false);
        cur->SetMat(font->Mat());
        cur->SetShowing(true);
    }
    meshIt = meshes.begin();
}

void RndText::FontMap3d::CleanupSyncMeshes() {
    for (; meshIt != meshes.end(); ++meshIt) {
        (*meshIt)->SetShowing(false);
    }
}

void RndText::FontMap3d::SetupCharacter(
    unsigned short c,
    float &x,
    float baseZ,
    const StyleState &state,
    unsigned short prevChar,
    float circleCircumference,
    FitType fitType,
    float indentation
) {
    float width, advance;
    RndMesh *pMesh;
    if (font->CharWidthAdvanceMesh(c, width, advance, &pMesh)) {
        x += (font->Kerning(prevChar, c) + state.kerning) * state.size;
        width = width > 0 ? width : advance;

        float f5 = 0;
        if (font->Monospace()) {
            f5 = Max((advance - width) / 2, 0.0f);
        }
        float f6 = state.size * f5;
        width *= state.size;

        if (width > 0) {
            baseZ += state.zOffset * state.size;
            if (pMesh && meshIt != meshes.end()) {
                RndMesh *it = *meshIt++;
                it->SetGeomOwner(pMesh);
                Transform tff0;
                tff0.v = font->CharOriginOffset();
                tff0.v *= state.size;
                tff0.v.x += f6 + x;
                tff0.v.z += baseZ;
                float scale = font->FontUnitInverse() * state.size;
                tff0.m.x.x = scale;
                tff0.m.y.y = scale;
                tff0.m.z.z = scale;
                scale *= 0;
                tff0.m.x.y = scale;
                tff0.m.x.z = scale;
                tff0.m.y.x = scale;
                tff0.m.y.z = scale;
                tff0.m.z.x = scale;
                tff0.m.z.y = scale;
                if (circleCircumference != 0) {
                    float fvar = width / 2 + tff0.v.x;
                    Transform tfa0 = XfmOnCircleEdge(circleCircumference, fvar);
                    tff0.v.x -= fvar;
                    Multiply(tff0, tfa0, tff0);
                }
                it->SetLocalXfm(tff0);
            }
            x += state.size * advance;
        }
    }
}

#pragma endregion

#pragma region RndText

RndText::Style::Style(Hmx::Object *owner) : font(owner), blacklight(false) {}

RndText::StyleState::StyleState(RndText *text, float f2)
    : StyleBase(text->mStyles[0]), curStyle(&text->mStyles[0]) {
    fontMapIndex = text->FontMapIndex(curStyle->font, curStyle->blacklight);
    scale = f2;
    size *= f2;
    brk = true;
}

RndText::RndText()
    : mWidth(0), mHeight(0), mCircleCircumference(0), mAlignment(kMiddleCenter),
      mFitType(kFitWrap), mCapsMode(kCapsModeNone), mLeading(1), mFixedLength(0),
      mMarkup(true), mBasicMarkup(true), mScrollDelay(0), mScrollRate(1), mScrollPause(0),
      mScrollEnabled(0), mCloneOffset(0), mNumClones(0), mNumSubTexts(0), mIndentation(0),
      mEventReceiver(nullptr), mEnterOffset(0), mEnteredIdx(-1), mExitedIdx(-1),
      mStyles(this), mCachedRect(0, 0, 0, 0), mCachedNumLines(0), unkc8(0) {
    mStyles.resize(1);
    mFontMap.reserve(1);
}

RndText::~RndText() {
    FOREACH (it, mFontMap) {
        delete *it;
    }
}

BEGIN_HANDLERS(RndText)
    HANDLE_EXPR(get_text_size, Max<int>(mFixedLength, mText.length()))
    HANDLE_ACTION(update_text, UpdateText())
    HANDLE_SUPERCLASS(RndDrawable)
    HANDLE_SUPERCLASS(RndTransformable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_CUSTOM_PROPSYNC(RndText::Style)
    SYNC_PROP(font, o.font)
    SYNC_PROP(size, o.size)
    SYNC_PROP_SET(text_color, o.textColor.PackRgb(), o.textColor.UnpackRgb(_val.Int()))
    SYNC_PROP_SET(text_alpha, o.textColor.alpha, o.textColor.alpha = _val.Float())
    SYNC_PROP(font_color_override, o.fontColorOverride)
    SYNC_PROP_SET(font_color, o.fontColor.PackRgb(), o.fontColor.UnpackRgb(_val.Int()))
    SYNC_PROP_SET(font_alpha, o.fontColor.alpha, o.fontColor.alpha = _val.Float())
    SYNC_PROP(italics, o.italics)
    SYNC_PROP(kerning, o.kerning)
    SYNC_PROP(z_offset, o.zOffset)
    SYNC_PROP(blacklight, o.blacklight)
END_CUSTOM_PROPSYNC

BEGIN_PROPSYNCS(RndText)
    SYNC_PROP_SET(text, TextASCII(), SetTextASCII(_val.Str()))
    SYNC_PROP_SET(fixed_length, mFixedLength, SetFixedLength(_val.Int()))
    SYNC_PROP(align, (int &)mAlignment)
    SYNC_PROP(caps_mode, (int &)mCapsMode)
    SYNC_PROP(width, mWidth)
    SYNC_PROP(height, mHeight)
    SYNC_PROP(circle, mCircleCircumference)
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
    bs << s.font;
    bs << s.size;
    bs << s.textColor;
    bs << s.fontColorOverride;
    bs << s.fontColor;
    bs << s.italics;
    bs << s.kerning;
    bs << s.zOffset;
    bs << s.blacklight;
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
    bs << mCircleCircumference;
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
            COPY_MEMBER(mCircleCircumference)
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
    bs >> s.font;
    bs >> s.size;
    bs >> s.textColor;
    bs >> s.fontColorOverride;
    bs >> s.fontColor;
    bs >> s.italics;
    bs >> s.kerning;
    bs >> s.zOffset;
    if (TEXT_REV >= 0x19) {
        bs >> s.blacklight;
    }
    return bs;
}

INIT_REVS(0x1C, 1)

BEGIN_LOADS(RndText)
    LOAD_REVS(bs)
    ASSERT_REVS(0x1C, 1)
    TEXT_REV = d.rev;
    StyleBase info;
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
        d >> info.textColor;
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
                fontBase->Mat()->SetZMode(
                    b ? BaseMaterial::kTransparent : BaseMaterial::kDisable
                );
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
            SetFixedLength(0);
        }
    }

    if (d.rev > 9 && d.rev < 0x16) {
        d >> info.italics;
    }
    if (d.rev < 0x16) {
        if (d.rev > 0xC) {
            d >> info.size;
        } else if (fontBase) {
            RndFont *oldfont2d = dynamic_cast<RndFont *>(fontBase.Ptr());
            MILO_ASSERT(oldfont2d, 0xBE9);
            info.size = oldfont2d->DeprecatedSize();
        }
    }
    if (d.rev < 0xD) {
        info.italics /= info.size;
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
                    d >> mCircleCircumference;
                }
                d >> (int &)mFitType;
            }
        }
        d >> mStyles;
    } else if (d.rev < 0x16) {
        mStyles.resize(1);
        info.zOffset = 0;
        StyleBase &base = mStyles[0];
        base = info;
        mStyles[0].font = fontBase;
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
    FOREACH (it, mFontMap) {
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
    if (mFontMap.empty()) {
        return 0;
    } else {
        float f6 = 0;
        bool b1 = true;
        FOREACH (it, mFontMap) {
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
    FOREACH (it, mFontMap) {
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
    FOREACH (it, mFontMap) {
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
    int numMaterials = 0;
    FOREACH (it, mFontMap) {
        numMaterials += (*it)->NumMaterials();
    }
    Hmx::Color *colors = (Hmx::Color *)_alloca(numMaterials * sizeof(Hmx::Color));
    int colorIdx = 0;
    FOREACH (it, mFontMap) {
        for (int i = 0; i < (*it)->NumMaterials(); colorIdx++, i++) {
            colors[colorIdx] = (*it)->Material(i)->GetColor();
        }
    }
    bool b3 = false;
    FOREACH (it, mStyles) {
        if (it->font && it->fontColorOverride) {
            int idx = FontMapIndex(it->font, it->blacklight);
            if (idx != -1) {
                b3 = true;
                FontMapBase *font = mFontMap[idx];
                for (int i = 0; i < font->NumMaterials(); i++) {
                    font->Material(i)->SetRGBA(it->fontColor);
                }
            }
        }
    }
    if (mScrollEnabled) {
        UpdateScrollOffsets();
    }

    FOREACH (it, mFontMap) {
        for (int i = 0; i < (*it)->NumMeshes(); i++) {
            RndMesh *curMesh = (*it)->Mesh(i);
            if (curMesh) {
                UIManager *ui = TheUI;
                if (sBlacklightModeEnabled && (*it)->IsBlacklight()
                    && ui->BlacklightScreenEnabled()) {
                    QueueBlacklightPacket(curMesh, mCloneOffset, mNumClones);
                } else {
                    DrawMesh(curMesh, mCloneOffset, mNumClones);
                }
            }
        }
    }
    if (b3) {
        colorIdx = 0;
        FOREACH (it, mFontMap) {
            for (int i = 0; i < (*it)->NumMaterials(); colorIdx++, i++) {
                (*it)->Material(i)->SetColor(colors[colorIdx]);
            }
        }
    }
}

RndDrawable *RndText::CollideShowing(const Segment &s, float &f, Plane &p) {
    FOREACH (it, mFontMap) {
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
    FOREACH (it, mFontMap) {
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
    RndText::Register();
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
    for (int i = 0; i < mFontMap.size(); i++) {
        if (mFontMap[i]->Font() == f && mFontMap[i]->IsBlacklight() == b) {
            return i;
        }
    }
    return -1;
}

float RndText::ComputeHeight(int i1, float f2, float &f3) {
    float f1;
    if (mStyles[0].font) {
        f1 = mStyles[0].font->AspectRatio() * mStyles[0].size * f2;
    } else {
        f1 = 0;
    }
    f3 = mLeading * f1;
    return ((i1 - 1) * mLeading + 1.0f) * f1;
}

void RndText::SetText(const char *str) {
    if (mFixedLength != 0) {
        unsigned short us;
        MILO_ASSERT(mText.capacity() >= mFixedLength, 0x75E);
        const char *p = str;
        for (int newLen = 0; *p != '\0' && newLen < mFixedLength; newLen++) {
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
        for (auto it = mFontMap.begin(); it != mFontMap.end(); it = mFontMap.erase(it)) {
            FontMapBase *cur = *it;
            sFontMapCache.push_back(cur);
        }
    }
    if (mFontMap.empty()) {
        for (int i = 0; i < mStyles.size(); i++) {
            RndFontBase *font = GetSafeFont(mStyles[i]);
            if (font) {
                if (FontMapIndex(font, mStyles[i].blacklight) == -1) {
                    FontMapBase *map = AcquireFontMap(font);
                    map->SetBlacklight(mStyles[i].blacklight);
                    mFontMap.push_back(map);
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
    const std::vector<RndText::Line> &lines, const Hmx::Rect &rect, float scale
) {
    unkc8 = scale;
    mCachedNumLines = lines.size();
    mCachedRect = rect;
    FOREACH (it, mFontMap) {
        (*it)->AllocateMeshes(this, mFixedLength);
    }
    if (mStyles[0].font) {
        StyleState state(this, scale);
        for (int i = 0; i < lines.size(); i++) {
            const Line &curLine = lines[i];
            unsigned short usIt = 0;
            float f10 = curLine.z;
            float fc = curLine.x;
            for (const unsigned short *it = curLine.start; it != curLine.end;) {
                unsigned short cur = *it;
                if (cur == 0x3C && mMarkup) {
                    it = ParseMarkup(it, state, cur);
                    if (cur != 0) {
                        it--;
                    }
                }
                if (cur != 0) {
                    mFontMap[state.fontMapIndex]->SetupCharacter(
                        cur,
                        fc,
                        f10,
                        state,
                        usIt,
                        mCircleCircumference,
                        mFitType,
                        mIndentation
                    );
                    usIt = cur;
                    it++;
                }
            }
        }
    }
    FOREACH (it, mFontMap) {
        (*it)->CleanupSyncMeshes();
    }
}

void RndText::SizeCheck() {
    static float s6590;
    static RndText *s6594;
    StyleState state(this, unkc8);
    FOREACH (it, mFontMap) {
        RndFontBase *curFontBase = (*it)->Font();
        if (curFontBase && curFontBase->BitmapFont()) {
            for (int i = 0; i < (*it)->NumMeshes(); i++) {
                RndMesh *curMesh = (*it)->Mesh(i);
                if (curMesh) {
                    float fref;
                    float f1 = curFontBase->AspectRatio() * state.size;
                    if (CalcScreenHeight(f1, curMesh, fref)) {
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
        mScrollOffset = 0;
        mEnterOffset = 0;
        while (mWidth - mEnterOffset >= mEnterCheckPts.front()) {
            mEnteredIdx++;
            if (mEnteredIdx >= mNumSubTexts) {
                mEnteredIdx = 0;
            }
            if (mEnterCheckPts.front() == mFullStringLength) {
                mEnterOffset += mWidth;
            }
            if (mNumSubTexts == mEnterCheckPts.size()) {
                mEnterCheckPts.push_back(mEnterCheckPts.front());
            }
            mEnterCheckPts.pop_front();
        }
        mScrollTimeMs = mScrollTriggerTime;
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
    if (mStyles[0].size > 0 && mWidth > 0) {
        if (mFitType == kFitEllipsis) {
            FitTextEllipsis();
            return;
        }
        if (mFitType == kFitScrollMarqueeReset || mFitType == kFitScrollMarqueeWrap
            || mFitType == kFitScrollPingPong
            || mFitType == kFitScrollMarqueeWrapAlways) {
            for (int i = 0; i < mStyles.size(); i++) {
                RndFontBase *font = GetSafeFont(mStyles[i]);

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
    std::vector<unsigned short> buffer;
    float scale = 1;
    int len = ConvertTextToWide(mText.c_str(), buffer);
    float *charWidths = (float *)_alloca((len + 2) * sizeof(float));
    OnComputeCharWidths(buffer.begin(), charWidths, false);
    Hmx::Rect rect;
    WrapText(buffer.begin(), len, charWidths, lines, rect, scale);
    float max = mStyles[0].size;
    float min = 0.2f;
    float cur;
    if ((mWidth && rect.w > mWidth) || (mHeight && rect.h > mHeight)) {
        cur = max;
        while (max - min > 0.2f) {
            cur = (min + max) / 2;
            scale = cur / mStyles[0].size;
            WrapText(buffer.begin(), len, charWidths, lines, rect, scale);
            if ((mWidth && rect.w > mWidth) || (mHeight && rect.h > mHeight)) {
                max = cur;
            } else {
                min = cur;
            }
        }
        if (max == cur) {
            scale = min / mStyles[0].size;
            WrapText(buffer.begin(), len, charWidths, lines, rect, scale);
        }
    }
    ConstructMeshes(lines, rect, scale);
}

void RndText::FitTextScroll() {
    BuildFontMaps(true);
    std::vector<RndText::Line> lines;
    std::vector<unsigned short> buffer;
    int len = ConvertTextToWide(mText.c_str(), buffer);
    float *charWidths = (float *)_alloca((len + 2) * sizeof(float));
    mNumSubTexts = 0;
    mEnterCheckPts.clear();
    mExitCheckPts.clear();
    OnComputeCharWidths(buffer.begin(), charWidths, false);
    mScrollEnabled = false;
    float advancedWidth = 0;
    float oldWidth = mWidth;
    Hmx::Rect rect;
    if (charWidths[len] > mWidth || mFitType == kFitScrollMarqueeWrapAlways) {
        mWidth = 0;
        mScrollEnabled = true;
        RndFontBase *font = GetSafeFont(mStyles[0]);
        MILO_ASSERT(font, 0xA9E);
        if (font) {
            unsigned short wcx;
            DecodeUTF8(wcx, "8");
            float advance;
            font->CharAdvance(wcx, wcx, advance);
            advancedWidth = (mStyles[0].kerning + advance) * mStyles[0].size;
        }
    }
    WrapText(buffer.begin(), len, charWidths, lines, rect, 1);
    ConstructMeshes(lines, rect, 1);
    if (mScrollEnabled) {
        mWidth = oldWidth;
        mNumClones = 1;
        mScrollTimeMs = 0;
        mScrollSpeed = mScrollRate * advancedWidth * -0.001f;
        if (mFitType == kFitScrollMarqueeWrapAlways) {
            mScrollOffset = mWidth;
            mEnterOffset = mWidth;
            mNumSubTexts++;
            mFullStringLength = mIndentation * (float)mNumSubTexts + charWidths[len];
            mEnterCheckPts.push_back(mFullStringLength);
            mEnterCheckPts.push_front(0);
            mExitCheckPts.push_back(mFullStringLength);
            mCloneOffset = mFullStringLength;

            for (float w = mFullStringLength; mFullStringLength > 0 && w <= mWidth;
                 w += mFullStringLength) {
                mNumClones++;
            }

            mEnteredIdx = -1;
            mExitedIdx = -1;
        } else {
            mScrollOffset = 0;
            mFullStringLength = charWidths[len];
            mCloneOffset = 0;
        }
        mScrollTriggerTime = mScrollDelay;
        for (int i = 0; i < mFontMap.size(); i++) {
            mFontMap[i]->SetupScrolling();
        }
    }
}

void RndText::FitTextEllipsis() {
    BuildFontMaps(true);
    std::vector<Line> lines;
    std::vector<unsigned short> buffer;
    int len = ConvertTextToWide(mText.c_str(), buffer);
    float *charWidths = (float *)_alloca((len + 5) * sizeof(float));
    OnComputeCharWidths(buffer.begin(), charWidths, false);

    Hmx::Rect r;

    if (charWidths[len] > mWidth) {
        int wlen = wcslen(L"...");
        int max = len;
        int min = 1;
        while (max > min + 1) {
            int mid = (min + max) >> 1;
            if (charWidths[mid] >= mWidth) {
                max = mid;
            } else {
                min = mid;
            }
        }
        int bigLen = min + wlen;

        unsigned short *u11 =
            (unsigned short *)_alloca((min + wlen + 1) * sizeof(unsigned short));
        memcpy(u11, buffer.begin(), min * 2);
        if (mFixedLength > wlen + 1 && bigLen > mFixedLength) {
            bigLen = mFixedLength;
        }
        int diff = bigLen - wlen;
        wcscpy((wchar_t *)&u11[diff], L"...");
        BuildFontMaps(true);
        OnComputeCharWidths(u11, charWidths, false);
        WrapText(u11, bigLen, charWidths, lines, r, 1);

        while (diff > 1
               && (lines.size() > 1 || r.w >= mWidth || wcschr(L" .,", u11[diff - 1]))) {
            int i6 = bigLen-- * 0xe >> 4;
            for (int i = bigLen; i >= i6; i--) {
                if (u11[i] == 0x20) {
                    bigLen = i + wlen;
                    break;
                }
            }

            diff = bigLen - wlen;
            wcscpy((wchar_t *)&u11[diff], L"...");
            BuildFontMaps(true);
            OnComputeCharWidths(u11, charWidths, false);
            WrapText(u11, bigLen, charWidths, lines, r, 1);
        }

    } else {
        WrapText(buffer.begin(), len, charWidths, lines, r, 1);
    }

    ConstructMeshes(lines, r, 1);
}

void RndText::UpdateScrollOffsets() {
    float delta = TheTaskMgr.DeltaUISeconds();
    mScrollTimeMs += delta;
    if (mScrollTimeMs >= mScrollTriggerTime) {
        bool b11 = false;
        float widthDiff = mFullStringLength - mWidth;
        float f13 = mScrollSpeed * delta * 1000;
        mScrollOffset += f13;
        switch (mFitType) {
        case kFitScrollMarqueeWrap:
            if (mScrollOffset < -mFullStringLength) {
                mScrollOffset = 0;
            }
            mCloneOffset = mFullStringLength;
            break;
        case kFitScrollMarqueeReset:
            if (mScrollOffset < -(mFullStringLength + 20)) {
                mScrollOffset = 0;
                b11 = true;
            }
            break;
        case kFitScrollPingPong:
            if (mScrollSpeed >= 0) {
                if (mScrollSpeed > 0 && mScrollOffset >= 0) {
                    mScrollSpeed = -mScrollSpeed;
                    mScrollOffset = 0;
                    b11 = true;
                }
            } else if (mScrollOffset < -widthDiff) {
                mScrollOffset = -widthDiff;
                mScrollSpeed = -mScrollSpeed;
                b11 = true;
            }
            break;
        case kFitScrollMarqueeWrapAlways: {
            static Message textScrolledIn("text_scrolled_in", -1);
            static Message textScrolledOut("text_scrolled_out", -1);
            float old8c = mEnterOffset;
            mEnterOffset += f13;
            float f14 = mEnterCheckPts.front();
            if (mWidth - (old8c + f13) >= f14) {
                mEnteredIdx++;
                if (mEnteredIdx >= mNumSubTexts) {
                    mEnteredIdx = 0;
                }
                textScrolledIn[0] = mEnteredIdx;
                if (f14 == mFullStringLength) {
                    mEnterOffset = mWidth;
                }
                if (mNumSubTexts == mEnterCheckPts.size()) {
                    mEnterCheckPts.push_back(mFullStringLength);
                }
                mEnterCheckPts.pop_front();
                if (mEventReceiver) {
                    mEventReceiver->Handle(textScrolledIn, false);
                }
            }
            float f15 = mExitCheckPts.front();
            if (mScrollOffset <= -f15) {
                mExitedIdx++;
                textScrolledOut[0] = mExitedIdx;
                if (f15 == mFullStringLength) {
                    mScrollOffset = 0;
                    mExitedIdx = -1;
                }
                mExitCheckPts.push_back(mScrollOffset);
                mExitCheckPts.pop_front();
                if (mEventReceiver) {
                    mEventReceiver->Handle(textScrolledOut, false);
                }
            }
            break;
        }
        default:
            mScrollOffset = 0;
            break;
        }

        if (b11) {
            mScrollTimeMs = 0;
        }
    }
    FOREACH (it, mFontMap) {
        (*it)->UpdateScrolling(mScrollOffset);
    }
}

const unsigned short *
RndText::ParseMarkup(const unsigned short *us1, StyleState &state, unsigned short &usref) {
    const unsigned short *p = us1 + 1;
    bool is2f = *p == 0x2F;
    if (is2f) {
        p++;
    }
    usref = 0;
    if (WStrniCmp(p, (const unsigned short *)L"sup", 3) == 0) {
        if (is2f) {
            state.size = state.scale * state.curStyle->size;
        } else {
            state.size = state.scale * state.curStyle->size * gSuperscriptScale;
        }
        p += 3;
    } else if (WStrniCmp(p, (const unsigned short *)L"gtr", 3) == 0) {
        if (is2f) {
            state.size = state.scale * state.curStyle->size;
        } else {
            state.size = state.scale * state.curStyle->size * gGuitarScale;
        }
        if (is2f) {
            state.zOffset = state.zOffset;
        } else {
            state.zOffset = gGuitarZOffset;
        }
        p += 3;
    } else if (WStrniCmp(p, (const unsigned short *)L"it", 2) == 0) {
        if (is2f) {
            state.italics = state.curStyle->italics;
        } else {
            state.italics = state.curStyle->italics + 0.1f;
        }
        p += 2;
    } else if (WStrniCmp(p, (const unsigned short *)L"color", 5) == 0) {
        if (is2f) {
            state.textColor = state.curStyle->textColor;
            p += 5;
        } else {
            int r, g, b;
            int a = state.textColor.alpha * 256.0f;
            swscanf((const wchar_t *)p, L"%d %d %d %d", r, g, b, a);
            state.textColor.red = (float)r * 0.003921569f;
            state.textColor.green = (float)g * 0.003921569f;
            state.textColor.blue = (float)b * 0.003921569f;
            state.textColor.alpha = (float)a * 0.003921569f;
            p += 6;
        }
    } else if (WStrniCmp(p, (const unsigned short *)L"&#", 2) == 0) {
        int x = 0x3F;
        swscanf((const wchar_t *)p, L"%d", x);
        usref = x;
        p += 2;
    } else if (WStrniCmp(p, (const unsigned short *)L"nobreak", 7) == 0) {
        if (is2f) {
            state.brk = true;
        } else {
            state.brk = false;
        }
        p += 7;
    } else if (WStrniCmp(p, (const unsigned short *)L"alt", 3) == 0) {
        p += 3;
        if (is2f) {
            for (; *p != L'>' && *p != L'\0'; p++)
                ;
        }
        // more...
    }

    while (true) {
        if (*p == L'>') {
            return ++p;
        }
        if (*p == L'\0') {
            break;
        }
        p++;
    }

    return p;
}

void RndText::ReplaceMissingCharacters(std::vector<unsigned short> &buffer) {
    std::map<RndFontBase *, std::set<unsigned short> > missingChars;
    std::vector<unsigned short> origBufferCopy;
    bool copied = false;
    StyleState state(this, 1);
    auto vecIt = buffer.begin();
    unsigned short curUS = *vecIt;
    while (curUS != 0) {
        curUS = *vecIt;
        if (curUS == '<' && mMarkup) {
            vecIt = (unsigned short *)ParseMarkup(vecIt, state, curUS);
            vecIt--;
        } else if (curUS != 10 && state.fontMapIndex != -1) {
            FontMapBase *base = mFontMap[state.fontMapIndex];
            if (base) {
                RndFontBase *font = base->Font();
                if (font && !font->CharDefined(curUS)) {
                    missingChars[font].insert(curUS);

                    curUS = 0;
                    unsigned short shorts[] = { 0x25A1, '?', '#', '*', '!', '9' };
                    for (int i = 0; i < (int)DIM(shorts); i++) {
                        unsigned short curShort = shorts[i];
                        if (font->CharDefined(curShort)) {
                            curUS = curShort;
                            break;
                        }
                    }
                    if (curUS == 0) {
                        std::vector<unsigned short> validChars = font->Chars();
                        for (int i = 0; i < validChars.size(); i++) {
                            unsigned short cur = validChars[i];
                            bool isSpace = cur == ' ' || cur == 0xA0;
                            if (!isSpace) {
                                curUS = cur;
                                break;
                            }
                        }
                    }
                    if (curUS != 0) {
                        if (!copied) {
                            origBufferCopy = buffer;
                            copied = true;
                        }
                        *vecIt = curUS;
                    }
                }
            }
        }
        curUS = *++vecIt;
    }
    FOREACH (it, missingChars) {
        RndFontBase *key = it->first;
        auto &set = it->second;
        String outputStr = MakeString(
            "%s:%s char%s (", PathName(this), TextToken(), set.size() > 1 ? "s" : ""
        );
        FOREACH (charIt, set) {
            unsigned short curChar = *charIt;
            bool displayable =
                !(curChar < 0x20 || curChar >= 0xFF || curChar == 0x25
                  || curChar == 0x7F);

            outputStr += MakeString(
                "%s'%c' 0x%02X",
                charIt == set.begin() ? "" : ", ",
                displayable ? (char)curChar : '?',
                curChar
            );
        }
        outputStr += MakeString(") missing from %s in string \"", PathName(key));
        for (int i = 0; i < origBufferCopy.size(); i++) {
            unsigned short curChar = origBufferCopy[i];
            if (curChar == 0) {
                break;
            }
            bool displayable =
                !(curChar < 0x20 || curChar >= 0xFF || curChar == 0x25
                  || curChar == 0x7F);

            if (displayable) {
                outputStr += MakeString("%c", (char)curChar);
            } else {
                outputStr += MakeString("\\x%02X", curChar);
            }
        }
        outputStr += "\"";
        MILO_NOTIFY(outputStr.c_str());
    }
}

void RndText::QueueBlacklightPacket(RndMesh *pMesh, float cloneOffset, int numClones) {
    unsigned int count = sBlacklightPacketCount;
    unsigned int cursize = sBlacklightPacketPool.capacity();
    if (count >= cursize) {
        unsigned int newsize = 8;
        if (cursize != 0) {
            newsize = cursize * 2;
        }
        sBlacklightPacketPool.resize(newsize);
    }
    BlacklightPacket &packet = sBlacklightPacketPool[sBlacklightPacketCount++];
    packet.mpMesh = pMesh;
    packet.mColor = pMesh->Mat()->GetColor();
    packet.mCloneOffset = cloneOffset;
    packet.mNumClones = numClones;
    packet.mpCam = RndCam::Current();
}

void RndText::ClearBlacklight() { sBlacklightPacketCount = 0; }

void RndText::DrawBlacklight() {
    RndCam *cur = RndCam::Current();
    for (int i = 0; i < sBlacklightPacketCount; i++) {
        BlacklightPacket &packet = sBlacklightPacketPool[i];
        if (packet.mpCam && packet.mpCam != RndCam::Current()) {
            packet.mpCam->Select();
        }
        packet.mpMesh->Mat()->SetColor(packet.mColor);
        DrawMesh(packet.mpMesh, packet.mCloneOffset, packet.mNumClones);
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

int RndText::ConvertTextToWide(
    const char *utf8, stlpmtx_std::vector<unsigned short> &buffer
) {
    char empty = 0;
    if (!utf8) {
        utf8 = &empty;
    }
    int len = strlen(utf8);

    {
        MemDoTempAllocations tmp;
        buffer.resize(len * 2 + 1);
        buffer[0] = 0;
    }

    unsigned short *p = buffer.begin();

    if (mCapsMode == kForceUpper) {
        unsigned short ezkett;
        DecodeUTF8(ezkett, "\xC3\x9F");
        unsigned short *u4;
        if (mFixedLength) {
            u4 = p + mFixedLength;
        } else {
            u4 = nullptr;
        }
        for (; *utf8 != '\0' && p != u4; ++p) {
            unsigned short w;
            utf8 += DecodeUTF8(w, utf8);
            if (w == ezkett) {
                *p++ = 0x53;
                if (p == u4)
                    break;
                *p = 0x53;
            } else {
                *p = WToUpper(w);
            }
        }
    } else if (mCapsMode == kForceLower) {
        while (*utf8 != '\0') {
            unsigned short w;
            utf8 += DecodeUTF8(w, utf8);
            *p++ = WToLower(w);
        }
    } else {
        while (*utf8 != '\0') {
            utf8 += DecodeUTF8(*p++, utf8);
        }
    }

    *p = 0;
    ReplaceMissingCharacters(buffer);
    return p - buffer.begin();
}

#pragma endregion
