#pragma once
#include "HamPanel.h"
#include "flow/PropertyEventProvider.h"
#include "hamobj/HamNavList.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "stl/_vector.h"
#include "ui/UIList.h"
#include "ui/UIListProvider.h"
#include "utl/Symbol.h"

class ChallengeRow { // size 0x3c
public:
    ChallengeRow();

    u32 unk0;
    String unk4;
    u32 unkc;
    String unk10;
    String unk18;
    u32 unk20;
    u32 unk24;
    u32 unk28;
    String unk2c;
    u32 unk34;
    u32 unk38;
};

class ChallengeResultPanel : public UIListProvider, public HamPanel {
public:
    OBJ_CLASSNAME(ChallengeResultPanel)
    OBJ_SET_TYPE(ChallengeResultPanel)
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);

    virtual void Text(int, int, UIListLabel *, UILabel *) const;
    virtual int NumData() const;

protected:
    // DataNode OnMsg(UIComponentScrollMsg const &);

    UIList *unk40;
    HamNavList *unk44;
    PropertyEventProvider *unk48;
    int unk4c;
    std::vector<ChallengeRow> mItems; // 0x50
    int unk5c;
    int unk60;
    int unk64;
    u32 unk68;
    int unk6c;

private:
    virtual ~ChallengeResultPanel();
    virtual void Poll();
    virtual void FinishLoad();

    ChallengeResultPanel();
    void UpdateList(int);
};
