#pragma once
#include "gesture/BaseSkeleton.h"
#include "gesture/CameraInput.h"
#include "gesture/Skeleton.h"
#include "math/Mtx.h"
#include "obj/Dir.h"
#include "rndobj/Cam.h"
#include "rndobj/Draw.h"
#include "rndobj/Env.h"
#include "rndobj/Line.h"
#include "rndobj/Mat.h"
#include "rndobj/Mesh.h"
#include "rndobj/Poll.h"
#include "rndobj/Trans.h"
#include "utl/MemMgr.h"

/** "Visualization of one natural input skeleton" */
class SkeletonViz : public RndDrawable, public RndTransformable, public RndPollable {
public:
    // Hmx::Object
    virtual ~SkeletonViz();
    OBJ_CLASSNAME(SkeletonViz);
    OBJ_SET_TYPE(SkeletonViz);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    virtual void PreLoad(BinStream &);
    virtual void PostLoad(BinStream &);
    // RndHighlightable
    virtual void Highlight() { RndDrawable::Highlight(); }
    // RndPollable
    virtual void Poll();

    OBJ_MEM_OVERLOAD(0x19);

    void Init();
    float PhysicalCamRotation() const;
    void SetUsePhysicalCam(bool);
    void SetPhysicalCamRotation(float);
    void Rotate(float);
    void SetAxesCoordSys(SkeletonCoordSys);
    void Visualize(
        const CameraInput &, const BaseSkeleton &, std::vector<SkeletonCallback *> *, bool
    );

private:
    void LoadResource(bool);
    void UpdateResource();

protected:
    SkeletonViz();

    /** "Draw skeleton from Natal camera perspective?" */
    bool mUsePhysicalCam; // 0x108
    /** "Degrees to rotation physical camera around skeleton".
        Ranges from -360 to 360. */
    float mPhysicalCamRotation; // 0x10c
    float unk110; // 0x110
    /** "Which coordinate system axes to draw" */
    SkeletonCoordSys mAxesCoordSys; // 0x114
    RndLine *mBoneLines[kNumBones]; // 0x118
    RndLine *mUtlLine; // 0x164
    ObjDirPtr<ObjectDir> mResource; // 0x168
    RndEnviron *mSkeletonEnv; // 0x17c
    RndMesh *mCamMesh; // 0x180
    RndMesh *mJointMesh; // 0x184
    RndMesh *mSphereMesh; // 0x188
    RndMat *mJointMat; // 0x18c
    RndCam *mPhysicalCam; // 0x190
    Transform unk194; // 0x194
    Transform unk1d4; // 0x1d4
    float unk214; // 0x214
    bool unk218; // 0x218
};

extern SkeletonViz *TheSkeletonViz;
