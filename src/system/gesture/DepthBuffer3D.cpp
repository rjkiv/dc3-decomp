#include "gesture/DepthBuffer3D.h"
#include "gesture/BaseSkeleton.h"
#include "gesture/GestureMgr.h"
#include "hamobj/HamGameData.h"
#include "hamobj/RhythmDetector.h"
#include "math/Mtx.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rnddx9/Rnd.h"
#include "rndobj/Draw.h"
#include "rndobj/Rnd_NG.h"
#include "rndobj/Tex.h"
#include "rndobj/Trans.h"

LargeQuadRenderData DepthBuffer3D::mQuad;

namespace {
    void JointToVertexData(Vector3 &, const Skeleton &, SkeletonJoint, const Vector4 &);
    void VertexToWorld(Vector3 &, const Transform &, float, const Vector4 &);
    RndMat *SetUpWorkingMat();
}

DepthBuffer3D::DepthBuffer3D()
    : mDrawSheet(0), mDrawPlayer1(1), mDrawPlayer2(1), mDrawNonPlayers(1),
      mDebugLayout(0), mNobodyColor(0, 0, 0, 0), mPlayerPalette(this), unk12c(this),
      unk140(1), mPlayerPaletteOffset(0), mPlayerPaletteScale(1), mMinimalMat(this),
      mMesh(this), mStretchNearCamera(1), mOpacity(1), unk17c(0), unk180(0),
      unk184(0xfffffc19), unk188(1), unk18c(this), mTile(1.5, 1.5), mScaleVoxel(1),
      mScaleVoxelGap(1), mFishEyeX(0), mFishEyeY(0), unk1d8(this), unk1ec(this),
      unk20c(80, 4, 4), unk220(40, 4, 4), unk234(60, 3, 3), unk248(30, 3, 3),
      unk25c(2048, 204.8f, 204.8f), unk270(4096, 204.8f, 204.8f), mMaxZoom(1),
      mMaxDepthZoom(1), unk28c(0) {}

DepthBuffer3D::~DepthBuffer3D() {}

BEGIN_HANDLERS(DepthBuffer3D)
    HANDLE_SUPERCLASS(RndDrawable)
    HANDLE_SUPERCLASS(RndTransformable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(DepthBuffer3D)
    SYNC_PROP(nobody_color, mNobodyColor)
    SYNC_PROP(nobody_alpha, mNobodyColor.alpha)
    SYNC_PROP_SET(
        player_palette, mPlayerPalette.Ptr(), SetPlayerPalette(_val.Obj<RndTex>())
    )
    SYNC_PROP(player_palette_offset, mPlayerPaletteOffset)
    SYNC_PROP(player_palette_scale, mPlayerPaletteScale)
    SYNC_PROP(minimal_mat, mMinimalMat)
    SYNC_PROP(draw_sheet, mDrawSheet)
    SYNC_PROP(mesh, mMesh)
    SYNC_PROP(stretch_near_camera, mStretchNearCamera)
    SYNC_PROP(opacity, mOpacity)
    SYNC_PROP(draw_player_1, mDrawPlayer1)
    SYNC_PROP(draw_player_2, mDrawPlayer2)
    SYNC_PROP(draw_non_players, mDrawNonPlayers)
    SYNC_PROP(tile_x, mTile.x)
    SYNC_PROP(tile_y, mTile.y)
    SYNC_PROP(scale_voxel, mScaleVoxel)
    SYNC_PROP(scale_voxelgap, mScaleVoxelGap)
    SYNC_PROP(fisheye_x, mFishEyeX)
    SYNC_PROP(fisheye_y, mFishEyeY)
    SYNC_PROP(max_zoom, mMaxZoom)
    SYNC_PROP(max_depth_zoom, mMaxDepthZoom)
    SYNC_PROP(debug_layout, mDebugLayout)
    SYNC_SUPERCLASS(RndDrawable)
    SYNC_SUPERCLASS(RndTransformable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

void DepthBuffer3D::Init() {
    REGISTER_OBJ_FACTORY(DepthBuffer3D);
    TheNgRnd.CreateLargeQuad(0x140, 0xF0, mQuad);
}

void DepthBuffer3D::UpdateAttachment(
    DepthBuffer3DAttachment &attachment, const Vector4 &v1, const Vector4 &v2
) {
    int skelIdx = TheGestureMgr->GetSkeletonIndexByTrackingID(
        TheGameData->Player(attachment.player)->GetSkeletonTrackingID()
    );
    bool b5 = false;
    Vector3 newPos;
    if (skelIdx + 1 > 0) {
        Skeleton &skeleton = TheGestureMgr->GetSkeleton(skelIdx);
        Vector3 localPos = LocalXfm().v;
        Vector3 pos;
        JointToVertexData(pos, skeleton, (SkeletonJoint)attachment.unk8, v1);
        VertexToWorld(pos, LocalXfm(), mStretchNearCamera, v2);
        Add(pos, localPos, newPos);
        attachment.obj->SetTransConstraint(mConstraint, nullptr, false);
        Normalize(mWorldXfm.m, attachment.obj->DirtyLocalXfm().m);
        b5 = true;
    }
    if (!b5) {
        newPos.Set(100000, 100000, 100000);
    }
    attachment.obj->SetLocalPos(newPos);
}

void DepthBuffer3D::AddAttachment(const DepthBuffer3DAttachment &attachment) {
    MILO_ASSERT(attachment.obj, 0x390);
    bool found = false;
    FOREACH (it, unk200) {
        if (it->obj == attachment.obj) {
            found = true;
            break;
        }
    }
    if (!found) {
        unk200.resize(unk200.size() + 1);
        unk200.back() = attachment;
        unk200.back().obj->SetTransParent(this, false);
        unk200.back().obj->SetTransConstraint(mConstraint, nullptr, false);
    }
}

void DepthBuffer3D::SetPlayerPalette(RndTex *tex) {
    if (tex && mPlayerPalette != tex) {
        if (unk140 != 1) {
            MILO_WARN_ONCE("dropping boxyman palette animation %f\n", unk140);
        }
        unk140 = 0;
        if (unk12c) {
            unk12c = mPlayerPalette;
        }
        mPlayerPalette = tex;
    }
}

void DepthBuffer3D::SetGrooviness(float f1) {
    unk17c = f1;
    unk180 = f1;
    unk1d8 = nullptr;
    unk1ec = nullptr;
}

void DepthBuffer3D::SetGrooviness(RhythmDetector *r1, RhythmDetector *r2) {
    unk1d8 = r1;
    unk1ec = r2;
}

void DepthBuffer3D::ForceDrawSkeletonIndex(int i1, bool b2) {
    unk184 = i1;
    unk188 = b2;
}
