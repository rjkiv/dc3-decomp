#include "os/PlatformMgr.h"
#include "obj/Data.h"
#include "obj/DataUtl.h"

#include "obj/Object.h"
#include "os/Debug.h"
#include "os/System.h"
#include "os/User.h"
#include "os/UserMgr.h"
#include "utl/Option.h"
#include "utl/UTF8.h"
#include "utl/Str.h"
#include "xdk/XAPILIB.h"

PlatformMgr ThePlatformMgr;

bool PlatformMgr::IsAnyUserSignedIntoLive() const {
    for (int i = 0; i < 4; i++) {
        if (IsSignedIntoLive(i))
            return true;
    }
    return false;
}

PlatformRegion PlatformMgr::GetRegion() const { return mRegion; }

Symbol PlatformRegionToSymbol(PlatformRegion r) {
    MILO_ASSERT(r < kNumRegions, 0x29);
    static Symbol sym[4] = { "", "na", "europe", "japan" };
    return sym[r];
}

PlatformRegion SymbolToPlatformRegion(Symbol s) {
    for (int i = 0; i < 4; i++) {
        PlatformRegion r = (PlatformRegion)i;
        if (PlatformRegionToSymbol(r) == s)
            return r;
    }
    MILO_FAIL("Invalid region %s", s);
    return kNumRegions;
}

void UTF8FilterKeyboardString(char *c, int i, const char *cc) {
    static Symbol platform_mgr("platform_mgr");
    static Symbol keyboard_allowed_chars("keyboard_allowed_chars");
    static const char *allowed =
        SystemConfig(platform_mgr)->FindStr(keyboard_allowed_chars);
    UTF8FilterString(c, i, cc, allowed, '?');
}

bool PlatformMgr::IsSignedIn(int padnum) const {
    if (padnum < 0) {
        MILO_FAIL("PadNum = %d", padnum);
    }
    return 1 << padnum & mSigninMask;
}

bool PlatformMgr::IsUserSignedIn(const LocalUser *pUser) const {
    MILO_ASSERT(pUser, 0x51);
    return IsSignedIn(pUser->GetPadNum());
}

bool PlatformMgr::IsPadNumSignedIn(int padnum) const { return IsSignedIn(padnum); }

bool PlatformMgr::HasPadNumsSigninChanged(int padnum) const {
    if (padnum < 0) {
        MILO_FAIL("PadNum = %d", padnum);
    }
    return 1 << padnum & mSigninChangeMask;
}

bool PlatformMgr::HasUserSigninChanged(const LocalUser *pUser) const {
    MILO_ASSERT(pUser, 99);
    return HasPadNumsSigninChanged(pUser->GetPadNum());
}

bool PlatformMgr::IsUserSignedIntoLive(const LocalUser *pUser) const {
    MILO_ASSERT(pUser, 0x6A);
    return IsSignedIntoLive(pUser->GetPadNum());
}

bool PlatformMgr::UserHasOnlinePrivilege(const LocalUser *pUser) const {
    MILO_ASSERT(pUser, 0x7A);
    return HasOnlinePrivilege(pUser->GetPadNum());
}

bool PlatformMgr::IsUserAGuest(const LocalUser *pUser) const {
    MILO_ASSERT(pUser, 0x87);
    int padnum = pUser->GetPadNum();
    if (padnum < 0) {
        MILO_FAIL("PadNum = %d", padnum);
    }
    return IsPadAGuest(padnum);
}

void PlatformMgr::ShowUserFriendsUI(const LocalUser *pUser) {
    MILO_ASSERT(pUser, 0xA6);
    ShowFriendsUI(pUser->GetPadNum());
}

void PlatformMgr::ShowOfferUI(const LocalUser *user) {
    MILO_ASSERT(user, 0xAC);
    ShowOfferUI(user->GetPadNum());
}

bool PlatformMgr::ShowUserPartyUI(const LocalUser *pUser) {
    MILO_ASSERT(pUser, 0xB2);
    return ShowPartyUI(pUser->GetPadNum());
}

void PlatformMgr::InviteUserParty(const LocalUser *pUser) {
    MILO_ASSERT(pUser, 0xB8);
    InviteParty(pUser->GetPadNum());
}

LocalUser *PlatformMgr::GetOwnerUserOfGuestUser(LocalUser *pUser) {
    MILO_ASSERT(pUser, 0xBE);
    return TheUserMgr->GetLocalUserFromPadNum(GetOwnerOfGuest(pUser->GetPadNum()));
}

void PlatformMgr::SetRegion(PlatformRegion region) {
    const char *regionStr = OptionStr("region", nullptr);
    if (regionStr) {
        mRegion = SymbolToPlatformRegion(regionStr);
    } else {
        mRegion = region;
    }
    MILO_ASSERT(mRegion != kRegionNone, 0xCC);
    String str(PlatformRegionToSymbol(mRegion));
    str.ToUpper();
    str = MakeString("REGION_%s", str);
    DataArrayPtr ptr(1);
    DataSetMacro(str.c_str(), ptr);
}

void PlatformMgr::SetDiskError(DiskError derr) {
    if (mDiskError != kFailedChecksum && mDiskError != derr) {
        mDiskError = derr;
        if (mDiskError != kNoDiskError) {
            static DiskErrorMsg msg;
            Handle(msg, false);
        }
        DiscErrorCallbackFunc *func = GetDiskErrorCallback();
        if (func)
            func();
        while (true) {
            MILO_LOG("DISK ERROR\n");
            Sleep(1);
        }
    }
}

