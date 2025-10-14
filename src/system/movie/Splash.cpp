#include "movie/Splash.h"
#include "Splash.h"
#include "obj/Object.h"
#include "os/CritSec.h"
#include "os/Debug.h"
#include "os/System.h"

bool gSplashing = false;
Splash *TheSplasher;

Splash::Splash()
    : unk8(SystemConfig("ui")->FindArray("splash_time")->Float(1) * 1000),
      unkc(SystemConfig("ui")->FindArray("wait_for_splash")->Int(1)), unk48(0), unk4c(0),
      unk50(0), unk54(0), unk58(-1), unk60(0), unk64(1), unk68(-1), unk94(0) {}

Splash::~Splash() { MILO_ASSERT(!gSplashing, 0x57); }

void Splash::SetWaitForSplash(bool b) {
    MILO_ASSERT(!gSplashing, 0x16e);
    unkc = b;
}

void Splash::Suspend() {}

void Splash::Resume() {}

void Splash::AddScreen(char const *c, int i) {
    MILO_ASSERT(!gSplashing, 0x175);
    CriticalSection *cs = &unk98;
    ScreenParams sp;
    sp.fname = c;
    sp.msecs = i;
    if (cs != nullptr) {
        cs->Enter();
    }
    unk10.push_back(sp);
    if (cs != nullptr)
        cs->Exit();
}

bool Splash::PrepareNext() { return false; }

void Splash::PrepareRemaining() {}

void Splash::EndSplasher() {}

void Splash::Poll() {}

void Splash::BeginSplasher() {}

void Splash::Draw() {}

bool Splash::SetMutableState(Splash::SplashState) { return false; }

bool Splash::SetImmutableState(Splash::SplashState) { return false; }

bool Splash::WaitForState(Splash::SplashState) { return false; }

void Splash::CheckWorkerSuspend(bool) {}

bool Splash::ShowNext() { return false; }

bool Splash::Show() { return false; }

bool Splash::UpdateThreadLoop() { return false; }

void Splash::UpdateThread() {}

unsigned long Splash::ThreadStart(void *) { return 1; }

void SuspendFunc() {}

void ResumeFunc() {}

void PollFunc() { TheSplasher->Poll(); }
