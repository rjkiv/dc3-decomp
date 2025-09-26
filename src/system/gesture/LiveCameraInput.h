#pragma once
#include "gesture/CameraInput.h"
#include "gesture/SpeechMgr.h"
#include "rndobj/Mat.h"
#include "rndobj/Tex.h"

struct CamTexClip {
    void StoreTextureClip(RndTex *, float, float, float, float);

    Transform mXfm; // 0x0
    RndTex *mTex; // 0x40
};

// size 0x14e0
class LiveCameraInput : public CameraInput {
public:
    enum BufferType {
        kBufferColor = 0,
        kBufferDepth = 1
    };
    class TextureStore {
    public:
        ~TextureStore() { RELEASE(mTex); }
        void StoreTexture(RndTex *);

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
    void StoreTextureClipAt(float, float, float, float, int, int);
    void ResetSnapshots();
    int NumSnapshotBatches() const;
    bool GetTweakedAutoexposure() const;
    bool SetTweakedAutoexposure(bool);
    void SetExposureRegion(float, float, float, float);
    bool SetAutoexposure(bool);
    bool GetAutoexposure() const;
    void DumpProperties();
    void SetTrackedSkeletons(int, int) const;

    static void PreInit();
    static void Init();
    static LiveCameraInput *sInstance;

protected:
    LiveCameraInput();
    virtual ~LiveCameraInput();

    static void Terminate();

    int unk11d4;
    int unk11d8;
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
    HANDLE mStreams[3]; // 0x1448
    int unk1454;
};
