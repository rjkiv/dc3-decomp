#include "gesture/DepthBuffer3D.h"
#include "gesture/BaseSkeleton.h"
#include "gesture/GestureMgr.h"
#include "gesture/JointUtl.h"
#include "hamobj/HamGameData.h"
#include "hamobj/RhythmDetector.h"
#include "math/Mtx.h"
#include "math/Vec.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rnddx9/Rnd.h"
#include "rndobj/BaseMaterial.h"
#include "rndobj/Draw.h"
#include "rndobj/Rnd_NG.h"
#include "rndobj/ShaderMgr.h"
#include "rndobj/Tex.h"
#include "rndobj/Trans.h"
#include <list>

LargeQuadRenderData DepthBuffer3D::mQuad;

namespace {
    void JointToVertexData(Vector3 &, const Skeleton &, SkeletonJoint, const Vector4 &);
    void VertexToWorld(Vector3 &, const Transform &, float, const Vector4 &);

    RndMat *SetUpWorkingMat() {
        RndMat *work = TheShaderMgr.GetWork();
        work->SetBlend(BaseMaterial::kBlendSrc);
        work->SetZMode(kZModeDisable);
        work->SetTexWrap(kTexWrapClamp);
        return work;
    }
}

DepthBuffer3D::DepthBuffer3D()
    : mDrawSheet(0), mDrawPlayer1(1), mDrawPlayer2(1), mDrawNonPlayers(1),
      mDebugLayout(0), mNobodyCol(0, 0, 0, 0), mPlayerPalette(this),
      mLastPlayerPalette(this), mAnimPerc(1), mPlayerPaletteOffset(0),
      mPlayerPaletteScale(1), mMinimalMat(this), mMesh(this), mStretchNearCamera(1),
      mOpacity(1), mPlayer1Groove(0), mPlayer2Groove(0), mForceDrawSkeletonIndex(-999),
      mDrawOthersOnForce(1), mTexSource(this), mTiles(1.5, 1.5), mVoxelScale(1),
      mVoxelSeparation(1), mFisheyeX(0), mFisheyeY(0), mGroove1(this), mGroove2(this),
      mZoomWidth(80, 4, 4), mZoomXCenter(40, 4, 4), mZoomHeight(60, 3, 3),
      mZoomYCenter(30, 3, 3), mZoomZCenter(2048, 204.8f, 204.8f),
      mZoomDepth(4096, 204.8f, 204.8f), mMaxZoom(1), mMaxDepthZoom(1),
      mHasPrevSmoothData(0) {}

DepthBuffer3D::~DepthBuffer3D() {}

