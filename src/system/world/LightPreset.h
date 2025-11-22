#pragma once
#include "SpotlightDrawer.h"
#include "math/Color.h"
#include "math/Mtx.h"
#include "math/Vec.h"
#include "obj/Object.h"
#include "os/Platform.h"
#include "rndobj/Anim.h"
#include "rndobj/Env.h"
#include "rndobj/EventTrigger.h"
#include "rndobj/Lit.h"
#include "rndobj/Trans.h"
#include "utl/MemMgr.h"
#include "utl/Str.h"
#include "world/LightHue.h"
#include "world/Spotlight.h"
#include "world/SpotlightDrawer.h"
#include <deque>

/** "Represents an animated sequence of states of certain
    objects in the world. For now, we store states for Spotlight and
    Environment objects." */
class LightPreset : public RndAnimatable {
public:
    struct EnvironmentEntry {
        EnvironmentEntry() : mFogEnable(0), mFogStart(0), mFogEnd(0) {
            mAmbientColor.Zero();
            mFogColor.Zero();
        }

        /** "Ambient color" */
        Hmx::Color mAmbientColor; // 0x0
        /** "Fog showing?" */
        bool mFogEnable; // 0x10
        /** "Intensity from smoke" */
        float mFogStart; // 0x14
        /** "Intensity from smoke" */
        float mFogEnd; // 0x18
        /** "Intensity from smoke" */
        Hmx::Color mFogColor; // 0x1c
    };

    struct EnvLightEntry {
        EnvLightEntry();

        Hmx::Quat unk0;
        /** "Light's position" */
        Vector3 mPosition; // 0x10
        /** "Light's color" */
        Hmx::Color mColor; // 0x20
        /** "Falloff distance for point lights" */
        float mRange; // 0x30
        /** "Light type" */
        RndLight::Type mLightType; // 0x34
        /** "Light transform" */
        Hmx::Matrix3 mRotation; // 0x38
    };

    struct SpotlightEntry {
        enum {
            kEnabled = 1
            // there's a flag for 2 but idk what it is
        };

        SpotlightEntry(Hmx::Object *owner)
            : mIntensity(0), mColor(0), unk8(3), mTarget(owner) {
            unk10.Reset();
            unk20.Zero();
        }

        float mIntensity; // 0x0
        int mColor; // 0x4 - packed
        unsigned char unk8; // 0x8
        ObjPtr<RndTransformable> mTarget; // 0xc
        Hmx::Quat unk10;
        Hmx::Matrix3 unk20;
    };

    struct SpotlightDrawerEntry {
        SpotlightDrawerEntry()
            : mTotalIntensity(0), mBaseIntensity(0), mSmokeIntensity(0),
              mLightInfluence(0) {}

        /** "Global intensity scale" */
        float mTotalIntensity; // 0x0
        /** "Intensity of smokeless beam" */
        float mBaseIntensity; // 0x4
        /** "Intensity from smoke" */
        float mSmokeIntensity; // 0x8
        /** "The amount the spotlights will influence the real lighting of the world" */
        float mLightInfluence; // 0xc
    };

    struct Keyframe {
        Keyframe(Hmx::Object *);

        /** "Description of the keyframe" */
        String mDescription; // 0x0
        ObjVector<SpotlightEntry> mSpotlightEntries; // 0x8
        std::vector<EnvironmentEntry> mEnvironmentEntries; // 0x18
        std::vector<EnvLightEntry> mLightEntries; // 0x24
        std::vector<SpotlightDrawerEntry> mSpotlightDrawerEntries; // 0x30
        /** "Trigger to fire when keyframe starts blending (deprecated)" */
        ObjPtrList<EventTrigger> mTriggers; // 0x3c
        std::vector<bool> mSpotlightChanges; // 0x50
        std::vector<bool> mEnvironmentChanges; // 0x5c
        std::vector<bool> mLightChanges; // 0x68
        std::vector<bool> mSpotlightDrawerChanges; // 0x74
        /** "Duration of the keyframe" */
        float mDuration; // 0xa0
        /** "Fade-out time of the keyframe" */
        float mFadeOutTime; // 0xa4
        float unka8; // 0xa8
    };

    enum KeyframeCmd {
        kPresetKeyframeFirst,
        kPresetKeyframeNext,
        kPresetKeyframePrev,
        kPresetKeyframeNum
    };

    enum PresetObject {
        kPresetSpotlight,
        kPresetSpotlightDrawer,
        kPresetEnv,
        kPresetLight
    };

    friend const char *GetName(LightPreset *preset, int idx, PresetObject obj);

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
    void SetHue(LightHue *hue) { mHue = hue; }

    int GetCurrentKeyframe(void) const;
    bool PlatformOk(void) const;

protected:
    LightPreset();

    void Clear();
    void CacheFrames();
    void GetKey(float, int &, int &, float &) const;
    void RemoveLight(int);
    void RemoveSpotlight(int);
    void RemoveSpotlightDrawer(int);
    void RemoveEnvironment(int);
    void AddLight(RndLight *);
    void AddSpotlightDrawer(SpotlightDrawer *);
    void AddEnvironment(RndEnviron *);
    void AdvanceManual(LightPreset::KeyframeCmd);
    int NextManualFrame(LightPreset::KeyframeCmd) const;
    void FillLightPresetData(RndLight *, LightPreset::EnvLightEntry &);
    void AnimateLightFromPreset(RndLight *, const LightPreset::EnvLightEntry &, float);
    void ApplyState(LightPreset::Keyframe const &);

    static std::deque<std::pair<KeyframeCmd, float> > sManualEvents;

    ObjVector<Keyframe> mKeyframes; // 0x10
    ObjPtrVec<Spotlight> mSpotlights; // 0x20
    ObjPtrVec<RndEnviron> mEnvironments; // 0x3c
    ObjPtrVec<RndLight> mLights; // 0x58
    ObjPtrVec<SpotlightDrawer> mSpotlightDrawers; // 0x74
    /** "Whether this preset loops its animation" */
    bool mLooping; // 0x90
    /** "Category for preset-picking" */
    Symbol mCategory; // 0x94
    /** "Limit this shot to given platform" - the options are kPlatformNone/PS3/Xbox */
    Platform mPlatformOnly; // 0x98
    /** "Triggers to fire upon selection (deprecated)" */
    ObjPtrList<EventTrigger> mSelectTriggers; // 0x9c
    /** "Whether this is a manual keyframe (keyframes controlled by MIDI)" */
    bool mManual; // 0xb0
    ObjVector<SpotlightEntry> mSpotlightState; // 0xb4
    std::vector<EnvironmentEntry> mEnvironmentState; // 0xc4
    std::vector<EnvLightEntry> mLightState; // 0xd0
    std::vector<SpotlightDrawerEntry> mSpotlightDrawerState; // 0xdc
    Keyframe *mLastKeyframe; // 0xe8
    float mLastBlend; // 0xec
    float mStartBeat; // 0xf0
    float mManualFrameStart; // 0xf4
    int mManualFrame; // 0xf8
    int mLastManualFrame; // 0xfc
    float mManualFadeTime; // 0x100
    float unk104;
    /** "Whether the keyframes are locked (no editing allowed)" */
    bool mLocked; // 0x108
    LightHue *mHue; // 0x10c
};
