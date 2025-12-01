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

    // Rnd::CompressTextureCallback
    virtual void TextureCompressed(int);

    // RndPollable
    virtual void Poll();
    virtual void Exit();

    // RndDrawable
    virtual void DrawShowing();

    NEW_OBJ(StreamRecorder)
    OBJ_MEM_OVERLOAD(0x24)

    ObjPtr<RndDir> unk4c;
    ObjPtr<RndTexRenderer> unk60;
    ObjPtrVec<RndTex> mBuffers; // 0x74
    ObjPtr<RndMat> mOutputMat; // 0x90
    int mMaxFrames; // 0xa4
    int mOutputWidth; // 0xa8
    int mOutputHeight; // 0xac
    int mFramesRecorded; // 0xb0
    int unkb4;
    int unkb8;
    int mPlaybackSpeed; // 0xbc
    float unkc0;
    float unkc4;
    float unkc8;
    std::list<int> unkcc;
    bool mUseAlpha; // 0xd4
    int unkd8;
    int unkdc;

protected:
    StreamRecorder();
    DataNode OnStopRecording(DataArray *);
    DataNode OnStopPlayback(DataArray *);
    DataNode OnPausePlayback(DataArray *);
    DataNode OnUnpausePlayback(DataArray *);
    void StopRecordingImmediate();
    bool SetFrame(int);
    void DeleteBuffers();
    void SetPhotoInput(RndDir *);
    void StoppedRecordingScript();
    DataNode OnPlayRecording(DataArray *);
    void CompressTextures();
    void Reset();
    DataNode OnStartRecording(DataArray *);
    DataNode OnReset(DataArray *);
};
