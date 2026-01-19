#include "gesture/LiveCameraInput.h"
#include "DrawUtl.h"
#include "gesture/CameraInput.h"
#include "gesture/Skeleton.h"
#include "gesture/SkeletonUpdate.h"
#include "gesture/SpeechMgr.h"
#include "obj/Data.h"
#include "obj/DataFunc.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/System.h"
#include "rnddx9/Tex.h"
#include "rndobj/Bitmap.h"
#include "rndobj/Mat.h"
#include "rndobj/Tex.h"
#include "utl/MemTrack.h"
#include "utl/Std.h"
#include "xdk/nui/nuiapi.h"
#include "xdk/nui/nuiaudio.h"
#include "xdk/nui/nuidetroit.h"
#include "xdk/win_types.h"
#include "xdk/xapilibi/handleapi.h"
#include "xdk/xapilibi/winerror.h"

namespace {
    // bool  GetExposureRegion(struct _NUI_CAMERA_AE_ROI &)
    // long  GetColorCameraProperty(  _NUI_CAMERA_PROPERTY)

    void SetColorCameraProperty(NUI_CAMERA_PROPERTY, long);
    void LoadDebugDepthBuffer(class RndTex *&);
}

LiveCameraInput *LiveCameraInput::sInstance;
int g_ColorPollCnt;
int g_ColorNoFrameDataCnt;

#pragma region TextureStore

void LiveCameraInput::TextureStore::StoreTexture(RndTex *tex) {
    if (mTex) {
        RELEASE(mTex);
    }
    if (tex) {
        mTex = Hmx::Object::New<RndTex>();
        RndBitmap bitmap;
        tex->LockBitmap(bitmap, 1);
        mTex->SetBitmap(bitmap, nullptr, true, RndTex::kRenderedNoZ);
        tex->UnlockBitmap();
    } else {
        mTex = nullptr;
    }
}

void LiveCameraInput::TextureStore::StoreColorBuffer(LiveCameraInput *cam) {
    if (mTex) {
        if (mTex->Width() == 640 && mTex->Height() == 480)
            goto update;
        RELEASE(mTex);
    }
    mTex = Hmx::Object::New<RndTex>();
    mTex->SetBitmap(640, 480, 16, RndTex::kScratch, false, nullptr);
    MILO_ASSERT(mTex, 0x53A);
    MILO_ASSERT(mTex->Bpp() == 16, 0x53B);
    MILO_ASSERT(mTex->GetType() == RndTex::kScratch, 0x53C);
update:
    UpdateFromColorBuffer(cam);
}

void LiveCameraInput::TextureStore::StoreDepthBuffer(LiveCameraInput *cam) {
    if (mTex) {
        if (mTex->Width() == 640 && mTex->Height() == 480)
            goto update;
        RELEASE(mTex);
    }
    mTex = Hmx::Object::New<RndTex>();
    mTex->SetBitmap(640, 480, 16, RndTex::kScratch, false, nullptr);
    MILO_ASSERT(mTex, 0x602);
    MILO_ASSERT(mTex->Bpp() == 16, 0x603);
    MILO_ASSERT(mTex->GetType() == RndTex::kScratch, 0x604);
update:
    UpdateFromDepthBuffer(cam);
}

#pragma endregion
#pragma region LiveCameraInput

