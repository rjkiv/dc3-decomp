#include "world/LightPreset.h"
#include "LightPreset.h"
#include "SpotlightDrawer.h"
#include "math/Color.h"
#include "math/Mtx.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rndobj/Anim.h"
#include "rndobj/Cam.h"
#include "rndobj/Env.h"
#include "rndobj/EventTrigger.h"
#include "rndobj/Lit.h"
#include "rndobj/PostProc.h"
#include "utl/BinStream.h"
#include "utl/Loader.h"
#include "world/Spotlight.h"
#include <float.h>

LightPreset *gEditPreset;
std::deque<std::pair<LightPreset::KeyframeCmd, float> > LightPreset::sManualEvents;

static bool sLoading;
class AutoLoading {
public:
    AutoLoading() { sLoading = true; }
    ~AutoLoading() { sLoading = false; }
};

#pragma region EnvironmentEntry

LightPreset::EnvironmentEntry::EnvironmentEntry()
    : mFogEnable(0), mFogStart(0), mFogEnd(0) {
    mAmbientColor.Zero();
    mFogColor.Zero();
}

void LightPreset::EnvironmentEntry::Save(BinStream &bs) const {
    bs << mAmbientColor;
    bs << mFogEnable;
    bs << mFogStart;
    bs << mFogEnd;
    bs << mFogColor;
}

void LightPreset::EnvironmentEntry::Load(BinStream &bs) {
    bs >> mAmbientColor;
    bs >> mFogEnable;
    bs >> mFogStart;
    bs >> mFogEnd;
    bs >> mFogColor;
}

void LightPreset::EnvironmentEntry::Animate(
    const LightPreset::EnvironmentEntry &entry, float f2
) {
    Interp(mAmbientColor, entry.mAmbientColor, f2, mAmbientColor);
    if (entry.mFogEnable) {
        Interp(mFogColor, entry.mFogColor, f2, mFogColor);
        Interp(mFogStart, entry.mFogStart, f2, mFogStart);
        Interp(mFogEnd, entry.mFogEnd, f2, mFogEnd);
    } else {
        float far = RndCam::Current() ? RndCam::Current()->FarPlane() : FLT_MAX;
        Interp(mFogStart, far, f2, mFogStart);
        Interp(mFogEnd, far, f2, mFogEnd);
    }
    if (f2 == 1) {
        mFogEnable = entry.mFogEnable;
    }
}

bool LightPreset::EnvironmentEntry::operator!=(
    const LightPreset::EnvironmentEntry &e
) const {
    if (mFogEnable != e.mFogEnable)
        return true;
    else if (mFogStart != e.mFogStart)
        return true;
    else if (mFogEnd != e.mFogEnd)
        return true;
    else if (mAmbientColor != e.mAmbientColor)
        return true;
    else
        return mFogColor != e.mFogColor;
}

BinStream &operator<<(BinStream &bs, const LightPreset::EnvironmentEntry &e) {
    e.Save(bs);
    return bs;
}

BinStreamRev &operator>>(BinStreamRev &d, LightPreset::EnvironmentEntry &e) {
    e.Load(d.stream);
    return d;
}

#pragma endregion
#pragma region EnvLightEntry

LightPreset::EnvLightEntry::EnvLightEntry() : mRange(0), mLightType(RndLight::kPoint) {
    unk0.Reset();
    mPosition.Zero();
    mColor.Zero();
    mRotation.Zero();
}

void LightPreset::EnvLightEntry::Save(BinStream &bs) const {
    bs << unk0;
    bs << mPosition;
    bs << mColor;
    bs << mRange;
    bs << mLightType;
}

void LightPreset::EnvLightEntry::Load(BinStream &bs) {
    bs >> unk0;
    bs >> mPosition;
    bs >> mColor;
    mColor.alpha = 1;
    bs >> mRange;
    bs >> (int &)mLightType;
}

void LightPreset::EnvLightEntry::Animate(
    const LightPreset::EnvLightEntry &entry, float f2
) {
    Interp(mColor, entry.mColor, f2, mColor);
    Interp(mRange, entry.mRange, f2, mRange);
    Interp(unk0, entry.unk0, f2, unk0);
    Interp(mPosition, entry.mPosition, f2, mPosition);
}

bool LightPreset::EnvLightEntry::operator!=(const LightPreset::EnvLightEntry &e) const {
    if (mRange != e.mRange)
        return true;
    else if ((unsigned int)mLightType != e.mLightType)
        return true;
    else if (unk0 != e.unk0)
        return true;
    else if (mPosition != e.mPosition)
        return true;
    else
        return mColor != e.mColor;
}

