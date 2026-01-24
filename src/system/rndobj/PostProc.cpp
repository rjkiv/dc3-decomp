#include "rndobj/PostProc.h"
#include "PostProc.h"
#include "Rnd.h"
#include "Utl.h"
#include "math/Utl.h"
#include "obj/Data.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "os/Debug.h"
#include "os/System.h"
#include "rndobj/DOFProc.h"
#include "rndobj/HiResScreen.h"
#include "utl/BinStream.h"

void RndPostProc::ResetDofProc() { TheDOFProc->UnSet(); }
RndPostProc *RndPostProc::Current() { return sCurrent; }

ProcCounter::ProcCounter()
    : mProcAndLock(0), mCount(0), mSwitch(0), mOdd(0), mFPS(0), mEvenOddDisabled(0),
      mTriFrameRendering(0) {}

void ProcCounter::SetProcAndLock(bool pandl) {
    mProcAndLock = pandl;
    mCount = -1;
}

void ProcCounter::SetEvenOddDisabled(bool eod) {
    if (mEvenOddDisabled == eod)
        return;
    else
        mEvenOddDisabled = eod;
    if (mEvenOddDisabled)
        mCount = -1;
}

unsigned int ProcCounter::SetEmulateFPS(int fps) {
    if (fps <= 0) {
        if (mFPS != fps) {
            mFPS = 0;
            mSwitch = 0;
            mOdd = 0;
            mCount = 0;
        }
        return mFPS;
    }
    fps = Clamp(1, 60, fps);
    if (fps == mFPS) {
        return mFPS;
    }
    mFPS = fps;
    int round = Round(120.0f / mFPS);
    mSwitch = round >> 1;
    mOdd = round;
    if (mCount < mSwitch) {
        return mFPS;
    }
    mCount = 0;
    return mFPS;
}

ProcessCmd ProcCounter::ProcCommands() {
    if (mProcAndLock) {
        if (mCount >= 0) {
            return kProcessNone;
        } else {
            mCount = 0;
            return kProcessAll;
        }
    } else if (mEvenOddDisabled) {
        return kProcessAll;
    } else {
        SetEmulateFPS(
            RndPostProc::Current() ? Round(RndPostProc::Current()->EmulateFPS()) : 0
        );
        if (mSwitch >= 2) {
            ProcessCmd cmd = kProcessNone;
            switch (mCount) {
            case -1:
                mCount = 1;
                cmd = kProcessAll;
                break;
            case 0:
                cmd = kProcessWorld;
                break;
            case 1:
                cmd = kProcessPost;
                break;
            default:
                break;
            }
            mCount++;
            if (mCount >= mSwitch) {
                mCount = 0;
                mSwitch += mOdd;
                mOdd = -mOdd;
            }
            return cmd;
        }
        return kProcessAll;
    }
}

RndPostProc::RndPostProc()
    : mPriority(1), mBloomColor(1, 1, 1, 0), mBloomThreshold(4), mBloomIntensity(0),
      mBloomGlare(0), mBloomStreak(0), mBloomStreakAttenuation(0.9f),
      mBloomStreakAngle(0), mForceCurrentInterp(0), mColorXfm(), mPosterLevels(0),
      mPosterMin(1), mKaleidoscopeComplexity(0), mKaleidoscopeSize(0.5f),
      mKaleidoscopeAngle(0), mKaleidoscopeRadius(0), mKaleidoscopeFlipUVs(1),
      mFlickerModBounds(0, 0), mFlickerTimeBounds(0.001f, 0.007f), mFlickerSeconds(0, 0),
      mColorModulation(1), mNoiseBaseScale(32, 24), mNoiseTopScale(1.35914f),
      mNoiseIntensity(0), mNoiseStationary(0), mNoiseMidtone(1), mNoiseMap(this, 0),
      mTrailThreshold(1), mTrailDuration(0), mBlendVec(1, 1, 0), mEmulateFPS(30),
      mLastRender(0), mHallOfTimeType(0), mHallOfTimeRate(0),
      mHallOfTimeColor(1, 1, 1, 0), mHallOfTimeMix(0), mMotionBlurWeight(1, 1, 1, 0),
      mMotionBlurBlend(0), mMotionBlurVelocity(1), mGradientMap(this, 0),
      mGradientMapOpacity(0), mGradientMapIndex(0), mGradientMapStart(0),
      mGradientMapEnd(1), mRefractMap(this, 0), mRefractDist(0.05f), mRefractScale(1, 1),
      mRefractPanning(0, 0), mRefractVelocity(0, 0), mRefractAngle(0),
      mChromaticAberrationOffset(0), mChromaticSharpen(0), mVignetteColor(0, 0, 0, 0),
      mVignetteIntensity(0), mHueTarget(-75), mHueFocus(0.958), mBlendAmount(0),
      mBrightnessPower(1) {
    mColorXfm.Reset();
}

