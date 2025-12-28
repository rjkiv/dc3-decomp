#pragma once

#include "hamobj/BustAMoveData.h"
#include "hamobj/DancerSkeleton.h"
#include "lazer/meta_ham/HamPanel.h"

class BustAMovePanel : public HamPanel {
public:
    BustAMovePanel();
    // Hmx::Object
    virtual ~BustAMovePanel();
    OBJ_CLASSNAME(BustAMovePanel);
    OBJ_SET_TYPE(BustAMovePanel);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    // UIPanel
    virtual void Poll();
    virtual void Exit();

    void OnBeat();
    void SetUpSongStructure(Symbol);
    void PlayVO(Symbol);

protected:
    u32 unk_0x3C;
    u32 unk_0x40; // FreestyleMoveRecorder*
    u32 unk_0x44;
    double unk_0x48;
    u32 unk_0x50;
    u32 unk_0x54;
    u32 unk_0x58; // FreestyleMoveRecorder*
    u8 pad[0x24];
    u32 unk_0x84;
    u8 pad2[0xc];

    ObjectDir *mDirs[2]; // 0x94
    u32 unk_0x9C;
    u32 unk_0xA0;
    DancerSkeleton mSkeletons[3]; // 0xA4

    bool unk_0x9B8;

private:
    bool InBustAMove();

    void CacheObjects();
    float GetMovePromptVOLength();
    void PlayIntroVO();
    void QueueMovePromptVO();
    void PlayMovePromptVO();
    DataArray *GetMoveNameData(int);
};