BinStream &operator<<(BinStream &bs, const LightPreset::EnvLightEntry &e) {
    e.Save(bs);
    return bs;
}

BinStreamRev &operator>>(BinStreamRev &d, LightPreset::EnvLightEntry &e) {
    e.Load(d.stream);
    return d;
}

#pragma endregion
#pragma region SpotlightEntry

LightPreset::SpotlightEntry::SpotlightEntry(Hmx::Object *owner)
    : mIntensity(0), mColor(0), mFlags(3), mTarget(owner) {
    mOrientation.Reset();
    unk30.Zero();
}

void LightPreset::SpotlightEntry::Save(BinStream &bs) const {
    Hmx::Color color(mColor);
    bs << mIntensity;
    bs << mOrientation;
    bs << color;
    bs << mTarget;
    bs << (bool)(mFlags & 1);
}

void LightPreset::SpotlightEntry::Load(BinStreamRev &d) {
    float intensity;
    d >> intensity;
    mIntensity = intensity;
    d >> mOrientation;
    Hmx::Color color;
    d >> color;
    color.alpha = 1;
    mColor = color.Pack();
    if (!mTarget.Load(d.stream, false, nullptr)) {
        mFlags &= ~2;
    }
    if (d.rev < 0x13) {
        Symbol s;
        d >> s;
    }
    if (d.rev > 1) {
        bool b;
        d >> b;
        if (b) {
            mFlags |= kEnabled;
        } else {
            mFlags &= ~kEnabled;
        }
        if (d.rev < 9) {
            int x;
            d >> x;
        }
    }
    if (mTarget || !(mFlags & 2)) {
        mOrientation.Set(0, 0, 0, 0);
    }
}

void LightPreset::SpotlightEntry::CalculateDirection(Spotlight *s, Hmx::Quat &q) const {
    q = mOrientation;
    if ((mFlags & 2) && mTarget) {
        Hmx::Matrix3 m38;
        s->CalculateDirection(mTarget, m38);
        q = Hmx::Quat(m38);
    }
}

void LightPreset::SpotlightEntry::Animate(
    Spotlight *spot, const LightPreset::SpotlightEntry &entry, float f3
) {
    float fout;
    Interp(mIntensity, entry.mIntensity, f3, fout);
    Hmx::Color c38(mColor);
    Hmx::Color c48(entry.mColor);
    Interp(c38, c48, f3, c38);
    mColor = c38.Pack();
    Hmx::Quat q58;
    CalculateDirection(spot, q58);
    Hmx::Quat q68;
    entry.CalculateDirection(spot, q68);
    Interp(q58, q68, f3, mOrientation);
    if (f3 == 1) {
        mFlags = entry.mFlags;
        mTarget = entry.mTarget;
    }
}

bool LightPreset::SpotlightEntry::operator!=(const LightPreset::SpotlightEntry &e) const {
    return e.mIntensity != mIntensity || e.mFlags != mFlags || e.mTarget != mTarget
        || (unsigned int)e.mColor != mColor || e.mOrientation != mOrientation;
}

BinStream &operator<<(BinStream &bs, const LightPreset::SpotlightEntry &e) {
    e.Save(bs);
    return bs;
}

BinStreamRev &operator>>(BinStreamRev &d, LightPreset::SpotlightEntry &e) {
    e.Load(d);
    return d;
}

#pragma endregion
#pragma region SpotlightDrawerEntry

LightPreset::SpotlightDrawerEntry::SpotlightDrawerEntry()
    : mTotalIntensity(0), mBaseIntensity(0), mSmokeIntensity(0), mLightInfluence(0) {}

void LightPreset::SpotlightDrawerEntry::Save(BinStream &bs) const {
    bs << mBaseIntensity;
    bs << mSmokeIntensity;
    bs << mTotalIntensity;
    bs << mLightInfluence;
}

void LightPreset::SpotlightDrawerEntry::Load(BinStreamRev &d) {
    d >> mBaseIntensity;
    d >> mSmokeIntensity;
    d >> mTotalIntensity;
    if (d.rev > 0xF) {
        d >> mLightInfluence;
    } else {
        mLightInfluence = 1;
    }
}

bool LightPreset::SpotlightDrawerEntry::operator!=(
    const LightPreset::SpotlightDrawerEntry &e
) const {
    if (mBaseIntensity != e.mBaseIntensity)
        return true;
    else if (mSmokeIntensity != e.mSmokeIntensity)
        return true;
    else if (mLightInfluence != e.mLightInfluence)
        return true;
    else if (mTotalIntensity != e.mTotalIntensity)
        return true;
    else
        return false;
}

