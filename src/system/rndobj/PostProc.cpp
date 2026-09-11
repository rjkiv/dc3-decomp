#include "rndobj/PostProc.h"
#include "PostProc.h"
#include "Rnd.h"
#include "Utl.h"
#include "math/Color.h"
#include "math/Rand.h"
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

RndPostProc *RndPostProc::sCurrent;
float RndPostProc::sBloomLocFactor = 1;
DOFOverrideParams RndPostProc::sDOFOverride;

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
    if (fps != mFPS) {
        mFPS = fps;
        int round = Round(120.0f / mFPS);
        mSwitch = round >> 1;
        mOdd = round & 1;
        if (mCount >= mSwitch) {
            mCount = 0;
        }
    }
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
        if (mSwitch < 2) {
            return kProcessAll;
        } else {
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
            if (++mCount >= mSwitch) {
                mCount = 0;
                mSwitch += mOdd;
                mOdd = -mOdd;
            }
            return cmd;
        }
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
    bs << mTrailThreshold << mTrailDuration;
    bs << mEmulateFPS;
    bs << mPosterLevels;
    bs << mPosterMin;
    bs << mKaleidoscopeComplexity << mKaleidoscopeSize << mKaleidoscopeAngle;
    bs << mKaleidoscopeRadius << mKaleidoscopeFlipUVs;
    bs << mHallOfTimeRate << mHallOfTimeColor << mHallOfTimeMix;
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
    bs << mBloomStreak << mBloomStreakAttenuation << mBloomStreakAngle;
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

INIT_REVS(0x25, 2)

BEGIN_LOADS(RndPostProc)
    LOAD_REVS(bs)
    ASSERT_REVS(0x25, 2)
    if (d.rev == 0x10) {
        int dRev;
        d >> dRev;
        MILO_ASSERT(dRev == 3, 0x2A8);
        Sphere s;
        bool b70;
        int i5c;
        // stupid and dumb
        BinStream &bs2 = (d >> b70).stream;
        bs2 >> s >> i5c;
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

bool RndPostProc::DoHueConverge() const { return true; }

bool RndPostProc::ColorXfmEnabled() const {
    return mColorModulation != 1 || mColorXfm.mHue != 0 || mColorXfm.mSaturation != 0
        || mColorXfm.mLightness != 0 || mColorXfm.mContrast != 0
        || mColorXfm.mBrightness != 0 || mColorXfm.mLevelInLo.Pack() != 0
        || mColorXfm.mLevelOutLo.Pack() != 0 || mColorXfm.mLevelInHi.Pack() != 0xffffff
        || mColorXfm.mLevelOutHi.Pack() != 0xffffff;
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
        mBlendVec.x = mTrailThreshold;
        mBlendVec.y = mDeltaSecs / mTrailDuration;
        mBlendVec.z = 1.0f / 3.0f;
    }
}

void RndPostProc::UpdateColorModulation() {
    if (mFlickerTimeBounds.x > 0 && mFlickerTimeBounds.y > 0 && mFlickerModBounds.y > 0) {
        if (mFlickerSeconds.x >= mFlickerSeconds.y) {
            mFlickerSeconds.x = Max(mFlickerSeconds.x - mFlickerSeconds.y, 0.0f);
            mColorModulation = 1 - RandomFloat(mFlickerModBounds.x, mFlickerModBounds.y);
            mFlickerSeconds.y =
                Max(mFlickerSeconds.x,
                    RandomFloat(mFlickerTimeBounds.x, mFlickerTimeBounds.y));
        }
        mFlickerSeconds.x += mDeltaSecs;
    } else {
        mColorModulation = 1;
    }
}

