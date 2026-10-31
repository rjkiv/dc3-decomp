#pragma once
#include "BoxMap.h"
#include "Lit.h"
#include "math/Vec.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Timer.h"
#include "rndobj/ColorXfm.h"
#include "rndobj/Draw.h"
#include "rndobj/Trans.h"
#include "rndobj/Lit.h"
#include "utl/MemMgr.h"

class RndEnviron : public RndTransformable, public RndDrawable {
public:
    virtual ~RndEnviron();
    virtual bool Replace(ObjRef *, Hmx::Object *);
    OBJ_CLASSNAME(Environ);
    OBJ_SET_TYPE(Environ);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    virtual void Highlight() { RndTransformable::Highlight(); }
    virtual void Select(const Vector3 *pos);
    virtual void UpdateApproxLighting(const Vector3 *pos);
    virtual int NumLights_Real() const { return mLights_Real.size(); }
    virtual int NumLights_Approx() const { return mLights_Approx.size(); }
    virtual bool IsFake(RndLight *l) const;
    virtual bool IsReal(RndLight *l) const;
    virtual void Draw();

    OBJ_MEM_OVERLOAD(0x1B);
    NEW_OBJ(RndEnviron)
    REGISTER_OBJ(RndEnviron)
    static RndEnviron *Current() { return sCurrent; }
    static Vector3 *CurrentPos() {
        if (sCurrentPosSet)
            return &sCurrentPos;
        else
            return nullptr;
    }
    static BoxMapLighting &GetGlobalBoxMap() { return sGlobalLighting; }

    void SetUseApproxLocal(bool b) { mUseApprox_Local = b; }
    void SetUseApproxGlobal(bool b) { mUseApprox_Global = b; }
    void SetUseApproxes(bool b) {
        SetUseApproxLocal(b);
        SetUseApproxGlobal(b);
    }
    bool GetUseApprox() const { return mUseApprox_Local || mUseApprox_Global; }
    bool UsesApproxLocal() const { return mUseApprox_Local; }
    bool UsesApproxGlobal() const { return mUseApprox_Global; }

    const Transform &ColorXfm() const;
    bool FogEnable() const;
    void SetFogEnable(bool b) { mAmbientFogOwner->mFogEnable = b; }
    Transform LRFadeRef() const;
    void RemoveLight(RndLight *l);
    void AddLight(RndLight *l);
    bool IsValidRealLight(const RndLight *l) const;
    const Hmx::Color &AmbientColor() const { return mAmbientFogOwner->mAmbientColor; }
    void SetAmbientColor(const Hmx::Color &col) {
        mAmbientFogOwner->mAmbientColor.Set(col.red, col.green, col.blue);
    }
    bool FadeOut() const { return mFadeOut; }
    bool UseColorXfm() const { return mUseColorXfm; }
    bool GetAmbientOcclusionEnabled() const { return mAOEnabled; }
    float GetAmbientOcclusionStrength() const { return mAOStrength; }
    bool GetUseToneMapping() const { return mUseToneMapping; }
    float FadeStart() const { return mFadeStart; }
    float FadeEnd() const { return mFadeEnd; }
    bool GetAnimateFromPreset() const { return mAnimateFromPreset; }
    const Hmx::Color &FogColor() const { return mAmbientFogOwner->mFogColor; }
    void SetFogColor(const Hmx::Color &c) { mAmbientFogOwner->mFogColor = c; }
    float GetFogStart() const { return mAmbientFogOwner->mFogStart; }
    float GetFogEnd() const { return mAmbientFogOwner->mFogEnd; }
    void SetFogRange(float start, float end) {
        mAmbientFogOwner->mFogStart = start;
        mAmbientFogOwner->mFogEnd = end;
    }
    ObjPtrList<RndLight> &Lights_Real() { return mLights_Real; }
    const ObjPtrList<RndLight> &Lights_Real() const { return mLights_Real; }
    ObjPtrList<RndLight> &Lights_Approx() { return mLights_Approx; }
    const ObjPtrList<RndLight> &Lights_Approx() const { return mLights_Approx; }

protected:
    RndEnviron();

    bool IsLightInList(const RndLight *light, const ObjPtrList<RndLight> &list) const;
    void OnRemoveAllLights();
    void ReclassifyLights();
    DataNode OnAllowableLights_Real(const DataArray *);
    DataNode OnAllowableLights_Approx(const DataArray *);

    static BoxMapLighting sGlobalLighting;
    static RndEnviron *sCurrent;
    static Vector3 sCurrentPos;
    static bool sCurrentPosSet;

    ObjPtrList<RndLight> mLights_Real; // 0x100
    ObjPtrList<RndLight> mLights_Approx; // 0x114
    ObjPtrList<RndLight> mLights_Old; // 0x128
    Hmx::Color mAmbientColor; // 0x13c
    ObjOwnerPtr<RndEnviron> mAmbientFogOwner; // 0x14c
    bool mFogEnable; // 0x160
    float mFogStart; // 0x164
    float mFogEnd; // 0x168
    Hmx::Color mFogColor; // 0x16c
    bool mFadeOut; // 0x17c
    float mFadeStart; // 0x180
    float mFadeEnd; // 0x184
    float mFadeMax; // 0x188
    ObjPtr<RndTransformable> mLRFadeRef; // 0x18c
    Vector4 mLRFade; // 0x1a0, mLeftOut, mLeftOpaque, mRightOpaque, mRightOut
    RndColorXfm mColorXfm; // 0x1b0
    bool mUseColorXfm; // 0x244
    bool mAnimateFromPreset; // 0x245
    bool mAOEnabled; // 0x246
    float mAOStrength; // 0x248
    Timer mUpdateTimer; // 0x250
    float mIntensityAverage; // 0x280
    float mIntensityRate; // 0x284
    float mExposure; // 0x288
    float mWhitePoint; // 0x28c
    bool mUseToneMapping; // 0x290
    bool mUseApprox_Local; // 0x291
    bool mUseApprox_Global; // 0x292
};

class RndEnvironTracker {
public:
    RndEnvironTracker(RndEnviron *env, const Vector3 *pos)
        : mOld(RndEnviron::Current()), mOldPosSet(RndEnviron::CurrentPos()) {
        if (mOldPosSet) {
            mOldPos = *RndEnviron::CurrentPos();
        } else {
            mOldPos.Zero();
        }
        if (env) {
            if (env != RndEnviron::Current()
                || !VecEqual(pos, RndEnviron::CurrentPos())) {
                env->Select(pos);
            }
        }
    }
    ~RndEnvironTracker() {
        Vector3 *vptr = mOldPosSet ? &mOldPos : nullptr;
        if (mOld) {
            if (mOld != RndEnviron::Current()
                || !VecEqual(vptr, RndEnviron::CurrentPos())) {
                mOld->Select(vptr);
            }
        }
    }

protected:
    bool VecEqual(const Vector3 *a, const Vector3 *b) const {
        if (a && b) {
            return *a == *b;
        } else
            return a == b;
    }

    RndEnviron *mOld; // 0x0
    Vector3 mOldPos; // 0x4
    bool mOldPosSet; // 0x10
};