void PlatformMgr::DebugFakeSigninChangeMsg(int padnum) {
    if (padnum < 0) {
        MILO_FAIL("PadNum = %d", padnum);
    }
    mSigninChangeMask = 1 << padnum;
    static SigninChangedMsg msg(0, 0);
    msg[0] = mSigninMask;
    msg[1] = mSigninChangeMask;
    Handle(msg, false);
}

BEGIN_HANDLERS(PlatformMgr)
    HANDLE_EXPR(is_connected, mConnected)
    HANDLE_EXPR(is_ethernet_cable_connected, IsEthernetCableConnected())
    HANDLE(signin, OnSignInUsers)
    HANDLE_EXPR(get_signin_mask, mSigninMask)
    HANDLE_EXPR(get_signin_changed_mask, mSigninChangeMask)
    HANDLE_EXPR(is_user_signed_in, IsUserSignedIn(_msg->Obj<LocalUser>(2)))
    HANDLE_EXPR(is_pad_signed_in, IsSignedIn(_msg->Int(2)))
    HANDLE_EXPR(has_user_signin_changed, HasUserSigninChanged(_msg->Obj<LocalUser>(2)))
    HANDLE_EXPR(is_guide_showing, mGuideShowing)
    HANDLE_EXPR(get_name, GetName(_msg->Int(2)))
    HANDLE_EXPR(is_user_signed_into_live, IsUserSignedIntoLive(_msg->Obj<LocalUser>(2)))
    HANDLE_EXPR(is_pad_signed_into_live, IsSignedIntoLive(_msg->Int(2)))
    HANDLE_EXPR(has_online_privilege, HasOnlinePrivilege(_msg->Int(2)))
    HANDLE_EXPR(has_created_content_privilege, HasCreatedContentPrivilege())
    HANDLE_EXPR(user_has_online_privilege, UserHasOnlinePrivilege(_msg->Obj<LocalUser>(2)))
    HANDLE_EXPR(has_kinectshare_privilege, HasKinectSharePrvilege())
    HANDLE_EXPR(guide_showing, mGuideShowing)
    HANDLE_EXPR(is_user_a_guest, IsUserAGuest(_msg->Obj<LocalUser>(2)))
    HANDLE_EXPR(is_pad_a_guest, IsPadAGuest(_msg->Int(2)))
    HANDLE_EXPR(
        get_owner_user_of_guest_user, GetOwnerUserOfGuestUser(_msg->Obj<LocalUser>(2))
    )
    HANDLE_ACTION(show_user_friends, ShowUserFriendsUI(_msg->Obj<LocalUser>(2)))
    HANDLE_ACTION(
        show_controller_required, ShowControllerRequiredUI(_msg->Obj<Hmx::Object>(2))
    )
    HANDLE_EXPR(show_user_party, ShowUserPartyUI(_msg->Obj<LocalUser>(2)))
    HANDLE_EXPR(is_in_party, IsInParty())
    HANDLE_EXPR(is_in_party_with_others, IsInPartyWithOthers())
    HANDLE_ACTION(invite_user_party, InviteUserParty(_msg->Obj<LocalUser>(2)))
    HANDLE_EXPR(show_fitness_body_profile, ShowFitnessBodyProfileUI(_msg->Int(2)))
    HANDLE_ACTION(show_offer_ui, ShowOfferUI(_msg->Obj<LocalUser>(2)))
    HANDLE_ACTION(
        set_background_download_priority, SetBackgroundDownloadPriority(_msg->Int(2))
    )
    HANDLE_EXPR(is_confirm_cancel_swapped, mConfirmCancelSwapped)
    HANDLE_ACTION(disable_xmp, DisableXMP())
    HANDLE_ACTION(enable_xmp, EnableXMP())
    HANDLE_ACTION(set_screen_saver, SetScreenSaver(_msg->Int(2)))
    HANDLE_ACTION(check_mailbox, CheckMailbox())
    HANDLE_ACTION(run_net_start_utility, RunNetStartUtility())
    HANDLE_ACTION(
        set_notify_ui_location, SetNotifyUILocation((NotifyLocation)_msg->Int(2))
    )
    HANDLE_EXPR(get_region, GetRegion())
    HANDLE_EXPR(is_any_user_signed_in_to_live, IsAnyUserSignedIntoLive())
    HANDLE_EXPR(has_xsocial_photo_post, mHasXSocialPhotoPost)
    HANDLE_EXPR(has_xsocial_link_post, mHasXSocialLinkPost)
    HANDLE_EXPR(poll_xsocial_capabilities, PollXSocialCapabilities())
    HANDLE_EXPR(query_xsocial_capabilities, QueryXSocialCapabilities())
    HANDLE_ACTION(smart_glass_send, SmartGlassSend(_msg->Int(2), _msg->Array(3)))
    HANDLE_EXPR(is_smart_glass_connected, IsSmartGlassConnected())
    HANDLE_ACTION(trigger_disk_error, SetDiskError(kFailedChecksum))
    HANDLE_ACTION(debug_fake_signin_change_msg, DebugFakeSigninChangeMsg(_msg->Int(2)))
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS
