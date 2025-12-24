#include "lazer/meta_ham/SigninScreen.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/PlatformMgr.h"
#include "ui/UI.h"
#include "ui/UIScreen.h"
#include "utl/Symbol.h"

SigninScreen::SigninScreen() {}

void SigninScreen::Poll() { UIScreen::Poll(); }

void SigninScreen::Enter(UIScreen *) {
    ThePlatformMgr.AddSink(this, gNullStr, gNullStr, kHandle, true);
}

void SigninScreen::Exit(UIScreen *) { ThePlatformMgr.RemoveSink(this, gNullStr); }

DataNode SigninScreen::OnMsg(SigninChangedMsg const &) { return NULL_OBJ; }

DataNode SigninScreen::OnMsg(UIChangedMsg const &) { return NULL_OBJ; }

BEGIN_HANDLERS(SigninScreen)
    HANDLE_ACTION(show_signin_ui, ThePlatformMgr.SignInUsers(1, 0x100000))
    HANDLE_MESSAGE(SigninChangedMsg)
    HANDLE_MESSAGE(UIChangedMsg)
    // HANDLE_SUPERCLASS(HamScreen)
END_HANDLERS
