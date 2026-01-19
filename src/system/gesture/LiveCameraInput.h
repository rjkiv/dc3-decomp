#pragma once
#include "gesture/CameraInput.h"
#include "gesture/SpeechMgr.h"
#include "rnddx9/Tex.h"
#include "rndobj/Mat.h"
#include "rndobj/Tex.h"
#include "xdk/NUI.h"

struct CamTexClip {
    void StoreTextureClip(RndTex *, float, float, float, float);

    Transform mXfm; // 0x0
    RndTex *mTex; // 0x40
};

// size 0x14e0
class LiveCameraInput : public CameraInput {
public:
    enum BufferType {
        kBufferOff = -1,
        kBufferColor = 0,
        kBufferDepth = 1,
        kBufferPlayer = 2,
        kBufferPlayerColor = 3,
        kBufferNum = 4,
    };
    struct LockedRect {
        // 0x0
    };
    // size 0x18
    struct Buffer {
        HANDLE unk0;
        const NUI_IMAGE_FRAME *unk4[2];
        int unkc;
        int unk10;
        RndMat *unk14;
    };
    class TextureStore {
    public:
        ~TextureStore() { RELEASE(mTex); }
        void StoreTexture(RndTex *);
        void StoreColorBuffer(LiveCameraInput *);
        void StoreColorBufferClip(LiveCameraInput *, float, float, float, float);
        void StoreDepthBuffer(LiveCameraInput *);
        void StoreDepthBufferClip(LiveCameraInput *, float, float, float, float);
        void UpdateFromColorBuffer(LiveCameraInput *);
        void UpdateFromColorBufferClip(LiveCameraInput *, float, float);
        void UpdateFromDepthBuffer(LiveCameraInput *);
        void UpdateFromDepthBufferClip(LiveCameraInput *, float, float);

        RndTex *mTex; // 0x0
    };
    virtual bool IsConnected() const { return mConnected; }
    virtual void PollTracking();

    int MaxSnapshots() const { return mMaxSnapshots; }
    int NumSnapshots() const;
    RndMat *GetSnapshot(int) const;
    int GetSnapshotBatchStartingIndex(int) const;
    RndTex *GetStoredTexture(int) const;
    void InitSnapshots(int);
    void ClearSnapshots();
    void InitTextureStore(int);
    void ClearTextureStore();
    void StartSnapshotBatch();
    int NumStoredTextures() const;
    int StoreTexture(RndTex *);
    void StoreTextureAt(RndTex *, int);
    void StoreTextureClipAt(float, float, float, float, int, int);
    void ApplyTextureClip(RndMat *, int) const;
    void StoreColorBuffer(int);
    void StoreColorBufferClip(float, float, float, float, int);
    void StoreDepthBuffer(int);
    void StoreDepthBufferClip(float, float, float, float, int);
    void ResetSnapshots();
    int NumSnapshotBatches() const;
    bool GetTweakedAutoexposure() const;
    bool SetTweakedAutoexposure(bool);
    void SetExposureRegion(float, float, float, float);
    bool SetAutoexposure(bool);
    bool GetAutoexposure() const;
    void DumpProperties();
    void SetTrackedSkeletons(int, int) const;
    void IncrementSnapshotCount();
    void SetNewFrame(const SkeletonFrame *);
    void PollNewStream(BufferType);
    void *StreamBufferData(BufferType) const;
    RndMat *DisplayMat(BufferType) const;
    RndTex *DisplayTex(BufferType) const;

    static void PreInit();
    static void Init();
    static void NuiAudioErrorCallback(HRESULT);
    static void NuiAudioDataCallback(NUIAUDIO_RESULTS *);
    static LiveCameraInput *sInstance;

protected:
    LiveCameraInput();
    virtual ~LiveCameraInput();
    virtual const SkeletonFrame *PollNewFrame() { return nullptr; }

    static void Terminate();

    int unk11d4;
    HANDLE unk11d8;
    int unk11dc;
    int unk11e0;
    int unk11e4;
    bool mConnected; // 0x11e8
    bool unk11e9;
    bool unk11ea;
    bool unk11eb;
    bool unk11ec;
    int mMaxSnapshots; // 0x11f0
    std::vector<RndMat *> mSnapshots; // 0x11f4
    int mNumSnapshots; // 0x1200
    std::vector<int> mSnapshotBatches; // 0x1204
    int mMaxTextures; // 0x1210
    std::vector<TextureStore> mTextureStore; // 0x1214
    int mNumStoredTextures; // 0x1220
    CamTexClip mTexClips[8]; // 0x1224
    SpeechMgr *mSpeechMgr; // 0x1444
    Buffer mStreams[kBufferNum]; // 0x1448
    DxTex *unk14a8;
    DxTex *unk14ac;
    RndTex *unk14b0;
};