RndPostProc::~RndPostProc() {
    Unselect();
    if (TheRnd.GetPostProcOverride() == this) {
        TheRnd.SetPostProcOverride(nullptr);
    }
}

BEGIN_HANDLERS(RndPostProc)
    HANDLE_SUPERCLASS(Hmx::Object)
    HANDLE_ACTION(select, Select())
    HANDLE_ACTION(unselect, Unselect())
    HANDLE_ACTION(multi_select, OnSelect())
    HANDLE_ACTION(multi_unselect, OnUnselect())
    HANDLE_ACTION(
        interp,
        Interp(_msg->Obj<RndPostProc>(2), _msg->Obj<RndPostProc>(3), _msg->Float(4))
    )
    HANDLE(allowed_normal_map, OnAllowedNormalMap)
END_HANDLERS

BEGIN_PROPSYNCS(RndPostProc)
    SYNC_PROP(priority, mPriority)
    SYNC_PROP(bloom_color, mBloomColor)
    SYNC_PROP(bloom_threshold, mBloomThreshold)
    SYNC_PROP(bloom_intensity, mBloomIntensity)
    SYNC_PROP(bloom_glare, mBloomGlare)
    SYNC_PROP(bloom_streak, mBloomStreak)
    SYNC_PROP(bloom_streak_attenuation, mBloomStreakAttenuation)
    SYNC_PROP(bloom_streak_angle, mBloomStreakAngle)
    SYNC_PROP_MODIFY(hue, mColorXfm.mHue, mColorXfm.AdjustColorXfm())
    SYNC_PROP_MODIFY(saturation, mColorXfm.mSaturation, mColorXfm.AdjustColorXfm())
    SYNC_PROP_MODIFY(lightness, mColorXfm.mLightness, mColorXfm.AdjustColorXfm())
    SYNC_PROP_MODIFY(brightness, mColorXfm.mBrightness, mColorXfm.AdjustColorXfm())
    SYNC_PROP_MODIFY(contrast, mColorXfm.mContrast, mColorXfm.AdjustColorXfm())
    SYNC_PROP_MODIFY(in_lo, mColorXfm.mLevelInLo, mColorXfm.AdjustColorXfm())
    SYNC_PROP_MODIFY(in_hi, mColorXfm.mLevelInHi, mColorXfm.AdjustColorXfm())
    SYNC_PROP_MODIFY(out_lo, mColorXfm.mLevelOutLo, mColorXfm.AdjustColorXfm())
    SYNC_PROP_MODIFY(out_hi, mColorXfm.mLevelOutHi, mColorXfm.AdjustColorXfm())
    SYNC_PROP(num_levels, mPosterLevels)
    SYNC_PROP(min_intensity, mPosterMin)
    SYNC_PROP(kaleidoscope_complexity, mKaleidoscopeComplexity)
    SYNC_PROP(kaleidoscope_size, mKaleidoscopeSize)
    SYNC_PROP(kaleidoscope_angle, mKaleidoscopeAngle)
    SYNC_PROP(kaleidoscope_radius, mKaleidoscopeRadius)
    SYNC_PROP(kaleidoscope_flipUVs, mKaleidoscopeFlipUVs)
    SYNC_PROP(flicker_intensity, mFlickerModBounds)
    SYNC_PROP(flicker_secs_range, mFlickerTimeBounds)
    SYNC_PROP(noise_base_scale, mNoiseBaseScale)
    SYNC_PROP(noise_intensity, mNoiseIntensity)
    SYNC_PROP(noise_stationary, mNoiseStationary)
    SYNC_PROP(noise_midtone, mNoiseMidtone)
    SYNC_PROP(noise_map, mNoiseMap)
    SYNC_PROP(threshold, mTrailThreshold)
    SYNC_PROP(duration, mTrailDuration)
    SYNC_PROP(emulate_fps, mEmulateFPS)
    SYNC_PROP(hall_of_time_type, mHallOfTimeType)
    SYNC_PROP(hall_of_time_rate, mHallOfTimeRate)
    SYNC_PROP(hall_of_time_color, mHallOfTimeColor)
    SYNC_PROP(hall_of_time_mix, mHallOfTimeMix)
    SYNC_PROP(motion_blur_blend, mMotionBlurBlend)
    SYNC_PROP(motion_blur_weight, mMotionBlurWeight)
    SYNC_PROP(motion_blur_exposure, mMotionBlurWeight.alpha)
    SYNC_PROP(motion_blur_velocity, mMotionBlurVelocity)
    SYNC_PROP(gradient_map, mGradientMap)
    SYNC_PROP(gradient_map_opacity, mGradientMapOpacity)
    SYNC_PROP(gradient_map_index, mGradientMapIndex)
    SYNC_PROP(gradient_map_start, mGradientMapStart)
    SYNC_PROP(gradient_map_end, mGradientMapEnd)
    SYNC_PROP(refract_map, mRefractMap)
    SYNC_PROP(refract_dist, mRefractDist)
    SYNC_PROP(refract_scale, mRefractScale)
    SYNC_PROP(refract_panning, mRefractPanning)
    SYNC_PROP(refract_velocity, mRefractVelocity)
    SYNC_PROP(refract_angle, mRefractAngle)
    SYNC_PROP(chromatic_aberration_offset, mChromaticAberrationOffset)
    SYNC_PROP(chromatic_sharpen, mChromaticSharpen)
    SYNC_PROP(vignette_color, mVignetteColor)
    SYNC_PROP(vignette_intensity, mVignetteIntensity)
    SYNC_PROP(hue_target, mHueTarget)
    SYNC_PROP(hue_focus, mHueFocus)
    SYNC_PROP(blend_amount, mBlendAmount)
    SYNC_PROP(brightness_power, mBrightnessPower)
    SYNC_PROP(force_current_interp, mForceCurrentInterp)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(RndPostProc)
    SAVE_REVS(0x25, 2)
    SAVE_SUPERCLASS(Hmx::Object)
    bs << mBloomColor;
    bs << mBloomIntensity;
    bs << mBloomThreshold;
    mColorXfm.Save(bs);
    bs << mFlickerModBounds << mFlickerTimeBounds;
    bs << mNoiseBaseScale << mNoiseTopScale << mNoiseIntensity << mNoiseStationary;
    bs << mNoiseMap;
    bs << mNoiseMidtone;
    bs << mTrailThreshold;
    bs << mTrailDuration;
    bs << mEmulateFPS;
    bs << mPosterLevels;
    bs << mPosterMin;
    bs << mKaleidoscopeComplexity;
    bs << mKaleidoscopeSize;
    bs << mKaleidoscopeAngle;
    bs << mKaleidoscopeRadius;
    bs << mKaleidoscopeFlipUVs;
    bs << mHallOfTimeRate;
    bs << mHallOfTimeColor << mHallOfTimeMix;
    bs << mHallOfTimeType;
    bs << mMotionBlurBlend;
    bs << mMotionBlurWeight;
    bs << mMotionBlurVelocity;
    bs << mGradientMap;
    bs << mGradientMapOpacity;
    bs << mGradientMapIndex;
    bs << mGradientMapStart;
    bs << mGradientMapEnd;
    bs << mRefractMap;
    bs << mRefractDist;
    bs << mRefractScale;
    bs << mRefractPanning;
    bs << mRefractAngle;
    bs << mRefractVelocity;
    bs << mChromaticAberrationOffset;
    bs << mChromaticSharpen;
    bs << mVignetteColor;
    bs << mVignetteIntensity;
    bs << mBloomGlare;
    bs << mBloomStreak;
    bs << mBloomStreakAttenuation;
    bs << mBloomStreakAngle;
    bs << mHueTarget;
    bs << mHueFocus;
    bs << mBlendAmount;
    bs << mBrightnessPower;