LiveCameraInput::LiveCameraInput()
    : mConnected(true), unk11e9(0), unk11ea(0), unk11eb(0), unk11ec(0), mSpeechMgr(0) {
    for (int i = 0; i < DIM(mTexClips); i++) {
        mTexClips[i].mTex = nullptr;
    }
    mSpeechMgr = nullptr;
    mNumSnapshots = 0;
    unk14a8 = 0;
    unk14ac = 0;
    unk14b0 = 0;
    mSnapshotBatches.clear();
    mNumSnapshots = 0;
    SkeletonUpdate::Init();
    DataArray *kinectArr = SystemConfig()->FindArray("kinect", false);
    bool b17 = false;
    if (kinectArr) {
        DataArray *speechArr = kinectArr->FindArray("speech");
        b17 = speechArr->FindArray("enabled")->Int(1);
    }
    for (int i = 0; i < kBufferNum; i++) {
        Buffer &cur = mStreams[i];
        cur.unk0 = nullptr;
        cur.unk4[0] = nullptr;
        cur.unk4[1] = nullptr;
        cur.unkc = 0;
        cur.unk10 = 1;
        cur.unk14 = nullptr;
    }
    int initFlags = 0x4049;
    if (!UsingCD()) {
        initFlags = 0x40004049;
    }
    BeginMemTrackObjectName("NuiInitialize");
    HRESULT initRes = NuiInitialize(initFlags, -1);
    EndMemTrackObjectName();
    if (initRes == E_NUI_DATABASE_NOT_FOUND) {
        MILO_NOTIFY(
            "Could not find NUI database.  Do you have Map DVD Drive enabled in Visual Studio?"
        );
    }
    MILO_ASSERT_FMT(SUCCEEDED(initRes), "NuiInitialize failed (0x%x)", initRes);
    if (b17) {
        mSpeechMgr = new SpeechMgr(kinectArr);
    }
    unk11d4 = 0;
    if (SUCCEEDED(NuiAudioCreate(5, NuiAudioErrorCallback, 1, &unk11d8, nullptr))) {
        NuiAudioRegisterCallbacks(&unk11d8, 1, NuiAudioDataCallback);
        unk11d4 = 1;
    }
    bool i6 = kinectArr->FindArray("title_tracked_skeletons")->Int(1);
    HANDLE new_skeleton_event = SkeletonUpdate::NewSkeletonEvent();
    MILO_ASSERT(new_skeleton_event, 0x14D);
    MILO_ASSERT_FMT(
        SUCCEEDED(NuiSkeletonTrackingEnable(new_skeleton_event, i6 ? 2 : 0)),
        "NuiSkeletonTrackingEnable failed"
    );
    BeginMemTrackObjectName("NuiImageStreamOpen:color");
    MILO_ASSERT_FMT(
        SUCCEEDED(NuiImageStreamOpen(
            NUI_IMAGE_TYPE_COLOR_YUV,
            NUI_IMAGE_RESOLUTION_640x480,
            0,
            2,
            nullptr,
            &mStreams[kBufferColor].unk0
        )),
        "NuiImageStreamOpen color failed"
    );
    EndMemTrackObjectName();
    mStreams[kBufferColor].unk14 = CreateCameraBufferMat(640, 480, RndTex::kScratch);
    BeginMemTrackObjectName("NuiImageStreamOpen:depth");
    MILO_ASSERT_FMT(
        SUCCEEDED(NuiImageStreamOpen(
            NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX_IN_COLOR_SPACE,
            NUI_IMAGE_RESOLUTION_320x240,
            0,
            2,
            0,
            &mStreams[kBufferDepth].unk0
        )),
        "NuiImageStreamOpen depth failed"
    );
    EndMemTrackObjectName();
    mStreams[kBufferDepth].unk14 = CreateCameraBufferMat(320, 240, RndTex::kScratch);
    mStreams[kBufferPlayer].unk14 = CreateCameraBufferMat(320, 240, RndTex::kScratch);
    mStreams[kBufferPlayerColor].unk14 =
        CreateCameraBufferMat(640, 480, RndTex::kScratch);
    RELEASE(unk14a8);
    unk14a8 = Hmx::Object::New<DxTex>();
    RELEASE(unk14ac);
    unk14ac = Hmx::Object::New<DxTex>();
    DataArray *maxArr = kinectArr->FindArray("camera")->FindArray("max_snapshots", false);
    if (maxArr) {
        mMaxSnapshots = maxArr->Int(1);
    } else {
        MILO_NOTIFY("Could not find max_snapshots in SystemConfig");
        mMaxSnapshots = 1;
    }
    mSnapshotBatches.reserve(6);
    SetColorCameraProperty(NUI_CAMERA_PROPERTY_AE_AWB_MODE, 1);
}

LiveCameraInput::~LiveCameraInput() {
    SkeletonUpdate::Terminate();
    for (int i = 0; i < 4; i++) {
        RndMat *curMat = mStreams[i].unk14;
        RndTex *diffuseTex = curMat ? curMat->GetDiffuseTex() : nullptr;
        delete diffuseTex;
        delete curMat;
        if (mStreams[i].unk0) {
            CloseHandle(mStreams[i].unk0);
        }
    }
    ClearSnapshots();
    if (unk11d4) {
        NuiAudioUnregisterCallbacks(&unk11d8, NuiAudioDataCallback);
        NuiAudioRelease(&unk11d8);
    }
    delete mSpeechMgr;
    NuiShutdown();
}

