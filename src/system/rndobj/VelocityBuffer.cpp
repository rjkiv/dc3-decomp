#include "rndobj/VelocityBuffer.h"
#include "math/Mtx.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rndobj/BaseMaterial.h"
#include "rndobj/Cam.h"
#include "rndobj/Mat.h"
#include "rndobj/Rnd.h"
#include "rndobj/Shader.h"
#include "rndobj/ShaderMgr.h"
#include "rndobj/Stats_NG.h"
#include "rndobj/Tex.h"
#include "rndobj/Utl.h"

RndVelocityBuffer RndVelocityBuffer::sSingleton;
static const int sMaxBones = 40;

bool RndXfmCache::GetXfms(
    const RndMesh *__restrict mesh,
    unsigned int ui2,
    unsigned int ui3,
    const float *&floats
) const {
    if (ui2 + ui3 <= unk1b580) {
        const RndMesh *ui2_mesh = unk0[ui2];
        const RndMesh *__restrict what = unk0[ui2 + ui3 - 1];
        if (ui2_mesh == mesh && what == mesh) {
            floats = unk1f40[ui2];
            return true;
        }
    }
    floats = nullptr;
    return false;
}

bool RndXfmCache::CacheXfms(
    const RndMesh *__restrict mesh,
    const float *__restrict floats,
    unsigned int ui3,
    unsigned int &uiref
) {
    uiref = -1;
    if (unk1b580 + ui3 <= 2000) {
        uiref = unk1b580;
        float *which = unk1f40[unk1b580];
        float diff = floats[0] - which[0];
        for (int i = 0; i < ui3; i++) {
            which[i] += diff;
        }
        if (ui3) {
            int *unkPtr = &unk19640[unk1b580 - 1];
            RndMesh **meshPtr = &unk0[unk1b580 - 1];
            for (int i = 0; i < ui3; i++) {
                meshPtr[i] = (RndMesh *)mesh;
            }
            for (int i = 0; i < ui3; i++) {
                unkPtr[i] = 0;
            }
        }
        unk1b580 += ui3;
    }
    return uiref < 2000;
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
    mMat->SetZMode(BaseMaterial::kDisable);
    CreateAndSetMetaMat(mMat);
}

void RndVelocityBuffer::DrawMesh(RndMesh *mesh) const {
    MILO_ASSERT(mesh, 0x123);
    MILO_ASSERT(mesh->Showing(), 0x124);
    MILO_ASSERT(TheRnd.DrawMode() == Rnd::kDrawVelocity, 0x125);
    if (mesh->Mat() && mesh->Mat()->GetZMode() != BaseMaterial::kTransparent) {
        int numBones = mesh->NumBones();
        int i10 = Max(1, numBones);
        const float *fptr;
        if (mXfmCaches[unk36c6c].GetXfms(mesh, 0, i10, fptr)) {
            if (numBones <= 40) {
                TheShaderMgr.SetMeshInfo(numBones, false);
                RndShader::SelectConfig(mMat, kVelocityObjectShader, false);
                mesh->DrawFacesInRange(0, -1);
                TheNgStats->mMotionBlurs++;
            } else {
                MILO_NOTIFY_ONCE(
                    "%s (%s): Has too many bones to apply object motion blur (%d bones of max %d)",
                    mesh->Name(),
                    PathName(mesh),
                    numBones,
                    sMaxBones
                );
            }
        }
    }
}

void RndVelocityBuffer::FreeData() {
    RELEASE(mVelocityTex);
    RELEASE(mMat);
}

void RndVelocityBuffer::ResetFrame() { mFrame = 0; }

void RndVelocityBuffer::CacheTransform(
    class RndMesh *__restrict mesh, const float *__restrict floats, unsigned int ui3
) {
    if ((TheRnd.ProcCmds() & kProcessWorld) > 0) {
        unsigned int uiref;
        if (mXfmCaches[unk36c6c].CacheXfms(mesh, floats, ui3, uiref)) {
            mesh->GetBlurCache().mCacheKey[unk36c6c] = uiref;
        }
    }
}