BinStream &operator<<(BinStream &bs, const LightPreset::SpotlightDrawerEntry &e) {
    e.Save(bs);
    return bs;
}

BinStreamRev &operator>>(BinStreamRev &d, LightPreset::SpotlightDrawerEntry &e) {
    e.Load(d);
    return d;
}

#pragma endregion
#pragma region Keyframe

LightPreset::Keyframe::Keyframe(Hmx::Object *owner)
    : mSpotlightEntries(owner), mTriggers(owner), mDuration(0), mFadeOutTime(0),
      unka8(-1) {
    LightPreset *preset = dynamic_cast<LightPreset *>(owner);
    MILO_ASSERT(preset, 0x56F);

    mSpotlightEntries.resize(preset->mSpotlights.size());
    mEnvironmentEntries.resize(preset->mEnvironments.size());
    mLightEntries.resize(preset->mLights.size());
    mSpotlightDrawerEntries.resize(preset->mSpotlightDrawers.size());
    if (!sLoading)
        preset->SetKeyframe(*this);
}

void LightPreset::Keyframe::Save(BinStream &bs) const {
    bs << mDuration;
    bs << mFadeOutTime;
    bs << mSpotlightEntries;
    bs << mEnvironmentEntries;
    bs << mLightEntries;
    bs << mDescription;
    bs << mSpotlightDrawerEntries;
    bs << mTriggers;
}

void LightPreset::Keyframe::Load(BinStreamRev &d) {
    MILO_ASSERT(d.rev != 14, 0x5A3);
    d >> mDuration;
    d >> mFadeOutTime;
    d >> mSpotlightEntries;
    d >> mEnvironmentEntries;
    d >> mLightEntries;
    if (d.rev > 5) {
        d >> mDescription;
    }
    if (d.rev > 9) {
        d >> mSpotlightDrawerEntries;
    }
    if (d.rev > 0x11 && d.rev < 0x16) {
        ObjPtr<RndPostProc> pp(mSpotlightEntries.Owner());
        d >> pp;
    }
    if (d.rev > 0x13) {
        d >> mTriggers;
    }
    if (d.rev > 0xB && d.rev < 0x16) {
        LegacyLoadStageKit(d.stream);
    }
}

void LightPreset::Keyframe::LegacyLoadStageKit(BinStream &bs) {
    for (int i = 0; i < 9; i++) {
        int x;
        bs >> x;
    }
}

void LightPreset::Keyframe::LegacyLoadP9(BinStreamRev &d) {
    MILO_ASSERT(d.rev == 14, 0x596);
    d >> mDescription;
    d >> mSpotlightEntries;
    d >> mEnvironmentEntries;
    d >> mLightEntries;
    d >> mSpotlightDrawerEntries;
    LegacyLoadStageKit(d.stream);
}

BinStream &operator<<(BinStream &bs, const LightPreset::Keyframe &k) {
    k.Save(bs);
    return bs;
}

BinStreamRev &operator>>(BinStreamRev &d, LightPreset::Keyframe &k) {
    k.Load(d);
    return d;
}

#pragma region LightPreset

LightPreset::LightPreset()
    : mKeyframes(this), mSpotlights(this, (EraseMode)0, kObjListOwnerControl),
      mEnvironments(this, (EraseMode)0, kObjListOwnerControl),
      mLights(this, (EraseMode)0, kObjListOwnerControl),
      mSpotlightDrawers(this, (EraseMode)0, kObjListOwnerControl), mLooping(0),
      mPlatformOnly(kPlatformNone), mSelectTriggers(this), mManual(0),
      mSpotlightState(this), mLastKeyframe(0), mLastBlend(-1), mStartBeat(0),
      mManualFrameStart(0), mManualFrame(0), mLastManualFrame(-1), mManualFadeTime(0),
      mEndFrame(0), mLocked(0), mHue(0) {}

LightPreset::~LightPreset() { Clear(); }