void LiveCameraInput::PollTracking() {
    unk11e9 = false;
    unk11ea = false;
    unk11eb = false;
    unk11ec = false;
    PollNewStream(kBufferColor);
    CameraInput::PollTracking();
}

DataNode OnCameraDumpUnique(DataArray *);
DataNode OnCameraDebugDepth(DataArray *);

void LiveCameraInput::PreInit() {
    if (!sInstance) {
        sInstance = new LiveCameraInput();
        SkeletonUpdate::CreateInstance();
        TheDebug.AddExitCallback(LiveCameraInput::Terminate);
        DataRegisterFunc("camera_dump", OnCameraDumpUnique);
        DataRegisterFunc("camera_debug_depth", OnCameraDebugDepth);
        LoadDebugDepthBuffer(sInstance->unk14b0);
    }
}

void LiveCameraInput::Init() {
    PreInit();
    if (sInstance) {
        DataArray *cfg = SystemConfig()->FindArray("kinect", false);
        if (cfg) {
            DataArray *speechArr = cfg->FindArray("speech");
            cfg = speechArr;
            speechArr->FindInt("enabled");
        }
        if (sInstance->mSpeechMgr) {
            sInstance->mSpeechMgr->InitGrammars(cfg);
        }
    }
}

void LiveCameraInput::Terminate() {
    if (!sInstance) {
        MILO_ASSERT(sInstance, 0xE2);
    }
    RELEASE(sInstance);
}

RndMat *LiveCameraInput::GetSnapshot(int idx) const {
    if ((idx < 0 || idx >= mNumSnapshots) && mNumSnapshots > 0) {
        MILO_LOG("Snapshot index %d out of bounds [0-%d].", idx, mNumSnapshots);
    } else if (idx < mSnapshots.size()) {
        return mSnapshots[idx];
    }
    return nullptr;
}

int LiveCameraInput::GetSnapshotBatchStartingIndex(int idx) const {
    return idx >= mSnapshotBatches.size() ? mNumSnapshots : mSnapshotBatches[idx];
}

RndTex *LiveCameraInput::GetStoredTexture(int idx) const {
    if (idx >= 0 && idx < mTextureStore.size()) {
        return mTextureStore[idx].mTex;
    } else {
        // lol did they forget the other %d
        MILO_LOG(
            "LiveCameraInput::GetStoredTexture: index %d out of bounds [max=%d]\n",
            mTextureStore.size() - 1
        );
        return nullptr;
    }
}

void LiveCameraInput::InitSnapshots(int numSnapshots) {
    ClearSnapshots();
    if (numSnapshots > 0) {
        MILO_ASSERT(numSnapshots <= mMaxSnapshots, 0x193);
        if (mSnapshots.size() != numSnapshots) {
            mSnapshots.resize(numSnapshots);
            for (int i = 0; i < numSnapshots; i++) {
                mSnapshots[i] = CreateCameraBufferMat(640, 480, RndTex::kRendered);
            }
        }
    }
}

void LiveCameraInput::InitTextureStore(int max) {
    mMaxTextures = max;
    ClearTextureStore();
}

void LiveCameraInput::ClearTextureStore() {
    for (int i = 0; i < mTextureStore.size(); i++) {
        RELEASE(mTextureStore[i].mTex);
    }
    mTextureStore.clear();
    mTextureStore.resize(mMaxTextures);
    mNumStoredTextures = 0;
}

void LiveCameraInput::StartSnapshotBatch() { mSnapshotBatches.push_back(mNumSnapshots); }

void LiveCameraInput::StoreTextureClipAt(
    float f1, float f2, float f3, float f4, int idx1, int idx2
) {
    if (idx1 >= 0 && idx1 < mTextureStore.size()) {
        mTexClips[idx2].StoreTextureClip(mTextureStore[idx1].mTex, f1, f2, f3, f4);
    } else {
        MILO_LOG(
            "LiveCameraInput::StoreColorBufferClip: index %d out of bounds [max=%d]\n",
            idx1,
            mTextureStore.size() - 1
        );
    }
}

void LiveCameraInput::ResetSnapshots() {
    mNumSnapshots = 0;
    mSnapshotBatches.clear();
}

void LiveCameraInput::SetNewFrame(const SkeletonFrame *frame) {
    mNewFrame = frame;
    mCachedFrame = *frame;
}

