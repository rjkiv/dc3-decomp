#include "rndobj/TexRenderer.h"
#include "math/Vec.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "rndobj/Anim.h"
#include "rndobj/Draw.h"
#include "rndobj/Poll.h"
#include "rndobj/Rnd.h"
#include "rndobj/Utl.h"
#include "utl/FilePath.h"

float ComputeAngle(const Vector3 &v1, const Vector3 &v2, const Vector3 &v3) {
    Vector3 v30, v20;
    Subtract(v2, v1, v30);
    Subtract(v3, v1, v20);
    Normalize(v30, v30);
    Normalize(v20, v20);
    return acos(Clamp(-1.0f, 1.0f, Dot(v20, v30)));
}

RndTexRenderer::RndTexRenderer()
    : mImpostorHeight(0), mDirty(1), mForce(0), mDrawPreClear(1), mDrawWorldOnly(0),
      mDrawResponsible(1), mNoPoll(0), mOutputTexture(this), mDraw(this), mCamera(this),
      mEnviron(this), mFirstDraw(1), mPrimeDraw(0), mForceMips(0), mMirrorCam(this),
      mClearBuffer(0), mClearColor(0, 0, 0) {}

BEGIN_HANDLERS(RndTexRenderer)
    HANDLE_SUPERCLASS(RndAnimatable)
    HANDLE_SUPERCLASS(RndDrawable)
    HANDLE_SUPERCLASS(RndPollable)
    HANDLE_SUPERCLASS(Hmx::Object)
    HANDLE(get_render_textures, OnGetRenderTextures)
END_HANDLERS