void RndPostProc::LoadRev(BinStreamRev &d) {
    if (d.rev > 4) {
        if (d.rev > 0xA) {
            d >> mBloomColor;
            if (d.rev < 0x18) {
                int x;
                d >> x;
            }
            d >> mBloomIntensity;
            d >> mBloomThreshold;
        } else {
            Hmx::Color c;
            d >> c;
            float f10 = c.red;
            if (c.red > c.green) {
                f10 = c.green;
            }
            if (f10 > c.blue) {
                f10 = c.blue;
            }
            if (f10 < 4) {
                mBloomThreshold = c.alpha;
                c.red = (4 - c.red) / (4 - f10);
                c.green = (4 - c.green) / (4 - f10);
                c.blue = (4 - c.blue) / (4 - f10);
                c.alpha = 0;
                mBloomColor = c;
            } else {
                mBloomColor.Set(1, 1, 1, 0);
                mBloomThreshold = c.alpha;
            }
            int x;
            d >> x;
            d >> mBloomIntensity;
            mBloomIntensity = sqrtf(mBloomIntensity);
            int y;
            d >> y;
        }
    }
    if (d.rev > 5 && d.altRev < 1) {
        ObjPtr<RndTex> tex(this);
        d >> tex;
    }
    if (d.rev > 6) {
        if (d.rev < 0x12) {
            d >> mColorXfm.mColorXfm;
        } else {
            MILO_ASSERT_FMT(
                mColorXfm.Load(d.stream),
                "%s can't load new %s version",
                PathName(this),
                ClassName()
            );
        }
        d.stream >> mFlickerModBounds >> mFlickerTimeBounds;
        if (d.rev < 9) {
            mFlickerModBounds.x = 1 - mFlickerModBounds.x;
            mFlickerModBounds.y = 1 - mFlickerModBounds.y;
        }
        if (d.rev < 0x1D) {
            mFlickerModBounds.x = 0;
        }
        d.stream >> mNoiseBaseScale >> mNoiseTopScale >> mNoiseIntensity;
        if (d.rev > 0xC) {
            d >> mNoiseStationary;
        }
        if (d.rev > 8) {
            d >> mNoiseMap;
        }
        if (d.rev > 0x24) {
            d >> mNoiseMidtone;
        } else {
            mNoiseMidtone = false;
        }
        if (d.rev < 0x12) {
            d >> mColorXfm.mHue;
            d >> mColorXfm.mSaturation;
            d >> mColorXfm.mLightness;
            d >> mColorXfm.mContrast;
            d >> mColorXfm.mBrightness;
        }
    }
    if (d.rev > 7) {
        d >> mTrailThreshold;
        d >> mTrailDuration;
        d >> mEmulateFPS;
    }
    if (d.rev > 9) {
        if (d.rev < 0x12) {
            d.stream >> mColorXfm.mLevelInLo >> mColorXfm.mLevelInHi;
            d.stream >> mColorXfm.mLevelOutLo >> mColorXfm.mLevelOutHi;
        }
        d.stream >> mPosterLevels;
    }
    if (d.rev > 0xD) {
        d.stream >> mPosterMin;
    }
    if (d.rev > 0xB) {
        if (d.rev < 0x16) {
            float f8;
            d >> f8;
            if (f8 != 0) {
                mKaleidoscopeComplexity = 2;
            }
        } else {
            d >> mKaleidoscopeComplexity;
            d >> mKaleidoscopeSize;
            d >> mKaleidoscopeAngle;
            d >> mKaleidoscopeRadius;
            d >> mKaleidoscopeFlipUVs;
        }
    }

    if (d.rev > 0xE && d.rev < 0x1F) {
        int x;
        d >> x;
        if (d.rev < 0x11) {
            int y;
            d >> y;
            ObjPtr<RndDrawable> draw(this);
            d >> draw;
        }
    }
    if (d.rev > 0x12) {
        d.stream >> mHallOfTimeRate;
        d.stream >> mHallOfTimeColor >> mHallOfTimeMix;
        if (d.rev > 0x13 && d.rev < 0x20) {
            bool b;
            d >> b;
            mHallOfTimeType = b ? 1 : 0;
        } else if (d.rev > 0x1F) {
            d.stream >> mHallOfTimeType;
        }
    }
    if (d.rev > 0x14) {
        d.stream >> mMotionBlurBlend;
        if (d.rev > 0x1A) {
            d.stream >> mMotionBlurWeight;
            if (d.rev > 0x21) {
                d >> mMotionBlurVelocity;
            }
        }
    }
    if (d.rev > 0x16) {
        d >> mGradientMap;
        d.stream >> mGradientMapOpacity;
        d.stream >> mGradientMapIndex;
        d.stream >> mGradientMapStart;
        d.stream >> mGradientMapEnd;
    }
    if (d.rev < 0x18) {
        mBloomThreshold *= 4;
    }
    if (d.rev > 0x18) {
        d >> mRefractMap;
        d >> mRefractDist;
        d >> mRefractScale;
        d >> mRefractPanning;
        d >> mRefractAngle;
        if (d.rev > 0x1B) {
            d >> mRefractVelocity;
        }
    }
    if (d.rev > 0x19) {
        d >> mChromaticAberrationOffset;
        if (d.rev > 0x22) {
            d >> mChromaticSharpen;
        }
    }
    if (d.rev > 0x1D) {
        d.stream >> mVignetteColor >> mVignetteIntensity;
    }
    if (d.rev > 0x20) {
        d >> mBloomGlare;
    }
    if (d.rev > 0x23) {
        d >> mBloomStreak >> mBloomStreakAttenuation >> mBloomStreakAngle;
    }
    if (d.altRev > 1) {
        d >> mHueTarget >> mHueFocus >> mBlendAmount >> mBrightnessPower;
    }
}

