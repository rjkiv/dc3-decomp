#pragma once
#include "gesture/BaseSkeleton.h"
#include "meta_ham/HamPanel.h"
#include "flow/PropertyEventProvider.h"
#include "hamobj/HamNavList.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "ui/UIList.h"
#include "ui/UIListProvider.h"
#include "utl/Symbol.h"
#include "net_ham/ChallengeSystemJobs.h"

class ChallengeResultPanel : public UIListProvider, public HamPanel {
public:
    enum PanelState {
        kStateInit = 0x0000,
        kStateScrollToRival = 0x0001,
        kStateCheckingRivalResult = 0x0002,
        kStateScrollToEnd = 0x0003,
        kStateCheckingFinalResult = 0x0004,
        kStateEnd = 0x0005
    };

    enum ResultGrade {
        kBeatNoOne = 0x0000,
        kBeatSomeButNotRival = 0x0001,
        kBeatRival = 0x0002,
        kBeatRivalButNotEveryone = 0x0003,
        kBeatAll = 0x0004
    };

    OBJ_CLASSNAME(ChallengeResultPanel)
    OBJ_SET_TYPE(ChallengeResultPanel)
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);

    virtual void Text(int, int, UIListLabel *, UILabel *) const;
    virtual int NumData() const;

    NEW_OBJ(ChallengeResultPanel)

protected:
    DataNode OnMsg(const UIComponentScrollMsg &);

private:
    virtual ~ChallengeResultPanel();
    virtual void Poll();
    virtual void FinishLoad();

    ChallengeResultPanel();
    void UpdateList(int);

    UIList *mList; // 0x40
    HamNavList *mNavList; // 0x44
    PropertyEventProvider *mSettings; // 0x48
    PanelState mState; // 0x4c - not a PanelState in pdb
    std::vector<ChallengeRow> mItems; // 0x50
    int mScore; // 0x5c - score
    int mMissionIndex; // 0x60
    int mCheckingIdx; // 0x64
    SkeletonSide unk68; // 0x68 - not a SkeletonSide in pdb
    int mMyIndex; // 0x6c
};