END_SAVES

BEGIN_COPYS(RndPostProc)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(RndPostProc)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mPriority)
        COPY_MEMBER(mBloomIntensity)
        COPY_MEMBER(mBloomColor)
        COPY_MEMBER(mBloomThreshold)
        COPY_MEMBER(mBloomGlare)
        COPY_MEMBER(mBloomStreak)
        COPY_MEMBER(mBloomStreakAttenuation)
        COPY_MEMBER(mBloomStreakAngle)
        COPY_MEMBER(mColorXfm)
        COPY_MEMBER(mFlickerModBounds)
        COPY_MEMBER(mFlickerTimeBounds)
        COPY_MEMBER(mNoiseBaseScale)
        COPY_MEMBER(mNoiseTopScale)
        COPY_MEMBER(mNoiseIntensity)
        COPY_MEMBER(mNoiseStationary)
        COPY_MEMBER(mNoiseMap)
        COPY_MEMBER(mNoiseMidtone)
        COPY_MEMBER(mTrailDuration)
        COPY_MEMBER(mTrailThreshold)
        COPY_MEMBER(mEmulateFPS)
        COPY_MEMBER(mPosterLevels)
        COPY_MEMBER(mPosterMin)
        COPY_MEMBER(mKaleidoscopeComplexity)
        COPY_MEMBER(mKaleidoscopeSize)
        COPY_MEMBER(mKaleidoscopeAngle)
        COPY_MEMBER(mKaleidoscopeRadius)
        COPY_MEMBER(mKaleidoscopeFlipUVs)
        COPY_MEMBER(mHallOfTimeRate)
        COPY_MEMBER(mHallOfTimeColor)
        COPY_MEMBER(mHallOfTimeMix)
        COPY_MEMBER(mHallOfTimeType)
        COPY_MEMBER(mMotionBlurBlend)
        COPY_MEMBER(mMotionBlurWeight)
        COPY_MEMBER(mMotionBlurVelocity)
        COPY_MEMBER(mGradientMap)
        COPY_MEMBER(mGradientMapIndex)
        COPY_MEMBER(mGradientMapOpacity)
        COPY_MEMBER(mGradientMapStart)
        COPY_MEMBER(mGradientMapEnd)
        COPY_MEMBER(mRefractMap)
        COPY_MEMBER(mRefractDist)
        COPY_MEMBER(mRefractScale)
        COPY_MEMBER(mRefractPanning)
        COPY_MEMBER(mRefractVelocity)
        COPY_MEMBER(mRefractAngle)
        COPY_MEMBER(mChromaticAberrationOffset)
        COPY_MEMBER(mChromaticSharpen)
        COPY_MEMBER(mVignetteColor)
        COPY_MEMBER(mVignetteIntensity)
        COPY_MEMBER(mHueTarget)
        COPY_MEMBER(mHueFocus)
        COPY_MEMBER(mBlendAmount)
        COPY_MEMBER(mBrightnessPower)
    END_COPYING_MEMBERS
