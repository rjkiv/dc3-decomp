#include "meta_ham/ChooseProfilePanel.h"
#include "AppLabel.h"
#include "ProfileMgr.h"
#include "meta_ham/HamPanel.h"
#include "meta_ham/HamProfile.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/PlatformMgr.h"
#include "ui/UILabel.h"
#include "ui/UIListLabel.h"
#include "utl/Symbol.h"

ChooseProfilePanel::ChooseProfilePanel() {}

void ChooseProfilePanel::Enter() {
    UpdateProfiles();
    HamPanel::Enter();
    ThePlatformMgr.AddSink(this);
}

void ChooseProfilePanel::Exit() { ThePlatformMgr.RemoveSink(this); }

HamProfile *ChooseProfilePanel::GetProfile(int index) {
    MILO_ASSERT(index >= 0, 0x3c);
    return TheProfileMgr.GetProfileFromPad(mPadNums[index]);
}

void ChooseProfilePanel::Text(
    int, int data, UIListLabel *uiListLabel, UILabel *uiLabel
) const {
    MILO_ASSERT_RANGE(data, 0, mPadNums.size(), 0x5d);
    static Symbol choose_save_data_sign_in("choose_save_data_sign_in");
    AppLabel *app_label = dynamic_cast<AppLabel *>(uiLabel);
    MILO_ASSERT(app_label, 0x61);
    int tag = mPadNums[data];
    if (uiListLabel->Matches("gamertag")) {
        if (data == -1) {
            // something with applabel
        } else {
            app_label->SetUserName(tag);
        }
    }
}

DataNode ChooseProfilePanel::OnMsg(SigninChangedMsg const &s) {
    UpdateProfiles();
    return DataNode(6);
}

BEGIN_HANDLERS(ChooseProfilePanel)
    HANDLE_EXPR(profile_selected, !mPadNums[_msg->Int(2)] == 0)
    HANDLE_EXPR(get_profile, GetProfile(_msg->Int(2)) == 0)
    HANDLE_ACTION(show_signin, ThePlatformMgr.SignInUsers(0, 0x1000000))
    HANDLE_ACTION(num_profiles, mPadNums.size())
    HANDLE_MESSAGE(SigninChangedMsg)
    HANDLE_SUPERCLASS(HamPanel)
END_HANDLERS
