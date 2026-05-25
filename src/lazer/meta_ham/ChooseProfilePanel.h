#pragma once
#include "meta_ham/HamPanel.h"
#include "meta_ham/HamProfile.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "os/PlatformMgr.h"
#include "ui/PanelDir.h"
#include "ui/UIListProvider.h"

class ChooseProfilePanel : public HamPanel, public UIListProvider {
public:
    // Hmx::Object
    OBJ_CLASSNAME(ChooseProfilePanel)
    OBJ_SET_TYPE(ChooseProfilePanel)
    virtual DataNode Handle(DataArray *, bool);
    // UIPanel
    virtual void Enter();
    virtual void Exit();
    // UIListProvider
    virtual void Text(int, int, UIListLabel *, UILabel *) const;
    virtual int NumData() const { return mPadNums.size(); }

    DataNode OnMsg(const SigninChangedMsg &);
    HamProfile *GetProfile(int);
    bool ProfileSelected(int) const;

    NEW_OBJ(ChooseProfilePanel)

private:
    void UpdateProfiles();

    std::vector<int> mPadNums; // 0x40
};
