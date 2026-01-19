#pragma once
#include "obj/Data.h"
#include "obj/Object.h"
#include "rndobj/Dir.h"
#include "rndobj/Draw.h"
#include "rndobj/Mat.h"
#include "rndobj/Poll.h"
#include "rndobj/Tex.h"
#include "rndobj/TexRenderer.h"
#include "utl/BinStream.h"
#include "utl/MemMgr.h"
#include <list>

class StreamRecorder : public RndDrawable,
                       public RndPollable,
                       public Rnd::CompressTextureCallback {
public:
    // Hmx::Object
    virtual ~StreamRecorder();
    OBJ_CLASSNAME(StreamRecorder)
    OBJ_SET_TYPE(StreamRecorder)
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(Hmx::Object const *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // RndDrawable
    virtual void DrawShowing();
    // RndPollable
    virtual void Poll();
    virtual void Exit();
    // Rnd::CompressTextureCallback
    virtual void TextureCompressed(int) { mFramesRecorded++; }

    NEW_OBJ(StreamRecorder)
    OBJ_MEM_OVERLOAD(0x24)

protected:
    StreamRecorder();

    void StopRecordingImmediate();
    bool SetFrame(int);
    void DeleteBuffers();
    void SetPhotoInput(RndDir *);
    void StoppedRecordingScript();
    void CompressTextures();
    void Reset();
    void SetDebugFrame(int);

    DataNode OnStopRecording(DataArray *);
    DataNode OnStopPlayback(DataArray *);
    DataNode OnPausePlayback(DataArray *);
    DataNode OnUnpausePlayback(DataArray *);
    DataNode OnPlayRecording(DataArray *);
    DataNode OnStartRecording(DataArray *);
    DataNode OnReset(DataArray *);

    ObjPtr<RndDir> unk4c; // 0x4c
    ObjPtr<RndTexRenderer> unk60; // 0x60
    ObjPtrVec<RndTex> mBuffers; // 0x74
    ObjPtr<RndMat> mOutputMat; // 0x90
    int mMaxFrames; // 0xa4
    int mOutputWidth; // 0xa8
    int mOutputHeight; // 0xac
    int mFramesRecorded; // 0xb0
    int unkb4;
    int mDebugFrame; // 0xb8
    int mPlaybackSpeed; // 0xbc - actually an enum StreamPlaybackSpeed
    float unkc0;
    float unkc4;
    float unkc8;
    std::list<int> unkcc; // 0xcc
    bool mUseAlpha; // 0xd4
    int unkd8;
    int unkdc;
};
