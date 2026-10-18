#include "rndobj/TexProc.h"
#include "Rnd.h"
#include "ShaderMgr.h"
#include "math/Geo.h"
#include "math/Rot.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "os/Debug.h"
#include "rndobj/BaseMaterial.h"
#include "rndobj/Cam.h"
#include "rndobj/Draw.h"
#include "rndobj/Mat.h"
#include "rndobj/Rnd_NG.h"
#include "rndobj/ShaderOptions.h"
#include "rndobj/Tex.h"
#include "utl/BinStream.h"

float gAmpTemp = 0.3f;
float gFreqTemp = 1.0f;
RndCam *TexProc::mCam;

TexProc::TexProc()
    : mInputTexture(this), mOutputTexture(this), mShader(kShaderTwirl), mDrawPreClear(1),
      mParams(0), mFrequency(0), mAmplitude(0), mAmplitudeBase(0), mAmplitudeBump(0),
      mPhase(0), mPhaseVel(0) {}

BEGIN_HANDLERS(TexProc)
    HANDLE_SUPERCLASS(RndDrawable)
    HANDLE_SUPERCLASS(Hmx::Object)
    HANDLE(set_params, OnSetParams)
END_HANDLERS

BEGIN_PROPSYNCS(TexProc)
    SYNC_PROP(input_texture, mInputTexture)
    SYNC_PROP(output_texture, mOutputTexture)
    SYNC_PROP(shader, (int &)mShader)
    SYNC_PROP_MODIFY(draw_pre_clear, mDrawPreClear, UpdatePreClearState())
    SYNC_PROP(frequency, mFrequency)
    SYNC_PROP(amplitude, mAmplitudeBase)
    SYNC_PROP(amplitude_bump, mAmplitudeBump)
    SYNC_PROP(phase, mPhase)
    SYNC_PROP(phase_vel, mPhaseVel)
    SYNC_SUPERCLASS(RndDrawable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(TexProc)
    SAVE_REVS(3, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(RndDrawable)
    bs << mInputTexture;
    bs << mOutputTexture;
    bs << mShader;
    bs << mDrawPreClear;
    bs << mFrequency;
    bs << mAmplitudeBase;
    bs << mAmplitudeBump;
    bs << mPhaseVel;
END_SAVES

BEGIN_COPYS(TexProc)
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(RndDrawable)
    CREATE_COPY(TexProc)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mInputTexture)
        COPY_MEMBER(mOutputTexture)
        COPY_MEMBER(mShader)
        COPY_MEMBER(mDrawPreClear)
        COPY_MEMBER(mFrequency)
        COPY_MEMBER(mAmplitude)
        COPY_MEMBER(mPhase)
        COPY_MEMBER(mPhaseVel)
        COPY_MEMBER(mAmplitudeBase)
        COPY_MEMBER(mAmplitudeBump)
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(3, 0)

BEGIN_LOADS(TexProc)
    LOAD_REVS(bs)
    ASSERT_REVS(3, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    LOAD_SUPERCLASS(RndDrawable)
    d >> mInputTexture;
    d >> mOutputTexture;
    d >> (int &)mShader;
    d >> mDrawPreClear;
    mDrawPreClear = true;
    if (d.rev > 1) {
        d >> mFrequency;
        d >> mAmplitudeBase;
        if (d.rev > 2) {
            d >> mAmplitudeBump;
        } else {
            float x;
            d >> x;
        }
        d >> mPhaseVel;
    }
END_LOADS

void TexProc::UpdatePreClearState() {
    TheRnd.PreClearDrawAddOrRemove(this, mDrawPreClear, false);
}

void TexProc::Poll() {
    float beat = TheTaskMgr.Beat();
    mPhase = mPhaseVel * beat * 2 * PI;
    float f2 = (1.0f - (beat - (float)(int)beat)) - 0.5f;
    if (f2 < 0) {
        f2 = 0;
    }
    float f1 = sin(f2 * 2 * PI);
    mAmplitude = mAmplitudeBump * f1 * f2 + mAmplitudeBase;
}

void TexProc::Init() {
    TexProc::Register();
    MILO_ASSERT(!mCam, 0x3A);
    mCam = ObjectDir::Main()->New<RndCam>("[tex proc cam]");
}

RndMat *TexProc::SetUpWorkingMat() {
    RndMat *mat = TheShaderMgr.GetWork();
    mat->SetZMode(BaseMaterial::kDisable);
    mat->SetBlend(BaseMaterial::kBlendSrc);
    mat->SetTexWrap(BaseMaterial::kClamp);
    mat->SetAlphaWrite(true);
    mat->SetAlphaCut(false);
    return mat;
}

void TexProc::SetRegisters() {
    TheShaderMgr.SetPConstant(
        (PShaderConstant)0x40,
        Vector4(
            mFrequency * gFreqTemp,
            mFrequency * gFreqTemp,
            mFrequency * gFreqTemp,
            mFrequency * gFreqTemp
        )
    );
    TheShaderMgr.SetPConstant(
        (PShaderConstant)0x41,
        Vector4(
            mAmplitude * gAmpTemp,
            mAmplitude * gAmpTemp,
            mAmplitude * gAmpTemp,
            mAmplitude * gAmpTemp
        )
    );
    TheShaderMgr.SetPConstant(
        (PShaderConstant)0x42, Vector4(mPhase, mPhase, mPhase, mPhase)
    );
}

DataNode TexProc::OnSetParams(DataArray *a) {
    if (CheckParams(a, true)) {
        if (mParams)
            mParams->Release();
        mParams = new DataArray(a->Size() - 2);
        SetParams(mParams, a);
    } else {
        MILO_NOTIFY("----- TexProc::OnSetParams() - one or more parameters is invalid");
    }
    return 0;
}

bool TexProc::CheckParams(DataArray *a, bool b) {
    int aSize = a->Size();
    bool b7 = true;
    if (!b && !(aSize >= 1 && aSize <= 4)) {
        b7 = false;
    }
    for (int i = b ? 2 : 0; i < aSize && b7; i++) {
        switch (a->Type(i)) {
        case kDataFloat:
            b7 &= true;
            break;
        case kDataArray:
            b7 &= CheckParams(a->Array(i), false);
            break;
        case kDataProperty: {
            DataNode eval = a->Evaluate(i);
            switch (eval.Type()) {
            case kDataFloat:
                b7 &= true;
                break;
            case kDataArray:
                b7 &= CheckParams(eval.Array(), false);
                break;
            default:
                b7 = false;
                break;
            }
            break;
        }
        default:
            b7 = false;
            break;
        }
    }
    return b7;
}

void TexProc::SetParams(DataArray *a1, DataArray *a2) {
    int aSize = a2->Size();
    for (int i = 2; i < aSize; i++) {
        switch (a2->Type(i)) {
        case kDataFloat:
            a1->Node(i - 2) = a2->Float(i);
            break;
        case kDataArray: {
            DataArray *arr = new DataArray(a2->Array(i)->Size());
            SetParams(arr, a2->Array(i));
            a1->Node(i - 2) = a2->Array(i);
            break;
        }
        case kDataProperty: {
            DataNode eval = a2->Evaluate(i);
            if (eval.Type() != kDataFloat) {
                if (eval.Type() == kDataArray) {
                    DataArray *arr = new DataArray(eval.Array()->Size());
                    SetParams(arr, eval.Array());
                    a1->Node(i - 2) = a2->Array(i);
                }
            } else {
                a1->Node(i - 2) = a2->Float(i);
            }
            break;
        }
        default:
            break;
        }
    }
}

void TexProc::DrawToTexture() {
    if (TheRnd.DrawMode() == Rnd::kDrawNormal && Showing() && mInputTexture
        && mOutputTexture) {
        RndMat *workingMat = SetUpWorkingMat();
        ShaderType shaderType;
        switch (mShader) {
        case kShaderTwirl:
            shaderType = kTwirlShader;
            break;
        case kShaderKillAlpha:
            shaderType = kKillAlphaShader;
            break;
        default:
            shaderType = kTwirlShader;
            break;
        }
        RndCam *cur = RndCam::Current();
        RndTex *tex = RndCam::Current()->TargetTex();
        if (tex) {
            MILO_NOTIFY_ONCE(
                "%s: Cannot render to texture (%s) while already rendering to texture (%s).",
                PathName(tex),
                PathName(this),
                PathName(tex)
            );
        }
        mCam->SetTargetTex(mOutputTexture);
        mCam->Select();
        workingMat->SetDiffuseTex(mInputTexture);
        workingMat->SetNormalMap(mInputTexture);
        Poll();
        SetRegisters();
        Hmx::Rect rect(0, 0, mOutputTexture->Width(), mOutputTexture->Height());
        Hmx::Color color;
        TheNgRnd.DrawRect(rect, workingMat, shaderType, color, nullptr, nullptr);
        mCam->SetTargetTex(nullptr);
        cur->Select();
    }
}
