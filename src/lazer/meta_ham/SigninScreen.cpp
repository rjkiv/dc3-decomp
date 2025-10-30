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
END_HANDLERS