END_COPYS

BEGIN_LOADS(RndPostProc)
    LOAD_REVS(bs)
    ASSERT_REVS(0x25, 2)
    if (d.rev == 0x10) {
        int dRev;
        d >> dRev;
        MILO_ASSERT(dRev == 3, 0x2A8);
        float f30 = 0;
        bool b70;
        Vector3 v40;
        int i5c;
        d >> b70 >> v40 >> f30 >> i5c;
    } else {
        LOAD_SUPERCLASS(Hmx::Object)
    }
    LoadRev(d);
END_LOADS

void RndPostProc::Select() {
    if (sCurrent != this) {
        if (sCurrent) {
            sCurrent->OnUnselect();
        }
        sCurrent = this;
        sCurrent->OnSelect();
    }
}

void RndPostProc::Unselect() {
    if (sCurrent == this) {
        sCurrent->OnUnselect();
        sCurrent = nullptr;
    }
}

void RndPostProc::OnSelect() {
    TheRnd.RegisterPostProcessor(this);
    static Message msg("selected");
    Handle(msg, false);
}

void RndPostProc::OnUnselect() {
    TheRnd.UnregisterPostProcessor(this);
    static Message msg("unselected");
    Handle(msg, false);
}

void RndPostProc::DoPost() {
    UpdateTimeDelta();
    UpdateColorModulation();
    UpdateBlendPrevious();
}

