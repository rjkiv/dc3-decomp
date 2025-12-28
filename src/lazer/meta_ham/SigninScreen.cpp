#include "lazer/meta_ham/SigninScreen.h"
#include "meta_ham/HamScreen.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/PlatformMgr.h"
#include "ui/UI.h"
#include "ui/UIScreen.h"
#include "utl/Symbol.h"

SigninScreen::SigninScreen() {}

SigninScreen::~SigninScreen() {}

void SigninScreen::Poll() { UIScreen::Poll(); }

void SigninScreen::Enter(UIScreen *screen) {
    HamScreen::Enter(screen);
    ThePlatformMgr.AddSink(this);
}

void SigninScreen::Exit(UIScreen *screen) {
    ThePlatformMgr.RemoveSink(this);
    HamScreen::Exit(screen);
}

DataNode SigninScreen::OnMsg(SigninChangedMsg const &) { return NULL_OBJ; }

DataNode SigninScreen::OnMsg(UIChangedMsg const &) { return NULL_OBJ; }

BEGIN_HANDLERS(SigninScreen)
    HANDLE_ACTION(show_signin_ui, ThePlatformMgr.SignInUsers(1, 0x1000000))
    HANDLE_MESSAGE(SigninChangedMsg)
    HANDLE_MESSAGE(UIChangedMsg)
    HANDLE_SUPERCLASS(HamScreen)
END_HANDLERS
