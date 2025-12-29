#pragma once
#include "meta_ham/HamPanel.h"
#include "meta_ham/HamProfile.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "os/PlatformMgr.h"
#include "stl/_vector.h"
#include "ui/PanelDir.h"

class ChooseProfilePanel : public HamPanel {
public:
    // Hmx::Object
    OBJ_CLASSNAME(ChooseProfilePanel)
    virtual DataNode Handle(DataArray *, bool);

    // UIPanel
    virtual void Enter();
    virtual void Exit();

    // UIListProvider
    virtual void Text(int, int, UIListLabel *, UILabel *) const;

    ChooseProfilePanel();
    HamProfile *GetProfile(int);
    DataNode OnMsg(SigninChangedMsg const &);

    ObjectDir *unk3c;
    std::vector<int> mPadNums; // 0x40

private:
    void UpdateProfiles();
};