int LiveCameraInput::NumSnapshots() const { return mNumSnapshots; }
int LiveCameraInput::NumStoredTextures() const { return mNumStoredTextures; }

void LiveCameraInput::PollNewStream(BufferType buf) {
    MILO_ASSERT(kBufferColor == buf || kBufferDepth == buf, 0x227);
    MILO_ASSERT_RANGE(buf, 0, DIM(mStreams), 0x22C);
    Buffer &curBuf = mStreams[buf];
    if (curBuf.unk0) {
        HRESULT hr =
            NuiImageStreamGetNextFrame(curBuf.unk0, 0, &curBuf.unk4[curBuf.unkc]);
        if (buf == kBufferColor) {
            g_ColorPollCnt++;
            unk11e9 = true;
        } else {
            unk11ea = true;
        }
        if (SUCCEEDED(hr)) {
            if (buf == kBufferColor) {
                unk11eb = true;
            } else {
                unk11ec = true;
            }
            mConnected = true;
            unk14a8->SetDeviceTex(nullptr);
            unk14ac->SetDeviceTex(nullptr);
            if (curBuf.unk4[curBuf.unk10]) {
                HRESULT hr =
                    NuiImageStreamReleaseFrame(curBuf.unk0, curBuf.unk4[curBuf.unk10]);
                MILO_ASSERT(SUCCEEDED(hr), 0x24E);
                curBuf.unk4[curBuf.unk10] = 0;
            }
            curBuf.unkc = curBuf.unkc - 1U & 1;
            curBuf.unk10 = curBuf.unk10 - 1U & 1;
        } else if (hr == E_NUI_DEVICE_NOT_CONNECTED) {
            mConnected = false;
        } else if (hr == (HRESULT)0x83010001 && buf == kBufferColor) {
            g_ColorNoFrameDataCnt++;
        }
    }
}

void *LiveCameraInput::StreamBufferData(BufferType type) const {
    MILO_ASSERT(type < kBufferNum, 0x1FC);
    int i3;
    if (type == kBufferPlayer) {
        i3 = 1;
    } else {
        i3 = type == kBufferPlayerColor ? 1 : 0;
    }
    if (mStreams[type].unk4[i3]) {
        return mStreams[type].unk4[i3]->pFrameTexture;
    } else {
        return nullptr;
    }
}

RndMat *LiveCameraInput::DisplayMat(BufferType type) const {
    MILO_ASSERT(type < kBufferNum, 0x20F);
    return mStreams[type].unk14;
}

RndTex *LiveCameraInput::DisplayTex(BufferType type) const {
    RndMat *mat = DisplayMat(type);
    if (mat) {
        return mat->GetDiffuseTex();
    } else {
        return nullptr;
    }
}

