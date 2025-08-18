#pragma once
#include "math/Mtx.h"
#include "math/Vec.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "rndobj/Tex.h"
#include "rndobj/Trans.h"
#include "math/Geo.h"
#include "utl/MemMgr.h"

class RndCam : public RndTransformable {
public:
    virtual ~RndCam();
    OBJ_CLASSNAME(Cam);
    OBJ_SET_TYPE(Cam);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    virtual void UpdatedWorldXfm();
    virtual void Select(int);

    void SetViewProj(const Hmx::Matrix4 &);
    Transform GetInvViewXfm();
    float WorldToScreen(const Vector3 &, Vector2 &) const;
    void ScreenToWorld(const Vector2 &, float, Vector3 &) const;
    void GetCamFrustum(Vector3 &, Vector3 (&)[4]);
    void SetTargetTex(RndTex *);
    void SetFrustum(float, float, float, float);

    NEW_OBJ(RndCam);
    OBJ_MEM_OVERLOAD(0x1B);

protected:
    RndCam();

    void UpdateLocal();

    DataNode OnSetFrustum(const DataArray *);
    DataNode OnSetZRange(const DataArray *);
    DataNode OnSetScreenRect(const DataArray *);
    DataNode OnFarPlane(const DataArray *);
    DataNode OnWorldToScreen(const DataArray *);
    DataNode OnScreenToWorld(const DataArray *);

    static DataNode OnGetDefaultNearPlane(DataArray *);
    static DataNode OnGetMaxFarNearPlaneRatio(DataArray *);

    static RndCam *sCurrent;
    static float sDefaultNearPlane;
    static float sMaxFarNearPlaneRatio;

    Transform mInvWorldXfm; // 0xc0
    Transform mLocalProjectXfm; // 0x100
    Transform mInvLocalProjectXfm; // 0x140
    Transform mWorldProjectXfm; // 0x180
    Transform mInvWorldProjectXfm; // 0x1c0
    Frustum mLocalFrustum; // 0x200
    Frustum mWorldFrustum; // 0x260
    /**
     * @brief The near-clipping plane.
     * Original _objects description:
     * "The distance in world coordinates to the near clipping
     * plane. The near/far ratio is limited to 1:1000 to preserve
     * Z-buffer resolution."
     */
    float mNearPlane; // 0x2c0
    /**
     * @brief The far-clipping plane.
     * Original _object description:
     * "The distance in world coordinates to the far clipping
     * plane. The near/far ratio is limited to 1:1000 to preserve
     * Z-buffer resolution. Note that on the PS2, object polys are
     * culled rather than clipped to the far plane."
     */
    float mFarPlane; // 0x2c4
    float mYFov; // 0x2c8
    float unk2cc; // 0x2cc
    /**
     * @brief
     * Original _objects description:
     * "The part of the Z-buffer to use, in normalized
     * coordinates. It can be useful to draw a scene where the near
     * and far planes must exceed the 1:1000 ratio (so multiple
     * cameras are used to draw farthest to nearest objects, each
     * using a closer range of the z-buffer) or to leave some
     * z-buffer for HUD overlay objects."
     */
    Vector2 mZRange; // 0x2d0
    Hmx::Rect mScreenRect; // 0x2d8
    ObjPtr<RndTex> mTargetTex; // 0x2e8
    int unk2fc;
    Hmx::Matrix4 unk300;
    Hmx::Matrix4 unk340;
};
