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
    OBJ_CLASSNAME(ChallengeResultPanel)
    OBJ_SET_TYPE(ChallengeResultPanel)
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);

    virtual void Text(int, int, UIListLabel *, UILabel *) const;
    virtual int NumData() const;

    NEW_OBJ(ChallengeResultPanel)

protected:
    DataNode OnMsg(const UIComponentScrollMsg &);

    UIList *mChallengeList; // 0x40
    HamNavList *mRightHandNavList; // 0x44
    PropertyEventProvider *mResultEventProvider; // 0x48
    int unk4c;
    std::vector<ChallengeRow> mItems; // 0x50
    int unk5c; // 0x5c - score
    int unk60; // 0x60 - challenge mission index
    int unk64;
    SkeletonSide unk68; // 0x68 - side
    int unk6c;

private:
    virtual ~ChallengeResultPanel();
    virtual void Poll();
    virtual void FinishLoad();

    ChallengeResultPanel();
    void UpdateList(int);
};