void LiveCameraInput::DumpProperties() {
    MILO_LOG("NUI CAMERA PROPERTIES ****************************************\n");
    union {
        LONG lValue;
        FLOAT fValue;
    };
    NuiCameraGetProperty(NUI_CAMERA_TYPE_COLOR, NUI_CAMERA_PROPERTY_AE_AWB_MODE, &lValue);
    MILO_LOG("%s:\n", "NUI_CAMERA_PROPERTY_AE_AWB_MODE");
    if (lValue == NUI_CAMERA_PROPERTY_AE_AWB_MODE_STANDARD) {
        MILO_LOG("      %s\n", "NUI_CAMERA_PROPERTY_AE_AWB_MODE_STANDARD");
    }
    if (lValue == NUI_CAMERA_PROPERTY_AE_AWB_MODE_FACEBASED) {
        MILO_LOG("      %s\n", "NUI_CAMERA_PROPERTY_AE_AWB_MODE_FACEBASED");
    }
    if (lValue == NUI_CAMERA_PROPERTY_AE_AWB_MODE_OFF) {
        MILO_LOG("      %s\n", "NUI_CAMERA_PROPERTY_AE_AWB_MODE_OFF");
    }
    if (lValue == NUI_CAMERA_PROPERTY_AE_AWB_MODE_DEFAULT) {
        MILO_LOG("      %s\n", "NUI_CAMERA_PROPERTY_AE_AWB_MODE_DEFAULT");
    }
    NuiCameraGetProperty(
        NUI_CAMERA_TYPE_COLOR, NUI_CAMERA_PROPERTY_FRAME_RATE_MAX, &lValue
    );
    MILO_LOG("%s:\n", "NUI_CAMERA_PROPERTY_FRAME_RATE_MAX");
    if (lValue == NUI_CAMERA_PROPERTY_FRAME_RATE_MAX_30FPS) {
        MILO_LOG("      %s\n", "NUI_CAMERA_PROPERTY_FRAME_RATE_MAX_30FPS");
    }
    if (lValue == NUI_CAMERA_PROPERTY_FRAME_RATE_MAX_15FPS) {
        MILO_LOG("      %s\n", "NUI_CAMERA_PROPERTY_FRAME_RATE_MAX_15FPS");
    }
    if (lValue == NUI_CAMERA_PROPERTY_FRAME_RATE_MAX_10FPS) {
        MILO_LOG("      %s\n", "NUI_CAMERA_PROPERTY_FRAME_RATE_MAX_10FPS");
    }
    NuiCameraGetProperty(
        NUI_CAMERA_TYPE_COLOR, NUI_CAMERA_PROPERTY_AE_FRAME_RATE_MIN, &lValue
    );
    MILO_LOG("%s:\n", "NUI_CAMERA_PROPERTY_AE_FRAME_RATE_MIN");
    if (lValue == NUI_CAMERA_PROPERTY_AE_FRAME_RATE_MIN_30FPS) {
        MILO_LOG("      %s\n", "NUI_CAMERA_PROPERTY_AE_FRAME_RATE_MIN_30FPS");
    }
    if (lValue == NUI_CAMERA_PROPERTY_AE_FRAME_RATE_MIN_15FPS) {
        MILO_LOG("      %s\n", "NUI_CAMERA_PROPERTY_AE_FRAME_RATE_MIN_15FPS");
    }
    if (lValue == NUI_CAMERA_PROPERTY_AE_FRAME_RATE_MIN_10FPS) {
        MILO_LOG("      %s\n", "NUI_CAMERA_PROPERTY_AE_FRAME_RATE_MIN_10FPS");
    }
    if (lValue == NUI_CAMERA_PROPERTY_AE_FRAME_RATE_MIN_LOWEST) {
        MILO_LOG("      %s\n", "NUI_CAMERA_PROPERTY_AE_FRAME_RATE_MIN_LOWEST");
    }
    if (lValue == NUI_CAMERA_PROPERTY_AE_FRAME_RATE_MODE_MIN_15FPS) {
        MILO_LOG("      %s\n", "NUI_CAMERA_PROPERTY_AE_FRAME_RATE_MODE_MIN_15FPS");
    }
    if (lValue == NUI_CAMERA_PROPERTY_AE_FRAME_RATE_MODE_MIN_10FPS) {
        MILO_LOG("      %s\n", "NUI_CAMERA_PROPERTY_AE_FRAME_RATE_MODE_MIN_10FPS");
    }
    NuiCameraGetPropertyF(
        NUI_CAMERA_TYPE_COLOR, NUI_CAMERA_PROPERTYF_EXPOSURE_TIME, &fValue
    );
    MILO_LOG("%s:\t\t%.4f\n", "NUI_CAMERA_PROPERTYF_EXPOSURE_TIME", fValue);
    NuiCameraGetPropertyF(
        NUI_CAMERA_TYPE_COLOR, NUI_CAMERA_PROPERTYF_AE_EXPOSURE_COMPENSATION, &fValue
    );
    MILO_LOG("%s:\t\t%.4f\n", "NUI_CAMERA_PROPERTYF_AE_EXPOSURE_COMPENSATION", fValue);
    NuiCameraGetPropertyF(NUI_CAMERA_TYPE_COLOR, NUI_CAMERA_PROPERTYF_COLOR_GAIN, &fValue);
    MILO_LOG("%s:\t\t%.4f\n", "NUI_CAMERA_PROPERTYF_COLOR_GAIN", fValue);
    NuiCameraGetPropertyF(
        NUI_CAMERA_TYPE_COLOR, NUI_CAMERA_PROPERTYF_AE_FACEBASED_MAX_GAIN, &fValue
    );
    MILO_LOG("%s:\t\t%.4f\n", "NUI_CAMERA_PROPERTYF_AE_FACEBASED_MAX_GAIN", fValue);
    NUI_CAMERA_AE_ROI region;
    HRESULT hr = NuiCameraGetExposureRegionOfInterest(NUI_CAMERA_TYPE_COLOR, &region);
    if (hr != ERROR_SUCCESS) {
        MILO_LOG("NuiCameraGetExposureRegionOfInterest returned bad result.\n");
    } else {
        MILO_LOG(
            "Region of Interest: left(%.3f) top(%.3f) width(%.3f) height(%.3f)\n",
            region.Left,
            region.Top,
            region.Width,
            region.Height
        );
    }
    MILO_LOG("**************************************************************\n");
}

