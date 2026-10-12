#pragma once
#include "gesture/BaseSkeleton.h"
#include "gesture/CameraInput.h"
#include "gesture/Skeleton.h"
#include "math/Color.h"
#include "math/Mtx.h"
#include "math/Vec.h"
#include "obj/Dir.h"
#include "obj/Object.h"
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
    NEW_OBJ(SkeletonViz)

    void Init();
    float PhysicalCamRotation() const;
    void SetUsePhysicalCam(bool use);
    void SetPhysicalCamRotation(float degrees);
    void Rotate(float delta_degress);
    void SetAxesCoordSys(SkeletonCoordSys cs);
    void Visualize(
        const CameraInput &cam_input,
        const BaseSkeleton &skeleton,
        std::vector<SkeletonCallback *> *callbacks,
        bool darken
    );
    void DrawLine3D(
        const Vector3 &p0,
        const Vector3 &p1,
        float size,
        const Hmx::Color &color,
        Hmx::Color *end_color
    );
    void SetPhysicalCamScreenRect(const Hmx::Rect &r);
    void
    DrawPoint3D(const Vector3 &p, float radius, const Hmx::Color &color, float alpha);

private:
    void LoadResource(bool async);
    void UpdateResource();
    void
    SetCamera(const SkeletonFrame &frame, const Transform &cam_origin, float hip_to_cam);
    void DrawJoints(
        const BaseSkeleton &skeleton,
        Vector3 *cam_joints,
        Vector3 *milo_joints,
        bool darken
    );

protected:
    SkeletonViz();

    /** "Draw skeleton from Natal camera perspective?" */
    bool mUsePhysicalCam; // 0x108
    /** "Degrees to rotation physical camera around skeleton".
        Ranges from -360 to 360. */
    float mPhysicalCamRotation; // 0x10c
    float mDesiredCamRotation; // 0x110
    /** "Which coordinate system axes to draw" */
    SkeletonCoordSys mAxesCoordSys; // 0x114
    RndLine *mLines[kNumBones]; // 0x118
    RndLine *mUtlLine; // 0x164
    ObjDirPtr<ObjectDir> mResource; // 0x168
    RndEnviron *mEnviron; // 0x17c
    RndMesh *mCamMesh; // 0x180
    RndMesh *mJointMesh; // 0x184
    RndMesh *mUtlSphere; // 0x188
    RndMat *mJointMat; // 0x18c
    RndCam *mPhysicalCam; // 0x190
    Transform mDefaultNatalToWorld; // 0x194
    Transform mNatalToWorld; // 0x1d4
    float mDrawScale; // 0x214
    bool mUseParentXfm; // 0x218
};

extern SkeletonViz *TheSkeletonViz;
