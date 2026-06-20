#pragma once
#include "math/Mtx.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "rndobj/Anim.h"
#include "rndobj/Draw.h"
#include "rndobj/Mesh.h"
#include "rndobj/MultiMesh.h"
#include "rndobj/Part.h"
#include "rndobj/Trans.h"
#include "rndobj/TransAnim.h"
#include "utl/MemMgr.h"

/** "A Generator object flies out object instances along a path." */
class RndGenerator : public RndAnimatable, public RndTransformable, public RndDrawable {
public:
    class Instance {
    public:
        float frameOrg; // 0x0
        Transform xfmMod; // 0x4
        Vector3 scale; // 0x44
    };
    // Hmx::Object
    virtual ~RndGenerator();
    OBJ_CLASSNAME(Mesh);
    OBJ_SET_TYPE(Mesh);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    virtual void Print();
    // RndAnimatable
    virtual void SetFrame(float, float);
    virtual float StartFrame();
    virtual float EndFrame();
    virtual void ListAnimChildren(std::list<RndAnimatable *> &) const;
    // RndDrawable
    virtual void UpdateSphere();
    virtual bool MakeWorldSphere(Sphere &, bool);
    virtual void DrawShowing();
    virtual void ListDrawChildren(std::list<RndDrawable *> &);
    // RndHighlightable
    virtual void Highlight() { RndDrawable::Highlight(); }

    OBJ_MEM_OVERLOAD(0x1F)
    NEW_OBJ(RndGenerator)
    static void Init() { REGISTER_OBJ_FACTORY(RndGenerator) }

    void Generate(float);
    void GetRateVar(float &lo, float &hi) {
        lo = mRateGenLow;
        hi = mRateGenHigh;
    }
    void SetRateVar(float lo, float hi) {
        mRateGenLow = lo;
        mRateGenHigh = hi;
    }
    void SetScaleVar(float lo, float hi) {
        mScaleGenLow = lo;
        mScaleGenHigh = hi;
    }
    void SetPathVar(float x, float y, float z) {
        mPathVarMaxX = x;
        mPathVarMaxY = y;
        mPathVarMaxZ = z;
    }
    void SetPath(RndTransAnim *, float, float);

protected:
    RndGenerator();

    void ResetInstances();
    void DrawMesh(Transform &, float);
    void DrawMultiMesh(Transform &, float);
    void DrawParticleSys(Transform &, float);

    DataNode OnSetPath(const DataArray *);
    DataNode OnSetRateVar(const DataArray *);
    DataNode OnSetScaleVar(const DataArray *);
    DataNode OnSetPathVar(const DataArray *);
    DataNode OnGenerate(const DataArray *);

    std::list<Instance> mInstances; // 0x110
    ObjPtr<RndTransAnim> mPath; // 0x118
    float mPathStartFrame; // 0x12c
    float mPathEndFrame; // 0x130
    ObjPtr<RndMesh> mMesh; // 0x134
    ObjPtr<RndMultiMesh> mMultiMesh; // 0x148
    ObjPtr<RndParticleSys> mParticleSys; // 0x15c
    float mNextFrameGen; // 0x170
    float mRateGenLow; // 0x174
    float mRateGenHigh; // 0x178
    float mScaleGenLow; // 0x17c
    float mScaleGenHigh; // 0x180
    float mPathVarMaxX; // 0x184
    float mPathVarMaxY; // 0x188
    float mPathVarMaxZ; // 0x18c
    RndParticle *mCurParticle; // 0x190
    std::list<RndMultiMesh::Instance>::iterator mCurMultiMesh; // 0x194
};
