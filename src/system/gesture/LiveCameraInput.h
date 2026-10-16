#pragma once
#include "gesture/CameraInput.h"
#include "gesture/SpeechMgr.h"
#include "obj/Data.h"
#include "rnddx9/Tex.h"
#include "rndobj/Mat.h"
#include "rndobj/Tex.h"
#include "xdk/NUI.h"
#include "xdk/nui/nuidetroit.h"

struct CamTexClip {
    CamTexClip() : mTex(nullptr) {}
    void
    StoreTextureClip(RndTex *pTex, float xoff, float yoff, float width, float height);

    Transform mTexGen; // 0x0
    RndTex *mTex; // 0x40
};

// size 0x14e0
class LiveCameraInput : public CameraInput {
public:
    enum BufferType {
        kBufferColor = 0,
        kBufferDepth = 1,
        kBufferPlayer = 2,
        kBufferPlayerColor = 3,
        kBufferNum = 4,
    };
    struct LockedRect {
        INT mPitch; // 0x0
        VOID *mBits; // 0x4
    };
    // size 0x18
    struct StreamData {
        HANDLE mStreamHandle; // 0x0
        const NUI_IMAGE_FRAME *mImageFrames[2]; // 0x4
        int mCurrentPing; // 0xc
        int mCurrentPong; // 0x10
        RndMat *mMat; // 0x14
    };
    class TextureStore {
    public:
        TextureStore() : mTex(nullptr) {}
        ~TextureStore() { RELEASE(mTex); }
        void StoreTexture(RndTex *pTex);
        void StoreColorBuffer(LiveCameraInput *pCam);
        void StoreColorBufferClip(
            LiveCameraInput *pCam, float xoff, float yoff, float xdim, float ydim
        );
        void StoreDepthBuffer(LiveCameraInput *pCam);
        void StoreDepthBufferClip(
            LiveCameraInput *pCam, float xoff, float yoff, float xdim, float ydim
        );
        void UpdateFromColorBuffer(LiveCameraInput *pCam);
        void UpdateFromColorBufferClip(LiveCameraInput *pCam, float xoff, float yoff);
        void UpdateFromDepthBuffer(LiveCameraInput *pCam);
        void UpdateFromDepthBufferClip(LiveCameraInput *pCam, float xoff, float yoff);

        RndTex *mTex; // 0x0
    };
    virtual bool IsConnected() const { return mCameraConnected; }
    virtual void PollTracking();

    int MaxSnapshots() const { return mMaxSnapshots; }
    int NumSnapshots() const;
    RndMat *GetSnapshot(int index) const;
    int GetSnapshotBatchStartingIndex(int batch) const;
    RndTex *GetStoredTexture(int index) const;
    void InitSnapshots(int numSnapshots);
    void ClearSnapshots();
    void InitTextureStore(int maxStoreSize);
    void ClearTextureStore();
    void StartSnapshotBatch();
    int NumStoredTextures() const;
    int StoreTexture(RndTex *pTex);
    void StoreTextureAt(RndTex *pTex, int index);
    void StoreTextureClipAt(
        float xoff, float yoff, float width, float height, int srcIndex, int dstIndex
    );
    void ApplyTextureClip(RndMat *pMat, int index) const;
    void StoreColorBuffer(int index);
    void StoreColorBufferClip(float xoff, float yoff, float xwid, float yhit, int index);
    void StoreDepthBuffer(int index);
    void StoreDepthBufferClip(float xoff, float yoff, float xwid, float yhit, int index);
    void ResetSnapshots();
    int NumSnapshotBatches() const;
    bool GetTweakedAutoexposure() const;
    bool SetTweakedAutoexposure(bool tweak);
    void SetExposureRegion(float left, float top, float width, float height);
    bool SetAutoexposure(bool on);
    bool GetAutoexposure() const;
    void DumpProperties();
    void SetTrackedSkeletons(int trackingID1, int trackingID2) const;
    void IncrementSnapshotCount();
    void SetNewFrame(const SkeletonFrame *new_frame);
    void PollNewStream(BufferType buf);
    void *StreamBufferData(BufferType type) const;
    RndMat *DisplayMat(BufferType type) const;
    RndTex *DisplayTex(BufferType type) const;
    RndTex *GetStreamTex(BufferType type) const;
    void LockStream(const void *stream, LockedRect &rect);
    void UnlockStream(const void *stream);

    static void PreInit();
    static void Init();
    static void NuiAudioErrorCallback(HRESULT hr);
    static void NuiAudioDataCallback(NUIAUDIO_RESULTS *pNuiAudioResults);
    static LiveCameraInput *sInstance;

    bool ColorBufferPolled() const { return mColorBufferPolled; }
    bool DepthBufferPolled() const { return mDepthBufferPolled; }
    bool NewColorBuffer() const { return mNewColorBuffer; }
    bool NewDepthBuffer() const { return mNewDepthBuffer; }

protected:
    LiveCameraInput();
    virtual ~LiveCameraInput();
    virtual const SkeletonFrame *PollNewFrame() { return nullptr; }

    static void Terminate();

    BOOL m_bNuiAudioInitialized; // 0x11d4
    NUIAUDIO_HANDLE m_hNuiAudio; // 0x11d8
    int m_NuiAudioPipelineStatus; // 0x11dc
    float m_fBeamDirection; // 0x11e0
    float m_fConfidence; // 0x11e4
    bool mCameraConnected; // 0x11e8
    bool mColorBufferPolled; // 0x11e9
    bool mDepthBufferPolled; // 0x11ea
    bool mNewColorBuffer; // 0x11eb
    bool mNewDepthBuffer; // 0x11ec
    int mMaxSnapshots; // 0x11f0
    std::vector<RndMat *> mSnapshots; // 0x11f4
    int mNumSnapshots; // 0x1200
    std::vector<int> mSnapshotBatches; // 0x1204
    int mMaxStoredTextures; // 0x1210
    std::vector<TextureStore> mTextureStore; // 0x1214
    int mNumStoredTextures; // 0x1220
    CamTexClip mTextureClips[8]; // 0x1224
    SpeechMgr *mSpeechMgr; // 0x1444
    StreamData mStreams[kBufferNum]; // 0x1448
    DxTex *mColorStreamTexture; // 0x14a8
    DxTex *mDepthStreamTexture; // 0x14ac
    RndTex *mDepthDebugTexture; // 0x14b0
};

void CameraDump(const char *);
void CameraDumpUnique(const char *);
DataNode OnCameraDumpUnique(DataArray *);
DataNode OnCameraDebugDepth(DataArray *);
