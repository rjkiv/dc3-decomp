#include "rndobj/MatAnim.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rndobj/Anim.h"
#include "rndobj/Tex.h"
#include "utl/BinStream.h"

Hmx::Object *RndMatAnim::sOwner;

#pragma region Hmx::Object

RndMatAnim::RndMatAnim() : mMat(this), mKeysOwner(this, this), mTexKeys(this) {}

BEGIN_HANDLERS(RndMatAnim)
    HANDLE_SUPERCLASS(RndAnimatable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(RndMatAnim)
    SYNC_SUPERCLASS(RndAnimatable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(RndMatAnim)
    SAVE_REVS(7, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(RndAnimatable)
    bs << mMat << mKeysOwner << mColorKeys;
    bs << mAlphaKeys;
    bs << mTransKeys << mScaleKeys << mRotKeys << mTexKeys;
END_SAVES

BEGIN_COPYS(RndMatAnim)
    CREATE_COPY_AS(RndMatAnim, m)
    MILO_ASSERT(m, 0xF2);
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(RndAnimatable)
    COPY_MEMBER_FROM(m, mMat)
    if (ty == kCopyShallow || (ty == kCopyFromMax && m->mKeysOwner != m))
        COPY_MEMBER_FROM(m, mKeysOwner)
    else {
        sOwner = this;
        mKeysOwner = this;
        mColorKeys = m->mKeysOwner->mColorKeys;
        mAlphaKeys = m->mKeysOwner->mAlphaKeys;
        mTransKeys = m->mKeysOwner->mTransKeys;
        mScaleKeys = m->mKeysOwner->mScaleKeys;
        mRotKeys = m->mKeysOwner->mRotKeys;
        mTexKeys = m->mKeysOwner->mTexKeys;
    }
END_COPYS

INIT_REVS(7, 0)

BEGIN_LOADS(RndMatAnim)
    LOAD_REVS(bs)
    ASSERT_REVS(7, 0)
    if (d.rev > 5) {
        LOAD_SUPERCLASS(Hmx::Object)
    }
    LOAD_SUPERCLASS(RndAnimatable)
    sOwner = this;
    d >> mMat;
    if (d.rev < 7) {
        LoadStages(d);
    }
    d >> mKeysOwner;
    if (!mKeysOwner) {
        mKeysOwner = this;
    }
    if (d.rev > 1) {
        Keys<Hmx::Color, Hmx::Color> k1;
        Keys<Hmx::Color, Hmx::Color> k2;
        if (d.rev < 5)
            d >> k1;
        if (d.rev < 3)
            d >> k2;
        d >> mColorKeys;
        if (d.rev < 4) {
            Keys<Hmx::Color, Hmx::Color> k3;
            d >> k3;
        }
        d >> mAlphaKeys;
        if (d.rev < 5 && mColorKeys.empty()) {
            if (!k1.empty())
                mColorKeys = k1;
            else if (!k2.empty())
                mColorKeys = k2;
        }
    }
    if (d.rev > 6) {
        d >> mTransKeys >> mScaleKeys >> mRotKeys;
        // d >> mTexKeys;
    }
END_LOADS

void RndMatAnim::Print() {
    TheDebug << "   mat: " << mMat << "\n";
    TheDebug << "   transKeys: " << mTransKeys << "\n";
    TheDebug << "   scaleKeys: " << mScaleKeys << "\n";
    TheDebug << "   rotKeys:" << mRotKeys << "\n";
    TheDebug << "   texKeys: " << mTexKeys << "\n";
    TheDebug << "   keysOwner: " << mKeysOwner << "\n";
    TheDebug << "   baseKeys: " << mColorKeys << "\n";
    TheDebug << "   alphaKeys: " << mAlphaKeys << "\n";
}

#pragma endregion
#pragma region RndAnimatable

void RndMatAnim::SetFrame(float f1, float f2) {
    RndAnimatable::SetFrame(f1, f2);
    if (mMat) {
        Vector3 v68;
        Vector3 v74;
        Transform t58(mMat->TexXfm());
        if (!TransKeys().empty()) {
            if (f2 != 1.0f) {
                TransKeys().AtFrame(f1, v68);
                Interp(t58.v, v68, f2, t58.v);
            } else
                TransKeys().AtFrame(f1, t58.v);
        }
        if (!RotKeys().empty()) {
            RotKeys().AtFrame(f1, v68);
            if (f2 != 1.0f) {
                Vector3 v80;
                if (ScaleKeys().empty())
                    MakeEuler(t58.m, v80);
                else
                    MakeEulerScale(t58.m, v80, v74);
                Interp(v80, v68, f2, v68);
            }
            MakeRotMatrix(v68, t58.m, true);
        }
        if (!ScaleKeys().empty()) {
            ScaleKeys().AtFrame(f1, v68);
            if (f2 != 1.0f) {
                Interp(v74, v68, f2, v68);
            }
            Scale(v68, t58.m, t58.m);
        }
        if (!TransKeys().empty() || !RotKeys().empty() || !ScaleKeys().empty()) {
            mMat->SetTexXfm(t58);
        }
        if (!GetTexKeys().empty()) {
            RndTex *tex;
            GetTexKeys().AtFrame(f1, tex);
            mMat->SetDiffuseTex(tex);
        }
        Hmx::Color col(mMat->GetColor());
        if (!ColorKeys().empty()) {
            ColorKeys().AtFrame(f1, col);
            if (f2 != 1.0f) {
                Interp(mMat->GetColor(), col, f2, col);
            }
            mMat->SetColor(col.red, col.green, col.blue);
        }
        if (!AlphaKeys().empty()) {
            float alpha = mMat->Alpha();
            AlphaKeys().AtFrame(f1, alpha);
            if (f2 != 1.0f) {
                Interp(mMat->Alpha(), alpha, f2, alpha);
            }
            mMat->SetAlpha(alpha);
        }
    }
}

float RndMatAnim::EndFrame() {
    float end = Max(TransKeys().LastFrame(), ScaleKeys().LastFrame());
    end = Max(end, GetTexKeys().LastFrame(), RotKeys().LastFrame());
    end = Max(end, AlphaKeys().LastFrame(), ColorKeys().LastFrame());
    return end;
}

void RndMatAnim::SetKey(float frame) {
    if (mMat) {
        Vector3 v28, v34;
        MakeEulerScale(mMat->TexXfm().m, v28, v34);
        TransKeys().Add(mMat->TexXfm().v, frame, true);
        RotKeys().Add(v28, frame, true);
        ScaleKeys().Add(v34, frame, true);
        GetTexKeys().Add(mMat->GetDiffuseTex(), frame, true);
        ColorKeys().Add(mMat->GetColor(), frame, true);
        AlphaKeys().Add(mMat->Alpha(), frame, true);
    }
}

#pragma endregion
#pragma region RndMatAnim

void RndMatAnim::LoadStage(BinStreamRev &d) {
    if (d.rev < 2) {
        MILO_NOTIFY("Can't convert old MatAnim stages");
    }
    if (d.rev > 0) {
        d >> mTransKeys >> mScaleKeys >> mRotKeys;
    }
    // if (d.rev > 1) {
    //     d >> mTexKeys;
    // }
}

int RndMatAnim::TexKeys::Add(RndTex *tex, float frame, bool b) {
    sOwner = mOwner;
    return Keys<TexPtr, RndTex *>::Add(tex, frame, b);
}

void RndMatAnim::TexKeys::operator=(const RndMatAnim::TexKeys &keys) {
    if (this != &keys) {
        sOwner = mOwner;
        resize(keys.size());
        TexKeys::iterator it = begin();
        TexKeys::const_iterator otherIt = keys.begin();
        for (; it != end(); ++it, ++otherIt) {
            *it = *otherIt;
        }
    }
}

void Interp(
    const RndMatAnim::TexPtr &texPtr, const RndMatAnim::TexPtr &, float, RndTex *&tex
) {
    tex = texPtr;
}
