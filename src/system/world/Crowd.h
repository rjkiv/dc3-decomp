#pragma once
#include "char/Character.h"
#include "math/Mtx.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "rndobj/Cam.h"
#include "rndobj/Draw.h"
#include "rndobj/Env.h"
#include "rndobj/Mesh.h"
#include "rndobj/MultiMesh.h"
#include "rndobj/Poll.h"
#include "utl/MemMgr.h"

/** "A quickly-rendered bunch of instanced characters within an area" */
class WorldCrowd : public RndDrawable, public RndPollable {
public:
    class CharDef {
    public:
        CharDef(Hmx::Object *owner)
            : mChar(owner), mHeight(75), mDensity(1), mRadius(10), mUseRandomColor(false),
              mMats(owner) {}

        /** "The character to use as the archetype" */
        ObjPtr<Character> mChar; // 0x0
        /** "The height at which to render the character" */
        float mHeight; // 0x14
        /** "Density to place this character" */
        float mDensity; // 0x18
        /** "Collision radius of the character -
            characters won't be placed within this range" */
        float mRadius; // 0x1c
        bool mUseRandomColor; // 0x20
        ObjPtrList<RndMat> mMats; // 0x24
    };

    struct CharData {
        struct Char3D {
            Transform unk0;
            int unk30;
            std::vector<Hmx::Color> unk34;
        };
        CharData(Hmx::Object *);

        CharDef mDef; // 0x0
        RndMultiMesh *mMultiMesh; // 0x38
        // this doesn't use StlNodeAlloc
        // it uses TransformListAlloc oh god
        std::list<RndMultiMesh::Instance> mBackup; // 0x3c
        std::vector<Char3D> m3DChars; // 0x44
        std::vector<Char3D> m3DCharsCreated; // 0x50
    };
    // Hmx::Object
    virtual ~WorldCrowd();
    OBJ_CLASSNAME(WorldCrowd);
    OBJ_SET_TYPE(WorldCrowd);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // RndDrawable
    virtual void UpdateSphere();
    virtual float GetDistanceToPlane(const Plane &, Vector3 &);
    virtual bool MakeWorldSphere(Sphere &, bool);
    virtual void Mats(std::list<class RndMat *> &, bool);
    virtual void DrawShowing();
    virtual void ListDrawChildren(std::list<RndDrawable *> &);
    virtual void CollideList(const Segment &, std::list<Collision> &);
    // RndPollable
    virtual void Poll();
    virtual void Enter();
    virtual void Exit();
    virtual void ListPollChildren(std::list<RndPollable *> &) const;

    OBJ_MEM_OVERLOAD(0x24)
    NEW_OBJ(WorldCrowd)

    void SetLod(int lod);
    void SetFullness(float, float);
    void Set3DCharList(const std::vector<std::pair<int, int> > &, Hmx::Object *);
    void Set3DCharXfm(const std::list<CharData>::iterator &, int, const Transform &);
    void Apply3DCharXfm(const std::list<CharData>::iterator &, int, RndCam *);

protected:
    WorldCrowd();

    void Force3DCrowd(bool);
    void SetMatAndCameraLod();
    void AssignRandomColors(bool);
    void Delete3DCrowdHandles();

    DataNode OnRebuild(DataArray *);
    DataNode OnIterateFrac(DataArray *);

    /** "The placement mesh" */
    ObjPtr<RndMesh> mPlacementMesh; // 0x48
    /** "Character archetypes for the crowd" */
    ObjList<CharData> mCharacters; // 0x5c
    /** "Number of characters to place" */
    int mNum; // 0x68
    int unk6c; // 0x6c
    Vector3 unk70; // 0x70
    /** "Makes crowd be 3D regardless of the CamShot" */
    bool mForce3DCrowd; // 0x80
    /** "Shows only the 3D crowd, but ONLY in Milo
        so you can more easily distinguish them from the 2d crowd" */
    bool mShow3DOnly; // 0x81
    float unk84; // 0x84
    float unk88; // 0x88
    int mLod; // 0x8c
    /** "The environ to render the imposter billboards with" */
    ObjPtr<RndEnviron> mEnviron; // 0x90
    /** "The environ used when rendering the 3D crowd set by a cam shot" */
    ObjPtr<RndEnviron> mEnviron3D; // 0xa4
    /** "Optional crowd facing focus when rotate is set to kCrowdRotateNone" */
    ObjPtr<RndTransformable> mFocus; // 0xb8
    /** "Force character Level of Detail.
        -1 means no LOD is forced." */
    LODType mCharForceLod; // 0xcc
    int unkd0; // 0xd0
    int unkd4; // 0xd4
};
