#include "rndobj/VelocityBuffer.h"
#include "math/Mtx.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rndobj/BaseMaterial.h"
#include "rndobj/Cam.h"
#include "rndobj/Mat.h"
#include "rndobj/Tex.h"
#include "rndobj/Utl.h"

RndVelocityBuffer RndVelocityBuffer::sSingleton;

// why doesn't this symbol spawn? something with the volatile RndMesh
bool RndXfmCache::GetXfms(
    unsigned int *ui1,
    volatile RndMesh &mesh,
    unsigned int ui2,
    unsigned int ui3,
    const float *&floats
) const {
    return true;
}

RndVelocityBuffer::RndVelocityBuffer()
    : unk36be8(0), unk36c6c(0), mFrame(0), mVelocityTex(nullptr), mMat(nullptr),
      unk36c7c(nullptr) {
    memset(&unk8, 0, 0xa4);
}

void RndVelocityBuffer::CacheCameraSettings(RndCam *camera) {
    MILO_ASSERT(camera, 0x88);
    Transform tfa0;
    Hmx::Matrix4 me0;
    camera->GetViewProjectXfms(tfa0, me0);
    unk8 = tfa0 * me0;
    camera->GetDepthRangeValues(unk48);
    camera->GetCamFrustum(unk58, unk68);
    mCam = camera;
}

bool RndVelocityBuffer::AdvanceFrame(RndCam *cam) {
    unk36c80 = false;
    unk36c6c ^= 1;
    mFrame++;
    if (cam != unk36c7c) {
        unk36c7c = cam;
        mFrame = 0;
    }
    return mFrame >= 2;
}

void RndVelocityBuffer::AllocateData(
    unsigned int ui1, unsigned int ui2, unsigned int ui3
) {
    MILO_ASSERT(mVelocityTex == NULL, 0x45);
    mVelocityTex = Hmx::Object::New<RndTex>();
    mVelocityTex->SetBitmap(ui1 / 2, ui2 / 2, ui3, RndTex::kRendered, false, nullptr);
    MILO_ASSERT(mMat == NULL, 0x4A);
    mMat = Hmx::Object::New<RndMat>();
    mMat->SetPerPixelLit(false);
    mMat->SetBlend(BaseMaterial::kBlendSrc);
    mMat->SetZMode(kZModeDisable);
    CreateAndSetMetaMat(mMat);
}

void RndVelocityBuffer::FreeData() {
    RELEASE(mVelocityTex);
    RELEASE(mMat);
}

void RndVelocityBuffer::ResetFrame() { mFrame = 0; }
