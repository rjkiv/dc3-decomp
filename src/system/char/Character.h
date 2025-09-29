#pragma once
#include "char/CharDriver.h"
#include "math/Sphere.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "rndobj/Dir.h"
#include "rndobj/Draw.h"
#include "rndobj/Env.h"
#include "rndobj/Trans.h"
#include "utl/MemMgr.h"

class Waypoint;
class CharEyes;
class CharInterest;
class CharacterTest;
class ShadowBone;

enum LODType {
    kLODPerFrame = -1,
    kLOD0 = 0,
    kLOD1 = 1,
    kLOD2 = 2,
    kNumLods = 3
};

class Character : public RndDir {
public:
    struct Lod {
        Lod(Hmx::Object *owner) : mScreenSize(0), mOpaque(owner), mTranslucent(owner) {}

        /** "when the unit sphere centered on the bounding sphere
            is smaller than this screen height fraction,
            it will draw the next lod". Ranges from 0 to 10000. */
        float mScreenSize; // 0x0
        /** "Opaque drawables to show at this LOD.
            Drawables not in any lod group will be drawn at every LOD" */
        DrawPtrVec mOpaque; // 0x4
        /** "Translucent drawables to show at this LOD.
            Drawables in it are guaranteed to be drawn last." */
        DrawPtrVec mTranslucent; // 0x20
    };
    Character();
    // Hmx::Object
    virtual ~Character();
    virtual bool Replace(ObjRef *, Hmx::Object *);
    OBJ_CLASSNAME(Character);
    OBJ_SET_TYPE(Character);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, CopyType);
    virtual void PreSave(BinStream &) { UnhookShadow(); }
    virtual void PreLoad(BinStream &);
    virtual void PostLoad(BinStream &);
    // RndDrawable
    virtual void UpdateSphere();
    virtual void DrawShowing();
    virtual void DrawShadow(const Transform &, float);
    virtual RndDrawable *CollideShowing(const Segment &, float &, Plane &);
    // RndPollable
    virtual void Poll();
    virtual void Enter();
    virtual void Exit();
    // ObjectDir
    virtual void SyncObjects();
    virtual void
    CollideListSubParts(const Segment &, std::list<RndDrawable::Collision> &);

    virtual void Teleport(Waypoint *);
    /** "Calculates a new bounding sphere" */
    virtual void CalcBoundingSphere();
    virtual float ComputeScreenSize(RndCam *);
    virtual void DrawOpaque();
    virtual void DrawTranslucent();
    virtual CharEyes *GetEyes();
    virtual bool ValidateInterest(CharInterest *, ObjectDir *) { return true; }
    virtual bool SetFocusInterest(CharInterest *, int);
    virtual void SetInterestFilterFlags(int);
    virtual void ClearInterestFilterFlags();

    OBJ_MEM_OVERLOAD(0x57)
    NEW_OBJ(Character)

    void SetSphereBase(RndTransformable *);
    bool SetFocusInterest(Symbol, int);
    void MergeDraws(const Character *);
    void FindInterestObjects(ObjectDir *);
    void EnableBlinks(bool, bool);
    void SetInterestObjects(const ObjPtrList<CharInterest> &, ObjectDir *);
    void SetSelfShadow(bool selfshadow) { mSelfShadow = selfshadow; }
    void SetLodType(LODType lod) { mForceLod = lod; }
    void ForceBlink();
    void SetTeleport(bool t) { unk298 = t; }
    CharDriver *Driver() const { return mDriver; }

    static void Init();
    static Character *Current() { return sCurrent; }

protected:
    virtual void AddedObject(Hmx::Object *);
    virtual void RemovingObject(Hmx::Object *);

    void UnhookShadow();

    static Character *sCurrent;

    ShadowBone *AddShadowBone(RndTransformable *);

    DataNode OnPlayClip(DataArray *);
    DataNode OnCopyBoundingSphere(DataArray *);
    DataNode OnGetCurrentInterests(DataArray *);

    ObjVector<Lod> mLods; // 0x1fc
    int mLastLod; // 0x20c
    /** "Forces LOD, kLODPerFrame is normal behavior of picking per frame,
        the others force the lod (0 is highest res lod, 2 is lowest res lod)" */
    LODType mForceLod; // 0x210
    /** "Group containing shadow geometry" */
    DrawPtrVec mShadow; // 0x214
    /** "translucent objects to show independent of lod.
        Drawables in it are guaranteed to be drawn last." */
    DrawPtrVec mTranslucent; // 0x230
    CharDriver *mDriver; // 0x24c
    /** "Whether this character should be self-shadowed." */
    bool mSelfShadow; // 0x250
    bool unk251; // 0x251
    bool unk252; // 0x252
    /** "Base for bounding sphere, such as bone_pelvis.mesh" */
    ObjOwnerPtr<RndTransformable> mSphereBase; // 0x254
    /** "bounding sphere for the character, fixed" */
    Sphere mBounding; // 0x268
    std::vector<ShadowBone *> mShadowBones; // 0x27c
    int unk288;
    /** "Test Character by animating it" */
    CharacterTest *mTest; // 0x28c
    /** "if true, is frozen in place, no polling happens" */
    bool mFrozen; // 0x290
    int unk294;
    bool unk298;
    /** "select an interest object here and select 'force_interest' below
        to force the character to look at it." */
    Symbol mInterestToForce; // 0x29c
    ObjPtr<RndEnviron> unk2a0;
    int unk2b4;
    /** "Props to show and hide for cut scenes" */
    DrawPtrVec mShowableProps; // 0x2b8
    bool mDebugDrawInterestObjects; // 0x2d4
};
