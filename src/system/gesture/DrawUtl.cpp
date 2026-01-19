#include "DrawUtl.h"

#include "SkeletonViz.h"
#include "rndobj/Utl.h"

Vector3 DrawUtlVec3(0.5, 0.05, 0.4);

Hmx::Rect DrawUtlRect;

namespace {
    void ScreenSpace(Hmx::Rect &rect) {
        rect.x = DrawUtlVec3.x;
        rect.y = DrawUtlVec3.y;
        rect.h = DrawUtlVec3.z;
        float scale = TheRnd.Width() / TheRnd.Height();
        float rectW;
        if (1.3333334f < scale) {
            rectW = 1.3333334f / scale;
            if (DrawUtlVec3.z - 1.3333334f / scale < 0.0) {
                rectW = DrawUtlVec3.z;
            }
            rect.h = rectW;
        }
        rect.w = scale * 0.75f * rect.h;
    }

    void PixelSpace(Hmx::Rect &rect) {
        int width = TheRnd.Width();
        int height = TheRnd.Height();
        ScreenSpace(rect);
        rect.x = width * rect.x;
        rect.y = height * rect.y;
        rect.w = width * rect.w;
        rect.h = height * rect.h;
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
    newMat->SetZMode(kZModeDisable);
    newMat->SetDiffuseTex(tex);
    CreateAndSetMetaMat(newMat);
    return newMat;
}

void DrawSnapshot(const GestureMgr &gm, int index) {
    if (index > 0) {
        MILO_ASSERT(index >= 0 && index < gm.GetLiveCameraInput()->NumSnapshots(), 0xfb);
    }
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
    DrawUtlRect.Set(temp.x, temp.y, temp.w, temp.h);
}

void SetDrawSpace(float x, float y, float z) {
    DrawUtlVec3.Set(x, y, z);
}

bool UpdateBufferTex(LiveCameraInput *cam, RndTex *tex, LiveCameraInput::BufferType bufType, GestureMgr *gm) {
    START_AUTO_TIMER("draw_natal_buffer");
    MILO_ASSERT(bufType < LiveCameraInput::kBufferNum, 0x12b);
    if (cam == nullptr) {
        return false;
    }
    MILO_ASSERT(tex, 0x130);
    MILO_ASSERT(tex->Bpp() == 16, 0x131);
    MILO_ASSERT(tex->GetType() == RndTex::kScratch, 0x132);
    //tex->TexelsLock(); seemingly pulls param out of nowehere
    if (bufType == LiveCameraInput::kBufferColor) {
        auto bufStream = cam->StreamBufferData(LiveCameraInput::kBufferColor);
        if (bufStream != nullptr) {

        }
    }
    return false;
}