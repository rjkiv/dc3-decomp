#pragma once
#include "obj/Data.h"
#include "rndobj/Draw.h"
#include "rndobj/Tex.h"
#include "utl/MemMgr.h"

/** "Applies a shader to the specified texture." */
class TexProc : public RndDrawable {
public:
    enum TexProcShaderType {
        /** "Swirly, twirly distortion" */
        kShaderTwirl = 0,
        /** "Leaves color untouched but sets all alpha values to 1" */
        kShaderKillAlpha = 1
    };
    virtual ~TexProc() {}
    OBJ_CLASSNAME(TexProc);
    OBJ_SET_TYPE(TexProc);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    virtual void DrawShowing() {
        if (!mDrawPreClear)
            DrawToTexture();
    }
    virtual void DrawPreClear() { DrawToTexture(); }
    virtual void UpdatePreClearState();
    virtual void Poll();

    OBJ_MEM_OVERLOAD(0x1D);
    NEW_OBJ(TexProc);

    void SetRegisters();
    bool CheckParams(DataArray *, bool);
    void SetParams(DataArray *, DataArray *);
    DataNode OnSetParams(DataArray *);

    static void Init();
    static RndMat *SetUpWorkingMat();

protected:
    TexProc();

    void DrawToTexture();

    static RndCam *mCam;

    /** "texture to read from" */
    ObjPtr<RndTex> mInputTex; // 0x40
    /** "texture to write to" */
    ObjPtr<RndTex> mOutputTex; // 0x54
    /** "Shader to apply" */
    TexProcShaderType mShaderType; // 0x68
    bool mDrawPreClear; // 0x6c
    DataArray *unk70; // 0x70
    /** "Wave frequency (for twirl shader only!)" */
    float mFrequency; // 0x74
    float unk78; // 0x78
    /** "Wave amplitude (for twirl shader only!)" */
    float mAmplitude; // 0x7c
    /** "Wave amplitude (for twirl shader only!)" */
    float mAmplitudeBump; // 0x80
    /** "Wave phase (for twirl shader only!)" */
    float mPhase; // 0x84
    /** "Wave phase velocity (change) (for twirl shader only!)" */
    float mPhaseVel; // 0x88
};
