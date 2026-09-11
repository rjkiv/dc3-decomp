#pragma once
#include "math/Color.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "rndobj/DOFProc.h"
#include "rndobj/ColorXfm.h"
#include "rndobj/Tex.h"
#include "utl/BinStream.h"
#include "utl/MemMgr.h"

enum ProcessCmd {
    kProcessNone = 0,
    kProcessWorld = 1,
    kProcessPost = 2,
    kProcessChar = 4,
    kProcessPostChar = 6,
    kProcessAll = 7
};

class ProcCounter {
public:
    ProcCounter();
    void SetProcAndLock(bool);
    void SetEvenOddDisabled(bool);
    ProcessCmd ProcCommands();

    void SetCount(int cnt) { mCount = cnt; }
    bool ProcAndLock() const { return mProcAndLock; }
    bool EvenOddDisabled() const { return mEvenOddDisabled; }

private:
    unsigned int SetEmulateFPS(int);

    bool mProcAndLock; // 0x0
    int mCount; // 0x4
    int mSwitch; // 0x8
    int mOdd; // 0xc
    int mFPS; // 0x10
    bool mEvenOddDisabled; // 0x14
    bool mTriFrameRendering; // 0x15
};

class PostProcessor {
public:
    PostProcessor() {}
    virtual ~PostProcessor() {}
    virtual void BeginWorld() {}
    virtual void EndWorld() {}
    virtual void DoPost() {}
    virtual float Priority() { return 1; }
    virtual const char *GetProcType() = 0;
};

/** "A PostProc drives post-processing effects." */
class RndPostProc : public Hmx::Object, public PostProcessor {
public:
    virtual ~RndPostProc();
    OBJ_CLASSNAME(PostProc);
    OBJ_SET_TYPE(PostProc);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);

    virtual void Select();
    virtual void Unselect();
    virtual void SetPriority(float f) { mPriority = f; }
    virtual void QueueMotionBlurObject(class RndDrawable *) {}
    virtual void SetBloomColor() {}
    virtual void DoPost();
    virtual float Priority() { return mPriority; }
    virtual const char *GetProcType() { return "RndPostProc"; }

    OBJ_MEM_OVERLOAD(0x22);
    NEW_OBJ(RndPostProc)

    void Interp(const RndPostProc *, const RndPostProc *, float);
    void LoadRev(BinStreamRev &);
    bool BlendPrevious() const;
    float BloomIntensity() const;
    bool HallOfTime() const;
    bool DoChromaticAberration() const;
    bool DoVignette() const;
    bool DoMotionBlur() const;
    bool DoGradientMap() const;
    bool DoRefraction() const;
    bool ColorXfmEnabled() const;
    float EmulateFPS() const { return mEmulateFPS; }
    bool DoHueConverge() const;

    static RndPostProc *Current();
    static DOFOverrideParams &DOFOverrides() { return sDOFOverride; }
    static void ResetDofProc();
    static void Init();
    static void Reset();

