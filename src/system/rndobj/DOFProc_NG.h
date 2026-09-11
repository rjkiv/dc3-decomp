#pragma once
#include "obj/Object.h"
#include "rndobj/DOFProc.h"
#include "rndobj/PostProc.h"
#include "rndobj/Tex.h"

class NgDOFProc : public DOFProc, public PostProcessor {
public:
    NgDOFProc();
    virtual ~NgDOFProc();
    OBJ_CLASSNAME(DOFProc);
    OBJ_SET_TYPE(DOFProc);
    // DOFProc
    virtual bool Enabled() const { return mEnabled; }
    virtual float FocalPlane() { return mFocalPlane; }
    virtual float BlurDepth() { return mBlurDepth; }
    virtual float MaxBlur() { return mMaxBlur; }
    virtual float MinBlur() { return mMinBlur; }
    // PostProcessor
    virtual void DoPost();
    virtual const char *GetProcType() { return "NgDOFProc"; }

    static void Init();
    static void Terminate();
    NEW_OBJ(NgDOFProc)

protected:
    virtual void Set(const RndCam *, float, float, float, float);
    virtual void UnSet() { mEnabled = false; }

    bool mEnabled; // 0x30
    float unk34; // 0x34
    float unk38; // 0x38
    float mFocalPlane; // 0x3c
    float mBlurDepth; // 0x40
    float mMinBlur; // 0x44
    float mMaxBlur; // 0x48
    RndTex *mBlurTex[2]; // 0x4c
};
