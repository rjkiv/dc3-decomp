#pragma once
#include "SpotlightDrawer.h"
#include "math/Color.h"
#include "math/Mtx.h"
#include "math/Vec.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Platform.h"
#include "rndobj/Anim.h"
#include "rndobj/Env.h"
#include "rndobj/EventTrigger.h"
#include "rndobj/Lit.h"
#include "rndobj/Trans.h"
#include "utl/BinStream.h"
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
        EnvironmentEntry();
        void Save(BinStream &d) const;
        void Load(BinStream &d);
        void Animate(const EnvironmentEntry &other, float blend);
        bool operator!=(const EnvironmentEntry &other) const;

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
        void Save(BinStream &d) const;
        void Load(BinStream &d);
        void Animate(const EnvLightEntry &other, float blend);
        bool operator!=(const EnvLightEntry &other) const;

        Hmx::Quat mOrientation; // 0x0
        /** "Light's position" */
        Vector3 mPosition; // 0x10
        /** "Light's color" */
        Hmx::Color mColor; // 0x20
        /** "Falloff distance for point lights" */
        float mRange; // 0x30
        /** "Light type" */
        unsigned int mType; // 0x34
        /** "Light transform" */
        Hmx::Matrix3 mCachedDisplayRot; // 0x38
    };

    enum SpotlightFlags {
        kFlareEnabled = 1,
        kTargetExists = 2
    };

    struct SpotlightEntry {
        SpotlightEntry(Hmx::Object *owner);
        void Save(BinStream &d) const;
        void Load(BinStreamRev &d);
        bool operator!=(const SpotlightEntry &other) const;
        void CalculateDirection(Spotlight *slight, Hmx::Quat &out) const;
        void Animate(Spotlight *slight, const SpotlightEntry &other, float blend);

        float mIntensity; // 0x0
        unsigned int mColor; // 0x4 - packed
        unsigned char mFlags; // 0x8
        ObjPtr<RndTransformable> mTarget; // 0xc
        Hmx::Quat mOrientation; // 0x20
        Hmx::Matrix3 mCachedDisplayRot; // 0x30
    };

    struct SpotlightDrawerEntry {
        SpotlightDrawerEntry();
        void Save(BinStream &d) const;
        void Load(BinStreamRev &d);
        bool operator!=(const SpotlightDrawerEntry &other) const;

        /** "Global intensity scale" */
        float mTotal; // 0x0
        /** "Intensity of smokeless beam" */
        float mBaseIntensity; // 0x4
        /** "Intensity from smoke" */
        float mSmokeIntensity; // 0x8
        /** "The amount the spotlights will influence the real lighting of the world" */
        float mLightingInfluence; // 0xc
    };

    struct Keyframe {
        Keyframe(Hmx::Object *owner);
        void Save(BinStream &d) const;
        void Load(BinStreamRev &d);
        void LegacyLoadP9(BinStreamRev &d);
        void LegacyLoadStageKit(BinStream &d);

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
        float mFrame; // 0xa8
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

    enum PropogateFlags {
        kColoring = 1,
        kTargeting = 2,
        kPropogateAll = -1,
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
    virtual float EndFrame() { return mCachedDuration; }

    OBJ_MEM_OVERLOAD(0x1B)
    NEW_OBJ(LightPreset)

    int GetCurrentKeyframe() const;
    bool PlatformOk() const;
    void SetSpotlight(Spotlight *spot, int propogateFlags);
    void OnKeyframeCmd(KeyframeCmd cmd);
    void ResetEvents();
    void SetFrameEx(float frame, float blend, bool same);

    void SetHue(LightHue *hue) { mHue = hue; }
    Symbol GetCategory() const { return mCategory; }
    bool GetManual() const { return mManual; }
    const std::vector<Keyframe> &GetKeyframes() const { return mKeyframes; }
    const ObjPtrVec<Spotlight> &GetSpotlights() const { return mSpotlights; }
    const ObjPtrVec<RndEnviron> &GetEnvironments() const { return mEnvironments; }
    const ObjPtrVec<RndLight> &GetLights() const { return mLights; }
    const ObjPtrVec<SpotlightDrawer> &GetSpotlightDrawers() const {
        return mSpotlightDrawers;
    }

protected:
    LightPreset();

    void Clear();
    void CacheFrames();
    void GetKey(float frame, int &prev, int &next, float &alpha) const;
    void RemoveLight(int index);
    void RemoveSpotlight(int index);
    void RemoveSpotlightDrawer(int index);
    void RemoveEnvironment(int index);
    void AddLight(RndLight *light);
    void AddSpotlightDrawer(SpotlightDrawer *sd);
    void AddEnvironment(RndEnviron *env);
    void AdvanceManual(LightPreset::KeyframeCmd cmd);
    int NextManualFrame(LightPreset::KeyframeCmd cmd) const;
    void FillLightPresetData(RndLight *l, LightPreset::EnvLightEntry &entry);
    void AnimateLightFromPreset(
        RndLight *l, const LightPreset::EnvLightEntry &entry, float blend
    );
    void AnimateEnvFromPreset(
        RndEnviron *e, const LightPreset::EnvironmentEntry &entry, float blend
    );
    void AnimateSpotFromPreset(
        Spotlight *spot, const LightPreset::SpotlightEntry &entry, float blend
    );
    void ApplyState(LightPreset::Keyframe const &frame);
    void SetKeyframe(Keyframe &frame);
    void FillEnvPresetData(RndEnviron *e, EnvironmentEntry &entry);
    void FillSpotlightDrawerPresetData(SpotlightDrawer *, SpotlightDrawerEntry &);
    void AddSpotlight(Spotlight *light, bool initToBlack);
    void FillSpotPresetData(Spotlight *spot, SpotlightEntry &entry, int propogateType);
    void Animate(float blend);
    void SyncNewSpotlights();
    void SyncKeyframeTargets();
    void AnimateState(
        const LightPreset::Keyframe &frame,
        const LightPreset::Keyframe &nextFrame,
        float blend
    );

    DataNode OnSetKeyframe(DataArray *);
    DataNode OnViewKeyframe(DataArray *);

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
    const Keyframe *mLastKeyframe; // 0xe8
    float mLastBlend; // 0xec
    float mStartBeat; // 0xf0
    float mManualFrameStart; // 0xf4
    int mManualFrame; // 0xf8
    int mLastManualFrame; // 0xfc
    float mManualFadeTime; // 0x100
    float mCachedDuration; // 0x104
    /** "Whether the keyframes are locked (no editing allowed)" */
    bool mLocked; // 0x108
    LightHue *mHue; // 0x10c
};