BEGIN_HANDLERS(DepthBuffer3D)
    HANDLE_SUPERCLASS(RndDrawable)
    HANDLE_SUPERCLASS(RndTransformable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(DepthBuffer3D)
    SYNC_PROP(nobody_color, mNobodyCol)
    SYNC_PROP(nobody_alpha, mNobodyCol.alpha)
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
    SYNC_PROP(tile_x, mTiles.x)
    SYNC_PROP(tile_y, mTiles.y)
    SYNC_PROP(scale_voxel, mVoxelScale)
    SYNC_PROP(scale_voxelgap, mVoxelSeparation)
    SYNC_PROP(fisheye_x, mFisheyeX)
    SYNC_PROP(fisheye_y, mFisheyeY)
    SYNC_PROP(max_zoom, mMaxZoom)
    SYNC_PROP(max_depth_zoom, mMaxDepthZoom)
    SYNC_PROP(debug_layout, mDebugLayout)
    SYNC_SUPERCLASS(RndDrawable)
    SYNC_SUPERCLASS(RndTransformable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(DepthBuffer3D)
    SAVE_REVS(11, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(RndDrawable)
    SAVE_SUPERCLASS(RndTransformable)
    bs << mNobodyCol << mPlayerPalette << mPlayerPaletteOffset;
    bs << mMinimalMat << mDrawSheet << mMesh;
    bs << mPlayerPaletteScale << mStretchNearCamera << mOpacity;
    bs << mDrawPlayer1 << mDrawPlayer2 << mDrawNonPlayers;
    bs << mTiles << mVoxelScale << mVoxelSeparation << mFisheyeX << mFisheyeY;
    bs << mMaxZoom << mDebugLayout << mMaxDepthZoom;
END_SAVES

BEGIN_COPYS(DepthBuffer3D)
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(RndDrawable)
    COPY_SUPERCLASS(RndTransformable)
    CREATE_COPY(DepthBuffer3D)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mNobodyCol)
        COPY_MEMBER(mPlayerPalette)
        COPY_MEMBER(mPlayerPaletteOffset)
        COPY_MEMBER(mPlayerPaletteScale)
        COPY_MEMBER(mMinimalMat)
        COPY_MEMBER(mDrawSheet)
        COPY_MEMBER(mMesh)
        COPY_MEMBER(mStretchNearCamera)
        COPY_MEMBER(mOpacity)
        COPY_MEMBER(mDrawPlayer1)
        COPY_MEMBER(mDrawPlayer2)
        COPY_MEMBER(mDrawNonPlayers)
        COPY_MEMBER(mTiles)
        COPY_MEMBER(mVoxelScale)
        COPY_MEMBER(mVoxelSeparation)
        COPY_MEMBER(mFisheyeX)
        COPY_MEMBER(mFisheyeY)
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(11, 0)

BEGIN_LOADS(DepthBuffer3D)
    LOAD_REVS(bs)
    ASSERT_REVS(11, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    LOAD_SUPERCLASS(RndDrawable)
    LOAD_SUPERCLASS(RndTransformable)
    if (d.rev > 1) {
        d >> mNobodyCol >> mPlayerPalette >> mPlayerPaletteOffset >> mMinimalMat;
        if (d.rev < 3) {
            int x;
            d >> x;
        }
    }
    if (d.rev > 2) {
        d >> mDrawSheet >> mMesh;
    }
    if (d.rev > 3) {
        d >> mPlayerPaletteScale >> mStretchNearCamera;
    }
    if (d.rev > 4) {
        d >> mOpacity;
    }
    if (d.rev > 5) {
        d >> mDrawPlayer1 >> mDrawPlayer2 >> mDrawNonPlayers;
    }
    if (d.rev > 6) {
        d >> mTiles >> mVoxelScale >> mVoxelSeparation;
    }
    if (d.rev > 7) {
        d >> mFisheyeX;
    }
    if (d.rev > 8) {
        d >> mFisheyeY;
    }
    if (d.rev > 9) {
        d >> mMaxZoom >> mDebugLayout;
    }
    if (d.rev > 10) {
        d >> mMaxDepthZoom;
    }
END_LOADS

void DepthBuffer3D::ListDrawChildren(std::list<RndDrawable *> &draws) {
    if (mMesh) {
        draws.push_back(mMesh);
    }
}

void DepthBuffer3D::Init() {
    REGISTER_OBJ_FACTORY(DepthBuffer3D);
    TheNgRnd.CreateLargeQuad(0x140, 0xF0, mQuad);
}

void DepthBuffer3D::UpdateAttachment(
    DepthBuffer3DAttachment &att,
    const Vector4 &texZoomParams,
    const Vector4 &depthZoomParams
) {
    int skelIdx = TheGestureMgr->GetSkeletonIndexByTrackingID(
        TheGameData->Player(att.player)->GetSkeletonTrackingID()
    );
    bool b5 = false;
    Vector3 newPos;
    if (skelIdx + 1 > 0) {
        Skeleton &skeleton = TheGestureMgr->GetSkeleton(skelIdx - 1);
        Vector3 localPos = LocalXfm().v;
        Vector3 pos;
        JointToVertexData(pos, skeleton, att.bone, texZoomParams);
        VertexToWorld(pos, LocalXfm(), mStretchNearCamera, depthZoomParams);
        Add(pos, localPos, newPos);
        att.obj->SetTransConstraint(TransConstraint(), nullptr, false);
        Normalize(LocalXfm().m, att.obj->DirtyLocalXfm().m);
        b5 = true;
    }
    if (!b5) {
        newPos.Set(100000, 100000, 100000);
    }
    att.obj->SetLocalPos(newPos);
}

DepthBuffer3DAttachment *DepthBuffer3D::GetAttachment(const RndTransformable *t) {
    FOREACH (it, mAttachments) {
        if (it->obj == t) {
            return &*it;
        }
    }
    return nullptr;
}

void DepthBuffer3D::AddAttachment(const DepthBuffer3DAttachment &attachment) {
    MILO_ASSERT(attachment.obj, 0x390);
    if (!GetAttachment(attachment.obj)) {
        mAttachments.resize(mAttachments.size() + 1);
        DepthBuffer3DAttachment &back = mAttachments[mAttachments.size() - 1];
        back = attachment;
        back.prev_parent = back.obj->TransParent();
        back.obj->SetTransParent(TransParent(), false);
        back.obj->SetTransConstraint(TransConstraint(), nullptr, false);
    }
}

void DepthBuffer3D::SetPlayerPalette(RndTex *tex) {
    if (tex && mPlayerPalette != tex) {
        if (mAnimPerc != 1) {
            MILO_WARN_ONCE("dropping boxyman palette animation %f\n", mAnimPerc);
        }
        mAnimPerc = 0;
        if (mLastPlayerPalette) {
            mLastPlayerPalette = mPlayerPalette;
        }
        mPlayerPalette = tex;
    }
}

void DepthBuffer3D::SetGrooviness(float f1) {
    mPlayer1Groove = f1;
    mPlayer2Groove = f1;
    mGroove1 = nullptr;
    mGroove2 = nullptr;
}

void DepthBuffer3D::SetGrooviness(RhythmDetector *r1, RhythmDetector *r2) {
    mGroove1 = r1;
    mGroove2 = r2;
}

void DepthBuffer3D::ForceDrawSkeletonIndex(int i1, bool b2) {
    mForceDrawSkeletonIndex = i1;
    mDrawOthersOnForce = b2;
}