BEGIN_PROPSYNCS(RndTexRenderer)
    SYNC_PROP_MODIFY(draw, mDraw, mDirty = true; mFirstDraw = true)
    SYNC_PROP_MODIFY(cam, mCamera, mDirty = true)
    SYNC_PROP_MODIFY(output_texture, mOutputTexture, InitTexture())
    SYNC_PROP_MODIFY(force, mForce, mDirty = true)
    SYNC_PROP_MODIFY(imposter_height, mImpostorHeight, mDirty = true)
    SYNC_PROP_MODIFY(draw_pre_clear, mDrawPreClear, UpdatePreClearState())
    SYNC_PROP(draw_world_only, mDrawWorldOnly)
    SYNC_PROP(draw_responsible, mDrawResponsible)
    SYNC_PROP(no_poll, mNoPoll)
    SYNC_PROP_MODIFY(prime_draw, mPrimeDraw, mDirty = true)
    SYNC_PROP_MODIFY(force_mips, mForceMips, InitTexture())
    SYNC_PROP_MODIFY(mirror_cam, mMirrorCam, mDirty = true)
    SYNC_PROP_MODIFY(environ, mEnviron, mDirty = true)
    SYNC_PROP(clear_buffer, mClearBuffer)
    SYNC_PROP(clear_color, mClearColor)
    SYNC_PROP(clear_alpha, mClearColor.alpha)
    SYNC_SUPERCLASS(RndAnimatable)
    SYNC_SUPERCLASS(RndDrawable)
    SYNC_SUPERCLASS(RndPollable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(RndTexRenderer)
    SAVE_REVS(13, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(RndAnimatable)
    SAVE_SUPERCLASS(RndDrawable)
    SAVE_SUPERCLASS(RndPollable)
    bs << mDraw;
    bs << mCamera;
    bs << mOutputTexture;
    bs << mForce;
    bs << mImpostorHeight;
    bs << mDrawResponsible;
    bs << mDrawPreClear;
    bs << mDrawWorldOnly;
    bs << mPrimeDraw;
    bs << mForceMips;
    bs << mMirrorCam;
    bs << mNoPoll;
    bs << mEnviron;
    bs << mClearBuffer;
    bs << mClearColor;
END_SAVES

BEGIN_COPYS(RndTexRenderer)
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(RndAnimatable)
    COPY_SUPERCLASS(RndDrawable)
    COPY_SUPERCLASS(RndPollable)
    CREATE_COPY(RndTexRenderer)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mDraw)
        COPY_MEMBER(mCamera)
        COPY_MEMBER(mOutputTexture)
        COPY_MEMBER(mForce)
        COPY_MEMBER(mDrawWorldOnly)
        COPY_MEMBER(mDrawResponsible)
        COPY_MEMBER(mImpostorHeight)
        COPY_MEMBER(mDrawPreClear)
        COPY_MEMBER(mPrimeDraw)
        COPY_MEMBER(mForceMips)
        COPY_MEMBER(mMirrorCam)
        COPY_MEMBER(mNoPoll)
        COPY_MEMBER(mEnviron)
        InitTexture();
        mDirty = true;
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(13, 0)

BEGIN_LOADS(RndTexRenderer)
    LOAD_REVS(bs)
    ASSERT_REVS(13, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    if (d.rev > 2) {
        LOAD_SUPERCLASS(RndAnimatable)
        LOAD_SUPERCLASS(RndDrawable)
        if (d.rev > 10) {
            LOAD_SUPERCLASS(RndPollable)
        }
    }
    if (d.rev < 1) {
        FilePath fp;
        d >> fp;
    } else {
        mDraw.Load(d, false, nullptr);
    }
    if (d.rev > 3) {
        d >> mCamera;
    } else {
        mCamera = nullptr;
    }
    d >> mOutputTexture;
    InitTexture();
    if (d.rev > 1) {
        d >> mForce;
        d >> mImpostorHeight;
    }
    if (d.rev > 4) {
        d >> mDrawResponsible;
    } else {
        mDrawResponsible = true;
    }
    if (d.rev > 5) {
        d >> mDrawPreClear;
    } else {
        mDrawPreClear = false;
    }
    if (d.rev > 6) {
        d >> mDrawWorldOnly;
    }
    if (d.rev > 7) {
        d >> mPrimeDraw;
    }
    if (d.rev > 8) {
        d >> mForceMips;
    }
    if (d.rev > 9) {
        d >> mMirrorCam;
    }
    if (d.rev > 10) {
        d >> mNoPoll;
    }
    if (d.rev > 11) {
        d >> mEnviron;
    }
    if (d.rev > 12) {
        d >> mClearBuffer;
        d >> mClearColor;
    }
    mDirty = true;
END_LOADS

void RndTexRenderer::DrawShowing() {
    if (!mDrawPreClear)
        DrawToTexture();
}

void RndTexRenderer::ListDrawChildren(std::list<RndDrawable *> &list) {
    if (mDraw != nullptr && mDrawResponsible) {
        list.insert(list.end(), mDraw);
    }
}

void RndTexRenderer::UpdatePreClearState() {
    TheRnd.PreClearDrawAddOrRemove(this, mDrawPreClear, 0);
    mDirty = 1;
}

void RndTexRenderer::SetFrame(float frame, float blend) {
    RndAnimatable *anim = dynamic_cast<RndAnimatable *>(mDraw.Ptr());
    if (anim != nullptr) {
        anim->SetFrame(frame, blend);
        mDirty = true;
    }
}

float RndTexRenderer::StartFrame(void) {
    RndAnimatable *anim = dynamic_cast<RndAnimatable *>(mDraw.Ptr());
    if (anim != nullptr) {
        return anim->StartFrame();
    } else
        return 0.0f;
}

float RndTexRenderer::EndFrame(void) {
    RndAnimatable *anim = dynamic_cast<RndAnimatable *>(mDraw.Ptr());
    if (anim != nullptr) {
        return anim->EndFrame();
    } else
        return 0.0f;
}

void RndTexRenderer::ListAnimChildren(std::list<RndAnimatable *> &list) const {
    RndAnimatable *anim = dynamic_cast<RndAnimatable *>(mDraw.Ptr());
    if (anim != nullptr) {
        list.insert(list.end(), anim);
    }
}
void RndTexRenderer::ListPollChildren(std::list<RndPollable *> &list) const {
    if (mDraw != nullptr && mNoPoll) {
        RndPollable *poll = dynamic_cast<RndPollable *>(mDraw.Ptr());
        if (poll != nullptr) {
            list.insert(list.end(), poll);
        }
    }
}

void RndTexRenderer::InitTexture(void) {
    if (mForceMips && mOutputTexture) {
        mOutputTexture->SetBitmap(
            mOutputTexture->Width(),
            mOutputTexture->Height(),
            mOutputTexture->Bpp(),
            mOutputTexture->GetType(),
            true,
            nullptr
        );
    }
    mDirty = true;
}

// void RndTexRenderer::DrawToTexture() {}

DataNode RndTexRenderer::OnGetRenderTextures(DataArray *) {
    return GetRenderTextures(Dir());
}
