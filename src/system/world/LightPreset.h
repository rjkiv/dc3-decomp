#pragma once
#include "math/Color.h"
#include "math/Mtx.h"
#include "obj/Object.h"
#include "rndobj/Anim.h"
#include "rndobj/Env.h"
#include "rndobj/EventTrigger.h"
#include "rndobj/Lit.h"
#include "rndobj/Trans.h"
#include "utl/MemMgr.h"
#include "utl/Str.h"
#include "world/Spotlight.h"
#include "world/SpotlightDrawer.h"

class LightPreset : public RndAnimatable {
public:
    struct EnvironmentEntry {
        Hmx::Color mAmbientColor; // 0x0
        bool mFogEnable; // 0x10
        float mFogStart; // 0x14
        float mFogEnd; // 0x18
        Hmx::Color mFogColor; // 0x1c
    };

    struct EnvLightEntry {
        Hmx::Quat unk0;
        Vector3 mPosition; // 0x10
        Hmx::Color mColor; // 0x20
        float mRange; // 0x30
        RndLight::Type mLightType; // 0x34
        Hmx::Matrix3 unk38; // 0x38
    };

    struct SpotlightEntry {
        float mIntensity; // 0x0
        int mColor; // 0x4 - packed
        bool mFlareEnabled; // 0x8
        ObjPtr<RndTransformable> mTarget; // 0xc
        Hmx::Quat unk10;
        Hmx::Matrix3 unk20;
    };

    struct SpotlightDrawerEntry {
        float mTotalIntensity; // 0x0
        float mBaseIntensity; // 0x4
        float mSmokeIntensity; // 0x8
        float mLightInfluence; // 0xc
    };

    struct Keyframe {
        Keyframe(Hmx::Object *);

        String unk0; // 0x0
        ObjVector<SpotlightEntry> mSpotlightEntries; // 0x8
        std::vector<EnvironmentEntry> mEnvironmentEntries; // 0x18
        std::vector<EnvLightEntry> mLightEntries; // 0x24
        std::vector<SpotlightDrawerEntry> mSpotlightDrawerEntries; // 0x30
        ObjPtrList<EventTrigger> mTriggers; // 0x3c
    };

    // Hmx::Object
    virtual ~LightPreset();
    virtual bool Replace(ObjRef *, Hmx::Object *);
    OBJ_CLASSNAME(LightPreset);
    OBJ_SET_TYPE(LightPreset);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // RndAnimatable
    virtual void StartAnim();
    virtual void SetFrame(float, float);
    virtual float EndFrame();

    OBJ_MEM_OVERLOAD(0x1B)
    NEW_OBJ(LightPreset)

protected:
    LightPreset();

    void Clear();

    ObjVector<Keyframe> mKeyframes; // 0x10
    ObjPtrVec<Spotlight> mSpotlights; // 0x20
    ObjPtrVec<RndEnviron> mEnvironments; // 0x3c
    ObjPtrVec<RndLight> mLights; // 0x58
    ObjPtrVec<SpotlightDrawer> mSpotlightDrawers; // 0x74
    bool unk90;
    Symbol unk94;
    int unk98;
    ObjPtrList<EventTrigger> unk9c;
    bool unkb0;
    ObjVector<SpotlightEntry> unkb4;
    std::vector<EnvironmentEntry> mEnvironmentState; // 0xc4
    std::vector<EnvLightEntry> mLightState; // 0xd0
    std::vector<SpotlightDrawerEntry> mSpotlightDrawerState; // 0xdc
    int unke8;
    float unkec;
    float unkf0;
    float unkf4;
    int unkf8;
    int unkfc;
    float unk100;
    float unk104;
    bool unk108;
    int unk10c;
};
