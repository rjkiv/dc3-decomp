#include "rndobj/TexRenderer.h"
#include "obj/Object.h"
#include "rndobj/Anim.h"
#include "rndobj/Draw.h"
#include "rndobj/Poll.h"
#include "rndobj/Rnd.h"
#include "utl/FilePath.h"

void RndTexRenderer::UpdatePreClearState() {
    TheRnd.PreClearDrawAddOrRemove(this, mDrawPreClear, 0);
    unk_0x58 = 1;
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
    unk_0x58 = true;
}

float RndTexRenderer::StartFrame(void) {
    RndAnimatable *anim = dynamic_cast<RndAnimatable *>((RndDrawable *)mDrawable);
    if (anim != nullptr) {
        return anim->StartFrame();
    } else
        return 0.0f;
}

float RndTexRenderer::EndFrame(void) {
    RndAnimatable *anim = dynamic_cast<RndAnimatable *>((RndDrawable *)mDrawable);
    if (anim != nullptr) {
        return anim->EndFrame();
    } else
        return 0.0f;
}

void RndTexRenderer::SetFrame(float frame, float blend) {
    RndAnimatable *anim = dynamic_cast<RndAnimatable *>((RndDrawable *)mDrawable);
    if (anim != nullptr) {
        anim->SetFrame(frame, blend);
        unk_0x58 = true;
    }
}

void RndTexRenderer::Save(BinStream &bs) {
    bs << 13; // Major revision 13. No alternative revision.
    Hmx::Object::Save(bs);
    RndAnimatable::Save(bs);
    RndDrawable::Save(bs);
    RndPollable::Save(bs);
    bs << mDrawable;
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
}

BEGIN_HANDLERS(RndTexRenderer)
    HANDLE_SUPERCLASS(RndAnimatable)
    HANDLE_SUPERCLASS(RndDrawable)
    HANDLE_SUPERCLASS(RndPollable)
    HANDLE_SUPERCLASS(Hmx::Object)
    HANDLE_EXPR(get_render_textures, 3);
END_HANDLERS

BEGIN_COPYS(RndTexRenderer)
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(RndAnimatable)
    COPY_SUPERCLASS(RndDrawable)
    COPY_SUPERCLASS(RndPollable)
    CREATE_COPY(RndTexRenderer)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mDrawable)
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
        unk_0x58 = true;
    END_COPYING_MEMBERS
END_COPYS

BEGIN_PROPSYNCS(RndTexRenderer)
    SYNC_PROP_MODIFY(draw, mDrawable, unk_0x58 = true; unk_0xB5 = true)
    SYNC_PROP_MODIFY(cam, mCamera, unk_0x58 = true)
    SYNC_PROP_MODIFY(output_texture, mOutputTexture, InitTexture())
    SYNC_PROP(force, mForce)
    SYNC_PROP(imposter_height, mImpostorHeight)
    SYNC_PROP_MODIFY(draw_pre_clear, mDrawPreClear, UpdatePreClearState())
    SYNC_PROP(draw_world_only, mDrawWorldOnly)
    SYNC_PROP(draw_responsible, mDrawResponsible)
    SYNC_PROP(no_poll, mNoPoll)
    SYNC_PROP(prime_draw, mPrimeDraw)
    SYNC_PROP(force_mips, mForceMips)
    SYNC_PROP(mirror_cam, mMirrorCam)
    SYNC_PROP(environ, mEnviron)
    SYNC_PROP(clear_buffer, mClearBuffer)
    SYNC_PROP(clear_color, mClearColor)
    SYNC_PROP(clear_alpha, mClearColor.alpha)
    SYNC_SUPERCLASS(RndAnimatable)
    SYNC_SUPERCLASS(RndDrawable)
    SYNC_SUPERCLASS(RndPollable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

void RndTexRenderer::ListAnimChildren(std::list<RndAnimatable *> &list) const {
    RndAnimatable *anim = dynamic_cast<RndAnimatable *>((RndDrawable *)mDrawable);
    if (anim != nullptr) {
        list.insert(list.end(), anim);
    }
}
void RndTexRenderer::ListDrawChildren(std::list<RndDrawable *> &list) {
    if (mDrawable != nullptr && mDrawResponsible) {
        list.insert(list.end(), mDrawable);
    }
}
void RndTexRenderer::ListPollChildren(std::list<RndPollable *> &list) const {
    if (mDrawable != nullptr && mNoPoll) {
        RndPollable *poll = dynamic_cast<RndPollable *>((RndDrawable *)mDrawable);
        if (poll != nullptr) {
            list.insert(list.end(), poll);
        }
    }
}

BEGIN_LOADS(RndTexRenderer)
    LOAD_REVS(bs)
    ASSERT_REVS(13, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    if (gRev > 2) {
        LOAD_SUPERCLASS(RndAnimatable)
        LOAD_SUPERCLASS(RndDrawable)
        if (gRev > 10)
            LOAD_SUPERCLASS(RndPollable)
    }
    if (gRev < 1) {
        FilePath fp;
        bs >> fp;
    } else {
        bsrev >> mDrawable;
    }
    if (gRev > 3) {
        bsrev >> mCamera;
    } else {
        mCamera = 0;
    }
    bsrev >> mOutputTexture;
    InitTexture();
    if (gRev > 1) {
        bsrev >> mForce;
        bsrev >> mImpostorHeight;
    }
    if (gRev > 4) {
        bsrev >> mDrawResponsible;
    } else {
        mDrawResponsible = true;
    }
    if (gRev > 5) {
        bsrev >> mDrawPreClear;
    } else {
        mDrawPreClear = true;
    }
    if (gRev > 6) {
        bsrev >> mDrawWorldOnly;
    }
    if (gRev > 7) {
        bsrev >> mPrimeDraw;
    }
    if (gRev > 8) {
        bsrev >> mForce;
    }
    if (gRev > 9) {
        bsrev >> mMirrorCam;
    }
    if (gRev > 10) {
        bsrev >> mNoPoll;
    }
    if (gRev > 11) {
        bsrev >> mEnviron;
    }
    if (gRev > 12) {
        bsrev >> mClearBuffer;
        bsrev >> mClearColor;
    }
    unk_0x58 = true;
END_LOADS

void RndTexRenderer::DrawToTexture() {}

void RndTexRenderer::DrawShowing() {
    if (!mDrawPreClear)
        DrawToTexture();
}

RndTexRenderer::RndTexRenderer()
    : mImpostorHeight(0), unk_0x58(1), mForce(0), mDrawPreClear(1), mDrawWorldOnly(0),
      mDrawResponsible(1), mNoPoll(0), mOutputTexture(this), mDrawable(this),
      mCamera(this), mEnviron(this), unk_0xB5(1), mPrimeDraw(0), mForceMips(0),
      mMirrorCam(this), mClearBuffer(0), mClearColor(0, 0, 0) {}