bool LightPreset::Replace(ObjRef *from, Hmx::Object *to) {
    auto spotIt = mSpotlights.FindRef(from);
    if (spotIt != mSpotlights.end()) {
        mSpotlights.Set(spotIt, to ? dynamic_cast<Spotlight *>(to) : nullptr);
        if (!*spotIt) {
            RemoveSpotlight(&*spotIt - &*mSpotlights.begin());
        }
        CacheFrames();
        return true;
    }
    auto envIt = mEnvironments.FindRef(from);
    if (envIt != mEnvironments.end()) {
        mEnvironments.Set(envIt, to ? dynamic_cast<RndEnviron *>(to) : nullptr);
        if (!*envIt) {
            RemoveEnvironment(&*envIt - &*mEnvironments.begin());
        }
        CacheFrames();
        return true;
    }
    auto lightIt = mLights.FindRef(from);
    if (lightIt != mLights.end()) {
        mLights.Set(lightIt, to ? dynamic_cast<RndLight *>(to) : nullptr);
        if (!*lightIt) {
            RemoveLight(&*lightIt - &*mLights.begin());
        }
        CacheFrames();
        return true;
    }
    auto spotDrawIt = mSpotlightDrawers.FindRef(from);
    if (spotDrawIt != mSpotlightDrawers.end()) {
        mSpotlightDrawers.Set(
            spotDrawIt, to ? dynamic_cast<SpotlightDrawer *>(to) : nullptr
        );
        if (!*spotDrawIt) {
            RemoveSpotlightDrawer(&*spotDrawIt - &*mSpotlightDrawers.begin());
        }
        CacheFrames();
        return true;
    }
    return Hmx::Object::Replace(from, to);
}