void LiveCameraInput::IncrementSnapshotCount() {
    if (mNumSnapshots < mSnapshots.size()) {
        mNumSnapshots++;
    } else {
        MILO_NOTIFY("Max snapshots already taken.");
    }
}

int LiveCameraInput::NumSnapshotBatches() const { return mSnapshotBatches.size(); }

void LiveCameraInput::NuiAudioErrorCallback(HRESULT hr) {
    if (hr != ERROR_SUCCESS) {
        MILO_NOTIFY("NuiAudioErrorCallback reached (0x%x)", hr);
    }
}

int LiveCameraInput::StoreTexture(RndTex *tex) {
    if (mNumStoredTextures >= mMaxTextures) {
        MILO_ASSERT(mNumStoredTextures==mTextureStore.size(), 799);
        MILO_LOG(
            "LiveCameraInput::AddTextureToStore: No room available. Max textures=\n",
            mMaxTextures
        );
        return 0;
    } else {
        for (int i = 0; i < mTextureStore.size(); i++) {
            if (!mTextureStore[i].mTex) {
                mTextureStore[i].mTex = tex;
                return i;
            }
        }
        mTextureStore[mNumStoredTextures++].StoreTexture(tex);
        return mNumStoredTextures - 1;
    }
}

void LiveCameraInput::StoreTextureAt(RndTex *tex, int idx) {
    if (idx >= 0 && idx < mMaxTextures) {
        mTextureStore[idx].StoreTexture(tex);
    } else {
        // i think they forgot the second %d
        MILO_LOG(
            "LiveCameraInput::StoreTextureAt: index %d out of bounds [max=%d]\n",
            mTextureStore.size() - 1
        );
    }
}

void LiveCameraInput::ApplyTextureClip(RndMat *mat, int idx) const {
    if (!(idx >= 0 && idx < 8)) {
        MILO_LOG(
            "LiveCameraInput::GetStoredTexture: index %d out of bounds [max=%d]\n", 7
        );
    }
    mat->SetTexGen(kTexGenXfmOrigin);
    mat->SetTexXfm(mTexClips[idx].mXfm);
    mat->SetDiffuseTex(mTexClips[idx].mTex);
}

void LiveCameraInput::StoreColorBuffer(int idx) {
    if (idx >= 0 && idx < mTextureStore.size()) {
        mTextureStore[idx].StoreColorBuffer(this);
    } else {
        MILO_LOG(
            "LiveCameraInput::StoreColorBuffer: index %d out of bounds [max=%d]\n",
            idx,
            mTextureStore.size() - 1
        );
    }
}

void LiveCameraInput::StoreColorBufferClip(
    float f1, float f2, float f3, float f4, int idx
) {
    if (idx >= 0 && idx < mTextureStore.size()) {
        mTextureStore[idx].StoreColorBufferClip(this, f1, f2, f3, f4);
    } else {
        MILO_LOG(
            "LiveCameraInput::StoreColorBufferClip: index %d out of bounds [max=%d]\n",
            idx,
            mTextureStore.size() - 1
        );
    }
}

void LiveCameraInput::StoreDepthBuffer(int idx) {
    if (idx >= 0 && idx < mTextureStore.size()) {
        mTextureStore[idx].StoreDepthBuffer(this);
    } else {
        MILO_LOG(
            "LiveCameraInput::StoreDepthBufferAt: index %d out of bounds [max=%d]\n",
            idx,
            mTextureStore.size() - 1
        );
    }
}

void LiveCameraInput::StoreDepthBufferClip(
    float f1, float f2, float f3, float f4, int idx
) {
    if (idx >= 0 && idx < mTextureStore.size()) {
        mTextureStore[idx].StoreDepthBufferClip(this, f1, f2, f3, f4);
    } else {
        MILO_LOG(
            "LiveCameraInput::StoreDepthBufferClip: index %d out of bounds [max=%d]\n",
            idx,
            mTextureStore.size() - 1
        );
    }
}
