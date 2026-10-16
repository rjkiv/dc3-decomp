#pragma once
#include "obj/Data.h"
#include "rndobj/Poll.h"
#include "rndobj/PostProc.h"
#include "utl/MemMgr.h"

/** "Settings for terr world effects." */
class RndPostProcMgr : public RndPollable {
public:
    // Hmx::Object
    virtual ~RndPostProcMgr();
    OBJ_CLASSNAME(PostProcMgr);
    OBJ_SET_TYPE(PostProcMgr);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, CopyType);
    virtual void Load(BinStream &);
    // RndPollable
    virtual void Poll();
    virtual void Enter();
    virtual void Exit();

    OBJ_MEM_OVERLOAD(0x22);
    NEW_OBJ(RndPostProcMgr)
    static void Init() { REGISTER_OBJ_FACTORY(RndPostProcMgr) }
    void CopyFromPostProc(RndPostProc *);
    void BlendToPostProc(RndPostProc *, float);

protected:
    RndPostProcMgr();

private:
    bool IsEnabled() const;
    RndPostProc *MsgToPostProc(DataArray *);

    DataNode OnCopyFromPostProc(DataArray *);
    DataNode OnBlendToPostProc(DataArray *);

    /** "Primary postproc to use.  When blending,
        the blend result will be written into this postproc." */
    ObjPtr<RndPostProc> mSelectedPostProc; // 0x8
    RndPostProc *mFromPostProc; // 0x1c
    ObjPtr<RndPostProc> mToPostProc; // 0x20
    float mPostProcBlendTime; // 0x34
    float mPostProcBlendStartTime; // 0x38
};
