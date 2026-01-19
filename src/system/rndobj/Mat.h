#pragma once
#include "math/Color.h"
#include "math/Mtx.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "rndobj/BaseMaterial.h"
#include "rndobj/MetaMaterial.h"
#include "rndobj/Tex.h"
#include "utl/BinStream.h"

struct bf {
    uint val;
};

struct MatShaderOptions {
    MatShaderOptions();
    union {
        struct {
            int itop : 24;
            int mHasAOCalc : 1;
            int mHasBones : 1;
            int i5 : 1;
            int i4 : 1;
            int i3 : 1;
            int i2 : 1;
            int i1 : 1;
            int i0 : 1;
        } shader_struct;
        u32 pack;

        // from bank 5
        uint value;
        bf shaderType;
        bf billboard;
        bf skinned;
        bf useAO;
    }; // 0x0
    bool mTempMat;

    // TODO: rename this once you have a better idea of what it does
    // i think this is some sort of enum/opcode
    void SetLast5(int mask) { pack = (pack & ~0x1f) | (mask & 0x1f); }

    void SetHasBones(bool bones) {
        shader_struct.mHasBones = 0;
        shader_struct.mHasBones = bones;
    }

    void SetHasAOCalc(bool calc) {
        shader_struct.mHasAOCalc = 0;
        shader_struct.mHasAOCalc = calc;
    }
};

class RndMat : public BaseMaterial {
public:
    enum PropDisplay {
        kPropDisplayHidden = 0,
        kPropDisplayReadOnly = 1
    };
    enum ColorModFlags {
        kColorModNone = 0,
        kColorModAlphaPack = 1,
        kColorModAlphaUnpackModulate = 2,
        kColorModModulate = 3,
        kColorModNum = 3
    };
    virtual ~RndMat();
    OBJ_CLASSNAME(Mat);
    OBJ_SET_TYPE(Mat);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);

    NEW_OBJ(RndMat);
    OBJ_MEM_OVERLOAD(69); // nice

    bool GetRefractEnabled(bool);
    float GetRefractStrength();
    RndTex *GetRefractNormalMap();
    void SetZMode(ZMode mode) {
        mZMode = mode;
        mDirty |= 2;
    }
    void SetTexWrap(TexWrap wrap) {
        mTexWrap = wrap;
        mDirty |= 2;
    }
    void SetBlend(Blend blend) {
        mBlend = blend;
        mDirty |= 2;
    }
    void SetTexGen(TexGen gen) {
        mTexGen = gen;
        mDirty |= 2;
    }
    void SetAlphaWrite(bool write) {
        mAlphaWrite = write;
        mDirty |= 2;
    }
    void SetAlphaCut(bool cut) {
        mAlphaCut = cut;
        mDirty |= 2;
    }
    void SetUseEnv(bool use_env) {
        mUseEnviron = use_env;
        mDirty |= 2;
    }
    void SetPreLit(bool lit) {
        mPrelit = lit;
        mDirty |= 2;
    }
    void SetAlphaThreshold(int thresh) { mAlphaThreshold = thresh; }
    void SetPerPixelLit(bool lit) {
        mPerPixelLit = lit;
        mDirty |= 2;
    }
    void SetPointLights(bool lit) { mPointLights = lit; }
    void SetColor(float r, float g, float b) {
        mColor.Set(r, g, b);
        mDirty |= 1;
    }
    void SetAlpha(float a) {
        mColor.alpha = a;
        mDirty |= 1;
    }
    void SetShaderOpts(const MatShaderOptions &opts) { mShaderOptions = opts; }
    void SetTexXfm(const Transform &xfm) {
        mTexXfm = xfm;
        mDirty |= 2;
    }
    void SetDiffuseTex(RndTex *tex) {
        mDiffuseTex = tex;
        mDirty |= 2;
    }
    void SetCull(Cull cull) {
        mCull = cull;
        mDirty |= 2;
    }
    bool Dirty() const { return mDirty; }

    void SetColorMod(const Hmx::Color &, int);
    void SetSpecularMap(RndTex *);
    void SetMetaMat(MetaMaterial *, bool);
    MetaMaterial *CreateMetaMaterial(bool);
    MetaMaterial *GetMetaMaterial() const { return mMetaMaterial; }

    static void Init();
    static void Terminate();
    static void ReloadMetaMaterials();
    static void UpdateAllMatPropertiesFromMetaMat(ObjectDir *);
    static void ReloadAndUpdateMat(ObjectDir *dir) {
        ReloadMetaMaterials();
        UpdateAllMatPropertiesFromMetaMat(dir);
    }

protected:
    RndMat();

    bool IsEditable(Symbol);
    MatPropEditAction GetMetaMatPropAction(Symbol);
    bool OnGetPropertyDisplay(PropDisplay, Symbol);
    void UpdatePropertiesFromMetaMat();
    void LoadOld(BinStreamRev &);

    DataNode OnGetMetaMaterials(const DataArray *);
    DataNode OnGetMetaMaterialsDir(const DataArray *);

    static ObjectDir *sMetaMaterials;
    static ObjectDir *LoadMetaMaterials();

    ObjPtr<MetaMaterial> mMetaMaterial; // 0x1f8
    int unk20c;
    std::vector<Hmx::Color> mColorMod; // 0x210
    MatShaderOptions mShaderOptions; // 0x21c
    bool mToggleDisplayAllProps; // 0x224
    bool unk225;
    bool unk226;
    int mDirty; // 0x228
};

RndMat *LookupOrCreateMat(const char *, ObjectDir *);