void RndPostProc::Init() {
    sBloomLocFactor = SystemConfig("rnd", "bloom_loc")->FindFloat(SystemLanguage());
}

void RndPostProc::Reset() {
    if (sCurrent) {
        sCurrent->OnUnselect();
        sCurrent = nullptr;
    }
    TheDOFProc->UnSet();
}

DataNode RndPostProc::OnAllowedNormalMap(const DataArray *) {
    return GetNormalMapTextures(Dir());
}

bool RndPostProc::BlendPrevious() const {
    return mTrailThreshold < 1 && mTrailDuration > 0 && !TheHiResScreen.IsActive();
}

float RndPostProc::BloomIntensity() const {
    if (mBloomGlare && TheHiResScreen.IsActive()) {
        return mBloomIntensity / 3.0f;
    } else
        return mBloomIntensity;
}

bool RndPostProc::HallOfTime() const { return mHallOfTimeRate != 0; }
bool RndPostProc::DoChromaticAberration() const {
    return mChromaticAberrationOffset != 0;
}
bool RndPostProc::DoVignette() const { return mVignetteIntensity != 0; }

bool RndPostProc::DoMotionBlur() const {
    return mMotionBlurBlend > 0 && mMotionBlurWeight.Pack() > 0
        && !TheHiResScreen.IsActive();
}

bool RndPostProc::DoGradientMap() const {
    return mGradientMapOpacity > 0 && mGradientMap;
}

bool RndPostProc::DoRefraction() const { return mRefractMap && mRefractDist; }

bool RndPostProc::ColorXfmEnabled() const {
    return mColorModulation != 1 || mColorXfm.mHue != 0 || mColorXfm.mSaturation != 0
        || mColorXfm.mLightness != 0 || mColorXfm.mContrast != 0
        || mColorXfm.mBrightness != 0 || mColorXfm.mLevelInLo.Pack() != 0
        || mColorXfm.mLevelInHi.Pack() != 0 || mColorXfm.mLevelOutLo.Pack() != 0
        || mColorXfm.mLevelOutHi.Pack() != 0;
}

void RndPostProc::UpdateTimeDelta() {
    float secs = TheTaskMgr.Seconds(TaskMgr::kRealTime);
    float delta = secs - mLastRender;
    mLastRender = secs;
    mDeltaSecs = Clamp(0.0f, 1.0f, delta);
}

void RndPostProc::UpdateBlendPrevious() {
    if (BlendPrevious()) {
        MILO_ASSERT(mTrailDuration > 0.f, 0x100);
        mBlendVec.Set(mTrailThreshold, mDeltaSecs / mTrailDuration, 1.0f / 3.0f);
    }
}
