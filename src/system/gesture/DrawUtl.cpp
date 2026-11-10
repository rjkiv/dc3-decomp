#include "DrawUtl.h"

#include "CameraInput.h"
#include "SkeletonViz.h"
#include "gesture/LiveCameraInput.h"
#include "gesture/SkeletonUpdate.h"
#include "math/Color.h"
#include "math/Geo.h"
#include "rndobj/Rnd.h"
#include "rndobj/Tex.h"
#include "rndobj/Utl.h"

struct ScreenSpaceValues {
    float left;
    float top;
    float width;
};
ScreenSpaceValues gScreenSpace = { 0.5f, 0.05f, 0.4f };

Hmx::Rect DrawUtlRect;

namespace {
    void ScreenSpace(Hmx::Rect &rect) {
        float left = gScreenSpace.left;
        float top = gScreenSpace.top;
        float width = gScreenSpace.width;
        rect.x = left;
        rect.y = top;
        rect.w = width;
        float scale = (float)TheRnd.Width() / (float)TheRnd.Height();
        if (scale > 1.3333334f) {
            rect.w = Min(width, 1.3333334f / scale);
        }
        rect.h = (scale * 0.75f) * rect.w;
    }

    void PixelSpace(Hmx::Rect &rect) {
        int width = TheRnd.Width();
        int height = TheRnd.Height();
        Hmx::Rect temp;
        ScreenSpace(temp);
        rect.x = width * temp.x;
        rect.y = height * temp.y;
        rect.w = width * temp.w;
        rect.h = height * temp.h;
    }
}

void TerminateDrawUtl() {
    if (TheSkeletonViz) {
        delete TheSkeletonViz;
    }
    TheSkeletonViz = nullptr;
}

bool ToggleDrawSkeletons() {
    MILO_ASSERT(TheSkeletonViz, 0xe2);
    TheSkeletonViz->SetShowing(!TheSkeletonViz->Showing());
    return TheSkeletonViz->Showing();
}

RndMat *CreateCameraBufferMat(int width, int height, RndTex::Type type) {
    auto tex = Hmx::Object::New<RndTex>();
    tex->SetBitmap(width, height, 16, type, false, nullptr);
    auto newMat = Hmx::Object::New<RndMat>();
    newMat->SetUseEnv(false);
    newMat->SetPreLit(true);
    newMat->SetBlend(BaseMaterial::kBlendSrc);
    newMat->SetZMode(BaseMaterial::kDisable);
    newMat->SetDiffuseTex(tex);
    CreateAndSetMetaMat(newMat);
    return newMat;
}

void DrawSnapshot(const GestureMgr &gm, int index) {
    MILO_ASSERT(index >= 0 && index < gm.GetLiveCameraInput()->NumSnapshots(), 0xfb);
    auto cam = gm.GetLiveCameraInput();
    auto snap = cam->GetSnapshot(index);
    DrawBufferMat(snap, DrawUtlRect);
}

void InitDrawUtl(const GestureMgr &gm) {
    TheSkeletonViz = Hmx::Object::New<SkeletonViz>();
    TheSkeletonViz->Init();
    TheSkeletonViz->SetUsePhysicalCam(true);
    TheSkeletonViz->SetAxesCoordSys(kCoordCamera);
    TheSkeletonViz->SetShowing(false);
    Hmx::Rect temp;
    PixelSpace(temp);
    DrawUtlRect = temp;
}

void SetDrawSpace(float x, float y, float z) {
    gScreenSpace.left = x;
    gScreenSpace.top = y;
    gScreenSpace.width = z;
}

bool UpdateBufferTex(
    LiveCameraInput *cam, RndTex *tex, LiveCameraInput::BufferType bufType, GestureMgr *gm
) {
    START_AUTO_TIMER("draw_natal_buffer");
    MILO_ASSERT(bufType < LiveCameraInput::kBufferNum, 0x12b);
    if (cam == nullptr) {
        return false;
    }
    MILO_ASSERT(tex, 0x130);
    MILO_ASSERT(tex->Bpp() == 16, 0x131);
    MILO_ASSERT(tex->GetType() == RndTex::kScratch, 0x132);
    // tex->TexelsLock(); seemingly pulls param out of nowehere
    if (bufType == LiveCameraInput::kBufferColor) {
        auto bufStream = cam->StreamBufferData(LiveCameraInput::kBufferColor);
        if (bufStream != nullptr) {
        }
    }
    return false;
}

void DrawBufferMat(RndMat *mat, Hmx::Rect &rect) {
    const Hmx::Color color(1, 1, 1, 1);
    TheRnd.DrawRect(rect, color, mat, nullptr, nullptr);
}

void DrawGestureMgr(GestureMgr &gm, LiveCameraInput::BufferType type, float f) {
    TheRnd.EndWorld();
    if (type != LiveCameraInput::kBufferNum && type != LiveCameraInput::kBufferPlayer
        && type != LiveCameraInput::kBufferDepth) {
        LiveCameraInput *input = gm.GetLiveCameraInput();
        if (UpdateBufferTex(input, input->DisplayTex(type), type, &gm)) {
            DrawBufferMat(input->DisplayMat(type), DrawUtlRect);
        }
    }

    if (TheSkeletonViz->Showing()) {
        Hmx::Rect screen;
        ScreenSpace(screen);
        TheSkeletonViz->SetPhysicalCamScreenRect(screen);

        SkeletonUpdateHandle handle = SkeletonUpdate::InstanceHandle();
        CameraInput *input = handle.GetCameraInput();
        for (int i = 0; i < 6; i++) {
            TheSkeletonViz->Visualize(
                *input, gm.GetSkeleton(i), &handle.Callbacks(), false
            );
        }
        gm.DrawSkeletonKinectData();
    }
}
