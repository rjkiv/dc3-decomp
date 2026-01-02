#include "lazer/meta_ham/SigninScreen.h"
#include "meta_ham/HamScreen.h"
#include "obj/Data.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "os/PlatformMgr.h"
#include "ui/UI.h"
#include "ui/UIScreen.h"
#include "utl/Symbol.h"

SigninScreen::SigninScreen() {}

BEGIN_HANDLERS(SigninScreen)
    HANDLE_ACTION(show_signin_ui, ThePlatformMgr.SignInUsers(1, 0x1000000))
    HANDLE_MESSAGE(SigninChangedMsg)
    HANDLE_MESSAGE(UIChangedMsg)
    HANDLE_SUPERCLASS(HamScreen)
END_HANDLERS

void SigninScreen::Poll() { UIScreen::Poll(); }

void SigninScreen::Enter(UIScreen *screen) {
    HamScreen::Enter(screen);
    ThePlatformMgr.AddSink(this);
}

void SigninScreen::Exit(UIScreen *screen) {
    ThePlatformMgr.RemoveSink(this);
    HamScreen::Exit(screen);
}

DataNode SigninScreen::OnMsg(const SigninChangedMsg &msg) {
    int mask = ThePlatformMgr.SignInMask();
    for (int i = 0; mask != 0; mask >>= 1, i++) {
        if ((mask & 1) && !ThePlatformMgr.IsPadAGuest(i)) {
            static Message msg("on_signed_in");
            Handle(msg, true);
            break;
        }
    }
    return 0;
}

DataNode SigninScreen::OnMsg(const UIChangedMsg &msg) {
    if (!msg.Showing() && ThePlatformMgr.SignInMask() == 0) {
        static Message sign_in_dismissed("sign_in_dismissed", 0);
        Handle(sign_in_dismissed, false);
    }
    return 0;
}
