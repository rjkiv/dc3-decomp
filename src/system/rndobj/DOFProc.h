#pragma once
#include "obj/Data.h"
#include "obj/Object.h"

class RndCam;

class DOFProc : public Hmx::Object {
public:
    DOFProc();
    virtual ~DOFProc();
    OBJ_CLASSNAME(DOFProc);
    OBJ_SET_TYPE(DOFProc);
    virtual DataNode Handle(DataArray *, bool);
    virtual void Set(const RndCam *, float, float, float, float) {}
    virtual void UnSet() {}
    virtual bool Enabled() const { return 0; }
    virtual int Blur() { return 0; }
    virtual float FocalPlane() { return 1; }
    virtual float BlurDepth() { return 1; }
    virtual float MaxBlur() { return 0; }
    virtual float MinBlur() { return 0; }

    NEW_OBJ(DOFProc);

    static void Init();
    static void Terminate();
};

extern DOFProc *TheDOFProc;

class DOFOverrideParams {
public:
    DOFOverrideParams()
        : mDepthScale(1), mDepthOffset(0), mMinBlurScale(1), mMinBlurOffset(0),
          mMaxBlurScale(1), mMaxBlurOffset(0), mBlurWidthScale(1) {}
    void SetDepthScale(float f) { mDepthScale = f; }
    void SetDepthOffset(float f) { mDepthOffset = f; }
    void SetMinBlurScale(float f) { mMinBlurScale = f; }
    void SetMinBlurOffset(float f) { mMinBlurOffset = f; }
    void SetMaxBlurScale(float f) { mMaxBlurScale = f; }
    void SetMaxBlurOffset(float f) { mMaxBlurOffset = f; }
    void SetBlurWidthScale(float f) { mBlurWidthScale = f; }
    float GetDepthScale() const { return mDepthScale; }
    float GetDepthOffset() const { return mDepthOffset; }
    float GetMinBlurScale() const { return mMinBlurScale; }
    float GetMinBlurOffset() const { return mMinBlurOffset; }
    float GetMaxBlurScale() const { return mMaxBlurScale; }
    float GetMaxBlurOffset() const { return mMaxBlurOffset; }
    float GetBlurWidthScale() const { return mBlurWidthScale; }

protected:
    float mDepthScale;
    float mDepthOffset;
    float mMinBlurScale;
    float mMinBlurOffset;
    float mMaxBlurScale;
    float mMaxBlurOffset;
    float mBlurWidthScale;
};