protected:
    RndPostProc();

    virtual void OnSelect();
    virtual void OnUnselect();

    void UpdateTimeDelta();
    void UpdateColorModulation();
    void UpdateBlendPrevious();

    DataNode OnAllowedNormalMap(const DataArray *);

    static RndPostProc *sCurrent;
    static float sBloomLocFactor;
    static DOFOverrideParams sDOFOverride;

    float mPriority; // 0x30
    /** "Color tint for bloom effect" */
    Hmx::Color mBloomColor; // 0x34
    /** "Luminance intensity at which to bloom" */
    float mBloomThreshold; // 0x44
    /** "Bloom intensity" */
    float mBloomIntensity; // 0x48
    /** "Whether or not to use the glare effect" */
    bool mBloomGlare; // 0x4C
    /** "Whether or not to use directional light streaks" */
    bool mBloomStreak; // 0x4D
    /** "Attenuation (scattering amount) of light streak.
        0.9 to 0.95 is the sweet spot.". Ranges from -2 to 2. */
    float mBloomStreakAttenuation; // 0x50
    /** "Angle for light streak". Ranges from -360 to 360. */
    float mBloomStreakAngle; // 0x54
    bool mForceCurrentInterp; // 0x58
    /** "Hue: -180 to 180, 0.0 is neutral" */
    /** "Saturation: -100 to 100, 0.0 is neutral" */
    /** "Lightness: -100 to 100, 0.0 is neutral" */
    /** "Contrast: -100 to 100, 0.0 is neutral" */
    /** "Brightness: -100 to 100, 0.0 is neutral" */
    /** "Input low end" */
    /** "Input high end" */
    /** "Output low end" */
    /** "Output high end" */
    RndColorXfm mColorXfm; // 0x5c
    /** "Number of levels for posterization, 0 turns off". Ranges from 0 to 255. */
    float mPosterLevels; // 0xf0
    /** "Minimum intensity to posterize, 1.0 is posterize all". Ranges from 0 to 1. */
    float mPosterMin; // 0xf4

    /** "Kaleidoscope settings" */
    /** "Number of slices in kaleidoscope, 0 turns off, 2 for vertical mirror". Ranges
     * from 0 to 64. */
    float mKaleidoscopeComplexity; // 0xf8
    /** "Smaller size means more repeated areas, but each area is smaller". Ranges
     * from 1.0e-2 to 64. */
    float mKaleidoscopeSize; // 0xfc
    /** "Additional clockwise degrees of rotation around center.".
        Ranges from 0 to 360. */
    float mKaleidoscopeAngle; // 0x100
    /** "Additional distance from center". Ranges from -0.5 to 0.5. */
    float mKaleidoscopeRadius; // 0x104
    /** "Flip texture UV coords when reflect" */
    bool mKaleidoscopeFlipUVs; // 0x108
    /** "Min and max intensity range [0..1], 0.0 to disable" */
    Vector2 mFlickerModBounds; // 0x10c
    /** "Min and max number of seconds for a light to dark cycle" */
    Vector2 mFlickerTimeBounds; // 0x114
    Vector2 mFlickerSeconds; // 0x11c
    float mColorModulation; // 0x124
    /** "X and Y tiling of the noise map" */
    Vector2 mNoiseBaseScale; // 0x128
    float mNoiseTopScale; // 0x130
    /** "intensity of the noise, 0.0 to disable". Ranges from -10 to 10. */
    float mNoiseIntensity; // 0x134
    /** "keep the noise map static over the screen" */
    bool mNoiseStationary; // 0x138
    /** "Applies the noise using at mid-tones of the scene,
        using an Overlay blend mode." */
    bool mNoiseMidtone; // 0x139
    /** "Optional noise bitmap" */
    ObjPtr<RndTex> mNoiseMap; // 0x13c
    /** "Min pixel value to leave trails [0..1]". Ranges from 0 to 1. */
    float mTrailThreshold; // 0x150
    /** "Seconds for the trails to last" */
    float mTrailDuration; // 0x154
    Vector3 mBlendVec; // 0x158
    /** "Frame rate to emulate, e.g. 24 for film. 0 disables emulation.". Ranges from 0
     * to 60. */
    float mEmulateFPS; // 0x168
    float mLastRender; // 0x16c
    float mDeltaSecs; // 0x170

    /** "Video feedback effect" */
    /** "Should the effect be blended, or should it produce solid colors?". Options are
     * kHOTBlended, kHOTSolidRingsDepth, kHOTSolidRingsAlpha */
    int mHallOfTimeType; // 0x174
    /** "Speed of effect.  0 is off.  1 is regular speed.". Ranges from -10 to 10. */
    float mHallOfTimeRate; // 0x178
    /** "Seconds for the trails to last." */
    Hmx::Color mHallOfTimeColor; // 0x17c
    /** "Amount of color to blend. 0 is no color, 1 is solid color. Not applicable if
     * solid rings checked.". Ranges from 0 to 1. */
    float mHallOfTimeMix; // 0x18c

    /** "Motion blur settings" */
    /** "The weighting for individual color channels in the previous frame blend." */
    /** The color's alpha field: "The weighting for bright pixels in the previous frame
     * blend.". Ranges from 0 to 1. */
    Hmx::Color mMotionBlurWeight; // 0x190
    /** "The amount of the previous frame to blend into the current frame. This can be
     * used to efficiently simulate motion blur or other effects. Set to zero to
     * disable.". Ranges from 0 to 1. */
    float mMotionBlurBlend; // 0x1a0
    /** "Whether or not to use the velocity motion blur effect. Should be enabled
    almost
     * all the time." */
    bool mMotionBlurVelocity; // 0x1a4

    /** "Gradient map settings" */
    /** "Gradient map; this texture should be layed out horizontally such that the
    color
     * to use when the pixel is black is on the left and white is on the right." */
    ObjPtr<RndTex> mGradientMap; // 0x1a8
    /** "The opacity of the gradient map effect.". Ranges from 0 to 1. */
    float mGradientMapOpacity; // 0x1bc
    /** "This indexes veritically into the gradient map texture. This is useful for
     * storing multiple gradient map textures in a single texture, and to blend between
     * them.". Ranges from 0 to 1. */
    float mGradientMapIndex; // 0x1c0
    /** "The depth where the gradient map will begin to take effect.". Ranges from 0
    to 1.
     */
    float mGradientMapStart; // 0x1c4
    /** "The depth where the gradient map will no longer take effect.". Ranges from 0
     * to 1. */
    float mGradientMapEnd; // 0x1c8

    /** "Full-screen refraction settings" */
    /** "This is a normal map used to distort the screen." */
    ObjPtr<RndTex> mRefractMap; // 0x1cc
    /** "The distance to refract each pixel of the screen. This can also be negative to
     * reverse the direction. Set to zero to disable." */
    float mRefractDist; // 0x1e0
    /** "This scales the refraction texture before distorting the screen,
        in the X and Y directions." */
    Vector2 mRefractScale; // 0x1e4
    /** "The amount to offset the refraction texture, in the X and Y directions. This
        is a fixed amount to offset the refraction effect." */
    Vector2 mRefractPanning; // 0x1ec
    /** "The velocity to scroll the refraction texture, in the X and Y directions. The
     * value is specified in units per second, and will offset the refraction effect
     over time." */
    Vector2 mRefractVelocity; // 0x1f4
    /** "The angle to rotate the refraction texture, in degrees.".
        Ranges from 0 to 360. */
    float mRefractAngle; // 0x1fc
    /** "Chromatic sharpen will sharpen the image, while chromatic aberration is an
     * artifact where color channels are slightly shifted. This is useful to simulate
     old cameras, poor quality video, or underwater scenes." */
    /** "The size, in pixels, of the chromatic aberration or sharpen effect." */
    float mChromaticAberrationOffset; // 0x200
    /** "Whether to sharpen the chromatic image or apply the aberration effect." */
    bool mChromaticSharpen; // 0x204
    /** "Color tint for vignette effect" */
    Hmx::Color mVignetteColor; // 0x208
    /** "0 for no effect, 1 for normal, less than one for smaller effect, 2 is full
     * color". Ranges from 0 to 2. */
    float mVignetteIntensity; // 0x218

    float mHueTarget; // 0x21c
    float mHueFocus; // 0x220
    float mBlendAmount; // 0x224
    float mBrightnessPower; // 0x228
};
