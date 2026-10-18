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
    unsigned int key,
    unsigned int numXfms,
    const float *&xfmArray
) const {
    if (key + numXfms <= mEntryCount) {
        const RndMesh *__restrict start = mEntries_Mesh[key];
        const RndMesh *__restrict end = mEntries_Mesh[key + numXfms - 1];
        if (start == mesh && end == mesh) {
            xfmArray = &mEntries_Xfm[key * 12];
            return true;
        }
    }
    xfmArray = nullptr;
    return false;
}

bool RndXfmCache::CacheXfms(
    const RndMesh *__restrict mesh,
    const float *__restrict xfmArray,
    unsigned int numXfms,
    unsigned int &key
) {
    key = -1;
    if (mEntryCount + numXfms <= kMaxItems) {
        key = mEntryCount;
        int start = mEntryCount;

        float *xfmPtr = &mEntries_Xfm[start * 12];
        for (int i = 0; i < numXfms * 12; i++) {
            xfmPtr[i] = xfmArray[i];
        }

        unsigned int *bonePtr = &mEntries_Bone[start];
        const RndMesh **meshPtr = &mEntries_Mesh[start];
        int boneIdx = 0;
        for (int i = 0; i < numXfms; i++) {
            meshPtr[i] = mesh;
        }
        for (; boneIdx < numXfms; boneIdx++) {
            bonePtr[boneIdx] = boneIdx;
        }
        mEntryCount = start + numXfms;
    }
    return key < kMaxItems;
}

RndVelocityBuffer::RndVelocityBuffer()
    : mIntensity(0), mFrame(0), mNumFrames(0), mVelocityTex(nullptr), mMat(nullptr),
      mLastCamera(nullptr) {
    memset(&mCamSettings, 0, sizeof(mCamSettings));
}

void RndVelocityBuffer::CacheCameraSettings(RndCam *camera) {
    MILO_ASSERT(camera, 0x88);
    Transform tfa0;
    Hmx::Matrix4 me0;
    camera->GetViewProjectXfms(tfa0, me0);
    mCamSettings.mViewProj = tfa0 * me0;
    camera->GetDepthRangeValues(mCamSettings.mZRange);
    camera->GetCamFrustum(mCamSettings.mCamPos, mCamSettings.mCamCorner);
    mCamSettings.mCachedCam = camera;
}

bool RndVelocityBuffer::AdvanceFrame(RndCam *cam) {
    mReadyToUse = false;
    mFrame ^= 1;
    mNumFrames++;
    if (cam != mLastCamera) {
        mLastCamera = cam;
        mNumFrames = 0;
    }
    return mNumFrames >= 2;
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
    RndMat *mat = mesh->Mat();
    if (mat && mat->GetZMode() != BaseMaterial::kTransparent) {
        mesh->GetBlurCache().mShouldCache = true;
        unsigned int otherFrame = mFrame ^ 1;
        unsigned int frame = otherFrame ^ 1;
        int numBones = mesh->NumBones();
        const RndXfmCache &otherCache = mCache[otherFrame];
        const RndXfmCache &cache = mCache[frame];
        unsigned int otherKey = mesh->GetBlurCache().mCacheKey[otherFrame];
        unsigned int key = mesh->GetBlurCache().mCacheKey[frame];
        int numXfms = Max(1, numBones);
        const float *xfmCurr;
        const float *xfmPrev;
        if (otherCache.GetXfms(mesh, otherKey, numXfms, xfmCurr)
            && cache.GetXfms(mesh, key, numXfms, xfmPrev)) {
            if (numBones <= 40) {
                TheShaderMgr.SetMeshInfo(numBones, false);
                RndShader::SelectConfig(mMat, kVelocityObjectShader, false);
                TheShaderMgr.SetVConstant((VShaderConstant)9, xfmCurr, numXfms * 3);
                TheShaderMgr.SetVConstant((VShaderConstant)0x81, xfmPrev, numXfms * 3);
                TheShaderMgr.SetVConstant((VShaderConstant)0, mViewProj[otherFrame]);
                TheShaderMgr.SetVConstant(kVShader_ViewProj, mViewProj[frame]);
                TheShaderMgr.SetPConstant((PShaderConstant)8, mCamSettings.mZRange);
                mesh->GeomOwner()->DrawFacesInRange(0, -1);
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

void RndVelocityBuffer::ResetFrame() { mNumFrames = 0; }

void RndVelocityBuffer::CacheTransform(
    RndMesh *__restrict mesh, const float *__restrict xfmArray, unsigned int numXfms
) {
    if ((TheRnd.ProcCmds() & kProcessWorld) > kProcessNone) {
        unsigned int frame = mFrame;
        unsigned int key;
        if (mCache[frame].CacheXfms(mesh, xfmArray, numXfms, key)) {
            mesh->GetBlurCache().mCacheKey[frame] = key;
        }
    }
}