BEGIN_HANDLERS(LightPreset)
    HANDLE(set_keyframe, OnSetKeyframe)
    HANDLE(view_keyframe, OnViewKeyframe)
    HANDLE_ACTION(next, OnKeyframeCmd(kPresetKeyframeNext))
    HANDLE_ACTION(prev, OnKeyframeCmd(kPresetKeyframePrev))
    HANDLE_ACTION(first, OnKeyframeCmd(kPresetKeyframeFirst))
    HANDLE_ACTION(reset_events, ResetEvents())
    HANDLE_SUPERCLASS(RndAnimatable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

void LightPreset::ResetEvents() { sManualEvents.clear(); }

template <class T>
__forceinline const char *GetObjName(const ObjPtrVec<T> &vec, int idx) {
    if (idx >= vec.size())
        return "<obj index out of bounds>";
    else if (!vec[idx])
        return "<obj not found>";
    else
        return vec[idx]->Name();
}

const char *GetName(LightPreset *preset, int idx, LightPreset::PresetObject obj) {
    switch (obj) {
    case LightPreset::kPresetSpotlight:
        return GetObjName(preset->mSpotlights, idx);
    case LightPreset::kPresetSpotlightDrawer:
        return GetObjName(preset->mSpotlightDrawers, idx);
    case LightPreset::kPresetEnv:
        return GetObjName(preset->mEnvironments, idx);
    case LightPreset::kPresetLight:
        return GetObjName(preset->mLights, idx);
    default:
        return "<invalid preset object>";
    }
}

BEGIN_CUSTOM_PROPSYNC(LightPreset::EnvironmentEntry)
    SYNC_PROP_SET(
        environment, GetName(gEditPreset, _prop->Int(_i - 1), LightPreset::kPresetEnv),
    )
    SYNC_PROP(ambient_color, o.mAmbientColor)
    SYNC_PROP_SET(fog_enable, o.mFogEnable, )
    SYNC_PROP_SET(fog_start, o.mFogStart, )
    SYNC_PROP_SET(fog_end, o.mFogEnd, )
    SYNC_PROP(fog_color, o.mFogColor)
END_CUSTOM_PROPSYNC

BEGIN_CUSTOM_PROPSYNC(LightPreset::EnvLightEntry)
    SYNC_PROP_SET(
        light, GetName(gEditPreset, _prop->Int(_i - 1), LightPreset::kPresetLight),
    )
    SYNC_PROP(position, o.mPosition)
    SYNC_PROP_SET(color, o.mColor.Pack(), )
    SYNC_PROP_SET(range, o.mRange, )
    SYNC_PROP_SET(type, RndLight::TypeToStr(o.mLightType), ) {
        static Symbol _s("rotation");
        if (sym == _s) {
            MakeRotMatrix(o.unk0, o.mRotation);
            if (PropSync(o.mRotation, _val, _prop, _i + 1, _op))
                return true;
            else
                return false;
        }
    }
END_CUSTOM_PROPSYNC

BEGIN_CUSTOM_PROPSYNC(LightPreset::SpotlightEntry)
    SYNC_PROP_SET(
        spotlight,
        GetName(gEditPreset, _prop->Int(_i - 1), LightPreset::kPresetSpotlight),
    )
    SYNC_PROP_SET(intensity, o.mIntensity, )
    SYNC_PROP_SET(color, (int)o.mColor, )
    SYNC_PROP(target, o.mTarget)
    SYNC_PROP_SET(flare_enabled, o.mFlags & LightPreset::SpotlightEntry::kEnabled, ) {
        static Symbol _s("rotation");
        if (sym == _s) {
            MakeRotMatrix(o.mOrientation, o.unk30);
            if (PropSync(o.unk30, _val, _prop, _i + 1, _op))
                return true;
            else
                return false;
        }
    }
END_CUSTOM_PROPSYNC

BEGIN_CUSTOM_PROPSYNC(LightPreset::SpotlightDrawerEntry)
    SYNC_PROP_SET(
        spotlight_drawer,
        GetName(gEditPreset, _prop->Int(_i - 1), LightPreset::kPresetSpotlightDrawer),
    )
    SYNC_PROP_SET(total, o.mTotalIntensity, )
    SYNC_PROP_SET(base_intensity, o.mBaseIntensity, )
    SYNC_PROP_SET(smoke_intensity, o.mSmokeIntensity, )
    SYNC_PROP_SET(light_influence, o.mLightInfluence, )
END_CUSTOM_PROPSYNC

BEGIN_CUSTOM_PROPSYNC(LightPreset::Keyframe)
    SYNC_PROP(description, o.mDescription)
    SYNC_PROP(duration, o.mDuration)
    SYNC_PROP(fade_out, o.mFadeOutTime)
    SYNC_PROP(spotlight_entries, o.mSpotlightEntries)
    SYNC_PROP(spotlight_drawer_entries, o.mSpotlightDrawerEntries)
    SYNC_PROP(environment_entries, o.mEnvironmentEntries)
    SYNC_PROP(light_entries, o.mLightEntries)
    SYNC_PROP(triggers, o.mTriggers)
END_CUSTOM_PROPSYNC

BEGIN_PROPSYNCS(LightPreset)
    gEditPreset = this;
    SYNC_PROP_MODIFY(keyframes, mKeyframes, CacheFrames())
    SYNC_PROP(looping, mLooping)
    SYNC_PROP(category, mCategory)
    SYNC_PROP(select_triggers, mSelectTriggers)
    SYNC_PROP(manual, mManual)
    SYNC_PROP(locked, mLocked)
    SYNC_PROP(platform_only, (int &)mPlatformOnly)
    SYNC_PROP(hue, mHue)
    SYNC_SUPERCLASS(RndAnimatable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(LightPreset)
    SAVE_REVS(0x16, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(RndAnimatable)
    bs << mKeyframes;
    bs << mSpotlights;
    bs << mEnvironments;
    bs << mLights;
    bs << mLooping;
    bs << mCategory;
    bs << mSelectTriggers;
    bs << mManual;
    bs << mLocked;
    bs << mPlatformOnly;
    bs << mSpotlightDrawers;
END_SAVES

BEGIN_COPYS(LightPreset)
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(RndAnimatable)
    CREATE_COPY(LightPreset)
    BEGIN_COPYING_MEMBERS
        Clear();
        COPY_MEMBER(mKeyframes)
        COPY_MEMBER(mSpotlights)
        COPY_MEMBER(mEnvironments)
        COPY_MEMBER(mLights)
        COPY_MEMBER(mSpotlightDrawers)
        mSpotlightState.resize(mSpotlights.size());
        mEnvironmentState.resize(mEnvironments.size());
        mLightState.resize(mLights.size());
        COPY_MEMBER(mLooping)
        COPY_MEMBER(mCategory)
        COPY_MEMBER(mSelectTriggers)
        COPY_MEMBER(mManual)
        COPY_MEMBER(mLocked)
        COPY_MEMBER(mPlatformOnly)
        CacheFrames();
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(0x16, 0)

BEGIN_LOADS(LightPreset)
    AutoLoading al;
    Clear();
    LOAD_REVS(bs)
    ASSERT_REVS(0x16, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    if (d.rev != 0xE) {
        LOAD_SUPERCLASS(RndAnimatable)
        d >> mKeyframes;
    } else {
        mKeyframes.resize(1);
        mKeyframes[0].LegacyLoadP9(d);
    }
    d >> mSpotlights;
    d >> mEnvironments;
    d >> mLights;
    if (d.rev < 5) {
        bool b;
        d >> b;
        if (b) {
            Keyframe k(this);
            d >> k;
        }
    }
    if (d.rev != 0xE) {
        d >> mLooping;
    }
    d >> mCategory;
    if (d.rev != 0xE && d.rev < 0x11) {
        std::vector<Symbol> syms;
        d >> syms;
        if (syms.size() > 0 && syms[0] != "") {
            mCategory = syms[0];
        }
    }
    String cat(mCategory);
    cat.ToLower();
    mCategory = cat.c_str();
    if (d.rev < 7) {
        String str;
        d >> str;
        if (!str.empty()) {
            MILO_NOTIFY("%s: %s", Name(), str);
        }
    } else if (d.rev < 0x15) {
        ObjPtr<EventTrigger> trig(this);
        d >> trig;
        if (trig) {
            mSelectTriggers.push_back(trig);
        }
    } else {
        d >> mSelectTriggers;
    }
    if (d.rev < 5) {
        String str;
        d >> str;
    }
    if (d.rev != 0xE) {
        if (d.rev < 0x16) {
            int x;
            d >> x;
        }
        int x;
        if (d.rev > 0 && d.rev < 0x11) {
            d >> x;
        }
        if (d.rev > 2 && d.rev < 0x11) {
            d >> x;
        }
    }
    if (d.rev > 3) {
        if (d.rev != 0xE) {
            d >> mManual;
        }
        d >> mLocked;
    }
    if (d.rev > 0xC) {
        d >> (int &)mPlatformOnly;
    }
    if (d.rev > 9) {
        d >> mSpotlightDrawers;
    }
    if (d.rev == 0xB) {
        int dummy;
        for (int i = 0; i < 8; i++) {
            d >> dummy;
        }
    }
    mSpotlightState.resize(mSpotlights.size());
    mEnvironmentState.resize(mEnvironments.size());
    mLightState.resize(mLights.size());
    mSpotlightDrawerState.resize(mSpotlightDrawers.size());

    for (uint i = 0; i != mSpotlights.size(); i++) {
        if (!mSpotlights[i] || !mSpotlights[i]->GetAnimateFromPreset()) {
            RemoveSpotlight(i);
            i--;
        }
    }
    for (uint i = 0; i != mEnvironments.size(); i++) {
        if (!mEnvironments[i] || !mEnvironments[i]->GetAnimateFromPreset()) {
            RemoveEnvironment(i);
            i--;
        }
    }
    for (uint i = 0; i != mLights.size(); i++) {
        if (!mLights[i] || !mLights[i]->GetAnimateFromPreset()) {
            RemoveLight(i);
            i--;
        }
    }
    for (uint i = 0; i != mSpotlightDrawers.size(); i++) {
        if (!mSpotlightDrawers[i]) {
            RemoveSpotlightDrawer(i);
            i--;
        }
    }
    SyncNewSpotlights();
    CacheFrames();
    sLoading = false;
END_LOADS

void LightPreset::StartAnim() {
    mManualFrame = 0;
    mLastManualFrame = -1;
    mManualFrameStart = 0;
    mManualFadeTime = 0;
    mStartBeat = TheTaskMgr.Beat();
    mLastKeyframe = 0;
    mLastBlend = -1.0f;
    static Message start_anim_msg("start_anim_msg");
    Handle(start_anim_msg, false);
    FOREACH (it, mSelectTriggers) {
        (*it)->Trigger();
    }
}

void LightPreset::SetFrame(float frame, float blend) { SetFrameEx(frame, blend, false); }

int LightPreset::GetCurrentKeyframe() const {
    if (mManual)
        return mManualFrame;
    else if (mKeyframes.empty())
        return -1;
    else {
        int i;
        int ret;
        float f;
        GetKey(GetFrame(), i, ret, f);
        return ret;
    }
}

bool LightPreset::PlatformOk() const {
    if (TheLoadMgr.EditMode() || !mPlatformOnly
        || TheLoadMgr.GetPlatform() == kPlatformNone) {
        return true;
    } else {
        Platform plat = TheLoadMgr.GetPlatform();
        if (TheLoadMgr.GetPlatform() == kPlatformPC) {
            plat = kPlatformXBox;
        }
        return plat == mPlatformOnly;
    }
}

int LightPreset::NextManualFrame(LightPreset::KeyframeCmd cmd) const {
    int frame;
    if (cmd == kPresetKeyframeFirst) {
        frame = 0;
    } else {
        frame = mManualFrame + (cmd == kPresetKeyframeNext ? 1 : -1);
    }
    if (mLooping) {
        return frame % mKeyframes.size();
    } else {
        return Max<int>(0, Min<int>(frame, mKeyframes.size() - 1));
    }
}

void LightPreset::AdvanceManual(LightPreset::KeyframeCmd cmd) {
    MILO_ASSERT(mManual, 0x2c0);
    if (cmd != kPresetKeyframeFirst || mManualFrame) {
        mManualFrameStart = GetFrame();
        mLastManualFrame = mManualFrame;
        mManualFrame = NextManualFrame(cmd);
    }
}

void LightPreset::FillLightPresetData(RndLight *light, LightPreset::EnvLightEntry &entry) {
    entry.mColor = light->GetColor();
    entry.unk0 = Hmx::Quat(light->WorldXfm().m);
    entry.mPosition = light->WorldXfm().v;
    entry.mRange = light->Range();
    entry.mLightType = light->GetType();
}

void LightPreset::RemoveLight(int idx) {
    for (uint i = 0; i != mKeyframes.size(); i++) {
        auto &entries = mKeyframes[i].mLightEntries;
        entries.erase(entries.begin() + idx);
    }
    mLightState.erase(mLightState.begin() + idx);
    mLights.erase(mLights.begin() + idx);
}

void LightPreset::RemoveSpotlightDrawer(int idx) {
    for (uint i = 0; i != mKeyframes.size(); i++) {
        auto &entries = mKeyframes[i].mSpotlightDrawerEntries;
        entries.erase(entries.begin() + idx);
    }
    mSpotlightDrawerState.erase(mSpotlightDrawerState.begin() + idx);
    mSpotlightDrawers.erase(mSpotlightDrawers.begin() + idx);
}

void LightPreset::ApplyState(const LightPreset::Keyframe &k) {
    mSpotlightState = k.mSpotlightEntries;
    mEnvironmentState = k.mEnvironmentEntries;
    mLightState = k.mLightEntries;
    mSpotlightDrawerState = k.mSpotlightDrawerEntries;
}

void LightPreset::RemoveSpotlight(int idx) {
    for (uint i = 0; i != mKeyframes.size(); i++) {
        Keyframe &cur = mKeyframes[i];
        cur.mSpotlightEntries.erase(cur.mSpotlightEntries.begin() + idx);
    }
    mSpotlightState.erase(mSpotlightState.begin() + idx);
    mSpotlights.erase(mSpotlights.begin() + idx);
}

void LightPreset::RemoveEnvironment(int idx) {
    for (uint i = 0; i != mKeyframes.size(); i++) {
        auto &entries = mKeyframes[i].mEnvironmentEntries;
        entries.erase(entries.begin() + idx);
    }
    mEnvironmentState.erase(mEnvironmentState.begin() + idx);
    mEnvironments.erase(mEnvironments.begin() + idx);
}

void LightPreset::AddLight(RndLight *lit) {
    mLights.push_back(lit);
    EnvLightEntry e;
    FillLightPresetData(lit, e);
    for (uint i = 0; i != mKeyframes.size(); i++) {
        mKeyframes[i].mLightEntries.push_back(e);
        MILO_ASSERT(mKeyframes[i].mLightEntries.size() == mLights.size(), 0x41a);
    }
    mLightState.push_back(e);
}

void LightPreset::Clear() {
    mKeyframes.clear();
    mSpotlights.clear();
    mEnvironments.clear();
    mSpotlightDrawers.clear();
    mLights.clear();
}

void LightPreset::OnKeyframeCmd(LightPreset::KeyframeCmd cmd) {
    sManualEvents.push_back(std::make_pair(cmd, TheTaskMgr.Beat() + 4.0f));
}

void LightPreset::AddEnvironment(RndEnviron *env) {
    mEnvironments.push_back(env);
    EnvironmentEntry e;
    FillEnvPresetData(env, e);
    for (int i = 0; i != mKeyframes.size(); i++) {
        mKeyframes[i].mEnvironmentEntries.push_back(e);
        MILO_ASSERT(mKeyframes[i].mEnvironmentEntries.size() == mEnvironments.size(), 0x40A);
    }
    mEnvironmentState.push_back(e);
}

void LightPreset::FillSpotlightDrawerPresetData(
    SpotlightDrawer *sd, LightPreset::SpotlightDrawerEntry &e
) {
    e.mBaseIntensity = sd->Params().mBaseIntensity;
    e.mSmokeIntensity = sd->Params().mSmokeIntensity;
    e.mLightInfluence = sd->Params().mLightingInfluence;
    e.mTotalIntensity = sd->Params().mIntensity;
}

void LightPreset::AddSpotlightDrawer(SpotlightDrawer *sd) {
    mSpotlightDrawers.push_back(sd);
    SpotlightDrawerEntry e;
    FillSpotlightDrawerPresetData(sd, e);
    for (int i = 0; i != mKeyframes.size(); i++) {
        mKeyframes[i].mSpotlightDrawerEntries.push_back(e);
        MILO_ASSERT(mKeyframes[i].mSpotlightDrawerEntries.size() == mSpotlightDrawers.size(), 0x42A);
    }
    mSpotlightDrawerState.push_back(e);
}

void LightPreset::AddSpotlight(Spotlight *s, bool b) {
    mSpotlights.push_back(s);
    SpotlightEntry e(this);
    FillSpotPresetData(s, e, -1);
    if (b) {
        e.mIntensity = 0;
        e.mColor = 0;
    }
    for (int i = 0; i != mKeyframes.size(); i++) {
        mKeyframes[i].mSpotlightEntries.push_back(e);
        MILO_ASSERT(mKeyframes[i].mSpotlightEntries.size() == mSpotlights.size(), 0x3FA);
    }
    mSpotlightState.push_back(e);
}

void LightPreset::SetSpotlight(Spotlight *s, int data) {
    uint idx;
    for (idx = 0; idx != mSpotlights.size(); idx++) {
        if (mSpotlights[idx] == s)
            break;
    }
    if (idx == mSpotlights.size())
        AddSpotlight(s, false);
    for (uint i = 0; i != mKeyframes.size(); i++) {
        FillSpotPresetData(s, mKeyframes[i].mSpotlightEntries[idx], data);
    }
}

void LightPreset::SetFrameEx(float frame, float blend, bool b) {
    START_AUTO_TIMER("light");
    RndAnimatable::SetFrame(frame, blend);
    if (frame == 0 && TheLoadMgr.EditMode()) {
        SyncNewSpotlights();
    }
    if (mKeyframes.empty()) {
        return;
    } else {
        Keyframe *kf7 = nullptr;
        float f74 = 1.0f;
        Keyframe *kf5;
        if (mManual) {
            kf5 = &mKeyframes[mManualFrame];
            while (!sManualEvents.empty() && sManualEvents.front().second <= mStartBeat) {
                sManualEvents.pop_front();
            }
            if (!sManualEvents.empty()) {
                float f1 = kf5->mFadeOutTime;
                float sec = sManualEvents.front().second;
                if (sec - f1 / 480.0f <= TheTaskMgr.Beat()) {
                    AdvanceManual(sManualEvents.front().first);
                    if (sec > TheTaskMgr.Beat()) {
                        mManualFadeTime = (sec - TheTaskMgr.Beat()) * 480.0f;
                    } else {
                        mManualFadeTime = 0;
                    }
                    sManualEvents.pop_front();
                    kf5 = &mKeyframes[mManualFrame];
                }
            }

            if (mLastManualFrame != -1) {
                kf7 = &mKeyframes[mLastManualFrame];
                if (mManualFadeTime > 0) {
                    f74 = Min((frame - mManualFrameStart) / mManualFadeTime, 1.0f);
                    f74 = Max(0.0f, f74);
                } else
                    f74 = 0;
            }

        } else {
            int i78, i7c;
            GetKey(frame, i78, i7c, f74);
            kf5 = &mKeyframes[i7c];
            if (i78 != -1) {
                kf7 = &mKeyframes[i78];
            }
        }

        bool b2 = false;
        Keyframe *last = mLastKeyframe;
        if (kf5 == last && mLastBlend == f74)
            b2 = true;
        if (!b2) {
            ApplyState(*kf5);
            if (kf7) {
                AnimateState(*kf7, *kf5, 1.0f - f74);
            }
            mLastKeyframe = kf5;
            mLastBlend = f74;
        }
        if (!b2 || !b) {
            Animate(blend);
        }
        if (kf5 != last) {
            FOREACH (it, mLastKeyframe->mTriggers) {
                (*it)->Trigger();
            }
        }
        static Message start("on_set_frame");
        Handle(start, false);
    }
}

void LightPreset::FillEnvPresetData(RndEnviron *env, LightPreset::EnvironmentEntry &e) {
    e.mAmbientColor = env->AmbientColor();
    e.mFogEnable = env->FogEnable();
    e.mFogStart = env->GetFogStart();
    e.mFogEnd = env->GetFogEnd();
    e.mFogColor = env->FogColor();
}

void LightPreset::SyncNewSpotlights() {
    for (ObjDirItr<Spotlight> it(Dir(), true); it != nullptr; ++it) {
        Spotlight *cur = it;
        if (mSpotlights.find(cur) == mSpotlights.end_const()) {
            AddSpotlight(cur, true);
        }
    }
}

DataNode LightPreset::OnViewKeyframe(DataArray *da) {
    ApplyState(mKeyframes[da->Int(2)]);
    Animate(1.0f);
    return 0;
}

DataNode LightPreset::OnSetKeyframe(DataArray *da) {
    if (mHue) {
        MILO_NOTIFY("Can't set keyframe with hue translation");
        return 0;
    } else {
        int idx = da->Int(2);
        SyncKeyframeTargets();
        SetKeyframe(mKeyframes[idx]);
        return OnViewKeyframe(da);
    }
}