void RndPostProc::Interp(const RndPostProc *p1, const RndPostProc *p2, float f3) {
    if ((p1 || p2) && !mForceCurrentInterp) {
        if (!p2) {
            p2 = p1;
        } else if (!p1) {
            p1 = p2;
        }
        const RndPostProc *p5 = f3 > 0 ? p2 : p1;
        mNoiseMidtone = p5->mNoiseMidtone;
        mNoiseStationary = p5->mNoiseStationary;
        mNoiseMap = p5->mNoiseMap.Ptr();
        mGradientMap = p5->mGradientMap.Ptr();
        mRefractMap = p5->mRefractMap.Ptr();
        mBloomGlare = p5->mBloomGlare;
        mMotionBlurVelocity = p5->mMotionBlurVelocity;
        mChromaticSharpen = p5->mChromaticSharpen;
        mBloomIntensity = ::Interp(
            p1->mBloomGlare && TheHiResScreen.IsActive() ? p1->mBloomIntensity / 3.0f
                                                         : p1->mBloomIntensity,
            p2->mBloomGlare && TheHiResScreen.IsActive() ? p2->mBloomIntensity / 3.0f
                                                         : p2->mBloomIntensity,
            f3
        );
        ::Interp(p1->mBloomColor, p2->mBloomColor, f3, mBloomColor);
        ::Interp(p1->mBlendVec, p2->mBlendVec, f3, mBlendVec);
        ::Interp(p1->mTrailDuration, p2->mTrailDuration, f3, mTrailDuration);
        ::Interp(p1->mTrailThreshold, p2->mTrailThreshold, f3, mTrailThreshold);
        float f7 = f3;
        if (p1 != p2 && p1->mNoiseMidtone != p2->mNoiseMidtone && p1->mNoiseIntensity != 0
            && p2->mNoiseIntensity != 0) {
            f7 = 1;
        }
        ::Interp(p1->mNoiseBaseScale, p2->mNoiseBaseScale, f7, mNoiseBaseScale);
        ::Interp(p1->mNoiseTopScale, p2->mNoiseTopScale, f7, mNoiseTopScale);
        ::Interp(p1->mNoiseIntensity, p2->mNoiseIntensity, f7, mNoiseIntensity);
        ::Interp(
            p1->mKaleidoscopeComplexity,
            p2->mKaleidoscopeComplexity,
            f3,
            mKaleidoscopeComplexity
        );
        ::Interp(p1->mKaleidoscopeSize, p2->mKaleidoscopeSize, f3, mKaleidoscopeSize);
        ::Interp(p1->mKaleidoscopeAngle, p2->mKaleidoscopeAngle, f3, mKaleidoscopeAngle);
        ::Interp(
            p1->mKaleidoscopeRadius, p2->mKaleidoscopeRadius, f3, mKaleidoscopeRadius
        );
        mKaleidoscopeFlipUVs =
            f3 < 1 ? p1->mKaleidoscopeFlipUVs : p2->mKaleidoscopeFlipUVs;
        ::Interp(p1->mEmulateFPS, p2->mEmulateFPS, f3, mEmulateFPS);
        ::Interp(p1->mPosterLevels, p2->mPosterLevels, f3, mPosterLevels);
        ::Interp(p1->mPosterMin, p2->mPosterMin, f3, mPosterMin);
        ::Interp(p1->mColorModulation, p2->mColorModulation, f3, mColorModulation);
        ::Interp(
            p1->mColorXfm.mBrightness, p2->mColorXfm.mBrightness, f3, mColorXfm.mBrightness
        );
        ::Interp(p1->mColorXfm.mHue, p2->mColorXfm.mHue, f3, mColorXfm.mHue);
        ::Interp(
            p1->mColorXfm.mSaturation, p2->mColorXfm.mSaturation, f3, mColorXfm.mSaturation
        );
        ::Interp(
            p1->mColorXfm.mLightness, p2->mColorXfm.mLightness, f3, mColorXfm.mLightness
        );
        ::Interp(
            p1->mColorXfm.mContrast, p2->mColorXfm.mContrast, f3, mColorXfm.mContrast
        );
        ::Interp(
            p1->mColorXfm.mLevelInLo, p2->mColorXfm.mLevelInLo, f3, mColorXfm.mLevelInLo
        );
        ::Interp(
            p1->mColorXfm.mLevelInHi, p2->mColorXfm.mLevelInHi, f3, mColorXfm.mLevelInHi
        );
        ::Interp(
            p1->mColorXfm.mLevelOutLo, p2->mColorXfm.mLevelOutLo, f3, mColorXfm.mLevelOutLo
        );
        ::Interp(
            p1->mColorXfm.mLevelOutHi, p2->mColorXfm.mLevelOutHi, f3, mColorXfm.mLevelOutHi
        );
        mColorXfm.AdjustColorXfm();
        ::Interp(
            p1->mGradientMapOpacity, p2->mGradientMapOpacity, f3, mGradientMapOpacity
        );
        ::Interp(p1->mGradientMapIndex, p2->mGradientMapIndex, f3, mGradientMapIndex);
        ::Interp(p1->mGradientMapStart, p2->mGradientMapStart, f3, mGradientMapStart);
        ::Interp(p1->mGradientMapEnd, p2->mGradientMapEnd, f3, mGradientMapEnd);
        ::Interp(p1->mRefractDist, p2->mRefractDist, f3, mRefractDist);
        ::Interp(p1->mRefractScale, p2->mRefractScale, f3, mRefractScale);
        ::Interp(p1->mRefractPanning, p2->mRefractPanning, f3, mRefractPanning);
        ::Interp(p1->mRefractVelocity, p2->mRefractVelocity, f3, mRefractVelocity);
        ::Interp(p1->mRefractAngle, p2->mRefractAngle, f3, mRefractAngle);
        ::Interp(p1->mMotionBlurBlend, p2->mMotionBlurBlend, f3, mMotionBlurBlend);
        ::Interp(p1->mMotionBlurWeight, p2->mMotionBlurWeight, f3, mMotionBlurWeight);
        ::Interp(
            p1->mChromaticAberrationOffset,
            p2->mChromaticAberrationOffset,
            f3,
            mChromaticAberrationOffset
        );
        ::Interp(p1->mVignetteColor, p2->mVignetteColor, f3, mVignetteColor);
        ::Interp(p1->mVignetteIntensity, p2->mVignetteIntensity, f3, mVignetteIntensity);
        ::Interp(p1->mHueTarget, p2->mHueTarget, f3, mHueTarget);
        ::Interp(p1->mHueFocus, p2->mHueFocus, f3, mHueFocus);
        ::Interp(p1->mBlendAmount, p2->mBlendAmount, f3, mBlendAmount);
        ::Interp(p1->mBrightnessPower, p2->mBrightnessPower, f3, mBrightnessPower);
        ::Interp(p1->mFlickerTimeBounds, p2->mFlickerTimeBounds, f3, mFlickerTimeBounds);
        ::Interp(p1->mFlickerModBounds, p2->mFlickerModBounds, f3, mFlickerModBounds);
        bool hasRate = p1->mHallOfTimeRate;
        if (hasRate) {
            mHallOfTimeType = p1->mHallOfTimeType;
            mHallOfTimeRate = p1->mHallOfTimeRate;
            mHallOfTimeColor = p1->mHallOfTimeColor;
            mHallOfTimeMix = p1->mHallOfTimeMix;
        } else {
            mHallOfTimeRate = 0;
        }
    }
}
