#include "rndobj/ScreenMask.h"
#include "math/Geo.h"
#include "obj/Object.h"
#include "rndobj/Cam.h"
#include "rndobj/Draw.h"
#include "rndobj/HiResScreen.h"
#include "rndobj/Rnd.h"
#include "rndobj/Tex.h"
#include "utl/BinStream.h"

RndScreenMask::RndScreenMask()
    : mMat(this), mColor(1, 1, 1, 1), mRect(0, 0, 1, 1), mUseCamRect(false) {}

BEGIN_HANDLERS(RndScreenMask)
    HANDLE_SUPERCLASS(RndDrawable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(RndScreenMask)
    SYNC_PROP(mat, mMat)
    SYNC_PROP(color, mColor)
    SYNC_PROP(alpha, mColor.alpha)
    SYNC_PROP(screen_rect, mRect)
    SYNC_PROP(use_cam_rect, mUseCamRect)
    SYNC_SUPERCLASS(RndDrawable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(RndScreenMask)
    SAVE_REVS(2, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(RndDrawable)
    bs << mMat << mColor << mRect << mUseCamRect;
END_SAVES

BEGIN_COPYS(RndScreenMask)
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(RndDrawable)
    CREATE_COPY(RndScreenMask)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mMat)
        COPY_MEMBER(mColor)
        COPY_MEMBER(mRect)
        COPY_MEMBER(mUseCamRect)
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(2, 0)

BEGIN_LOADS(RndScreenMask)
    LOAD_REVS(bs)
    ASSERT_REVS(2, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    LOAD_SUPERCLASS(RndDrawable)
    d >> mMat;
    d >> mColor;
    if (d.rev > 0) {
        d >> mRect;
    }
    if (d.rev > 1) {
        d >> mUseCamRect;
    }
END_LOADS

void RndScreenMask::DrawShowing() {
    if (TheRnd.DrawMode() == Rnd::kDrawNormal) {
        int h = TheRnd.Height();
        int w = TheRnd.Width();
        RndTex *targetTex = RndCam::Current()->TargetTex();
        if (targetTex) {
            h = targetTex->Height();
            w = targetTex->Width();
        }
        float fw = w;
        float fh = h;
        if (!mUseCamRect && targetTex) {
            if (RndCam::Current()->GetScreenRect() != Hmx::Rect(0, 0, 1, 1)) {
                MILO_NOTIFY_ONCE(
                    "%s: Overriding camera screen_rect not supported with render texture",
                    Name()
                );
            }
        }
        RndCam *cur = RndCam::Current();
        if (!mUseCamRect && !cur->TargetTex()) {
            TheRnd.GetDefaultCam()->Select();
            Hmx::Rect inv = TheHiResScreen.InvScreenRect();
            Hmx::Rect rndRect;
            rndRect.x = (mRect.x * inv.w + inv.x) * fw;
            rndRect.y = (mRect.y * inv.h + inv.y) * fh;
            rndRect.w = (mRect.w * inv.w) * fw;
            rndRect.h = (mRect.h * inv.h) * fh;
            TheRnd.DrawRect(rndRect, mColor, mMat, nullptr, nullptr);
            cur->Select();
        } else {
            Hmx::Rect inv = TheHiResScreen.InvScreenRect();
            Hmx::Rect rndRect;
            rndRect.x = (mRect.x * inv.w + inv.x) * fw;
            rndRect.y = (mRect.y * inv.h + inv.y) * fh;
            rndRect.w = (mRect.w * inv.w) * fw;
            rndRect.h = (mRect.h * inv.h) * fh;
            TheRnd.DrawRect(rndRect, mColor, mMat, nullptr, nullptr);
        }
    }
}
