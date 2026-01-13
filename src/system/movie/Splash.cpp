#include "movie/Splash.h"
#include "Splash.h"
#include "obj/Object.h"
#include "os/CritSec.h"
#include "os/Debug.h"
#include "os/System.h"
#include "rndobj/EventTrigger.h"
#include "rndobj/Movie.h"
#include "rndobj/Rnd_NG.h"

bool gSplashing = false;
Splash *TheSplasher;

Splash::Splash()
    : unk8(SystemConfig("ui")->FindArray("splash_time")->Float(1) * 1000),
      unkc(SystemConfig("ui")->FindArray("wait_for_splash")->Int(1)), unk48(0), unk4c(0),
      unk50(0), unk54(0), unk58(-1), unk60(0), unk64(1), unk68(-1), mState(0) {}

Splash::~Splash() { MILO_ASSERT(!gSplashing, 0x57); }

void Splash::SetWaitForSplash(bool b) {
    MILO_ASSERT(!gSplashing, 0x16e);
    unkc = b;
}

void Splash::Suspend() {
    MILO_ASSERT(MainThread(), 0xcf);
    unk60 += 1;
    if (unk60 < 2) {
        if (!unk64) {
            SetMutableState(SplashState::s2);
        }
        else {
            bool b = SetMutableState(SplashState::s1);
            if (b) {
                WaitForState(SplashState::s2);
                TheNgRnd.Suspend();
            }
        }
    }
}

void Splash::Resume() {}

void Splash::AddScreen(char const *c, int i) {
    MILO_ASSERT(!gSplashing, 0x175);
    ScreenParams sp;
    sp.fname = c;
    sp.msecs = i;
    CritSecTracker tracker(&unk98);
    mScreens.push_back(sp);
}

bool Splash::PrepareNext() {
    //CriticalSection *cs = &unk98;
    CritSecTracker tracker(&unk98);
    if (mScreens.empty()) {
        return false;
    }
    else {
        auto local58 = mScreens.back().fname;
        FilePath fp = local58;
        auto loadedObj = DirLoader::LoadObjects(fp, 0, 0);
        RndDir *rndDir = dynamic_cast<RndDir *>(loadedObj);
        if (!rndDir) {
            MILO_FAIL("Missing file %s", local58);
        }
        auto splashMovie = rndDir->Find<TexMovie>(kSplashMovie, false);
        if (splashMovie) {
            splashMovie->GetMovie().CheckOpen(false);
        }
        CritSecTracker tracker2(&unk98);
        PreparedScreenParams psp = {rndDir};
        mPreparedScreens.push_back(psp);
        mScreens.clear();
        return true;
    }
}

void Splash::PrepareRemaining() {
    for (bool b = PrepareNext(); b; b = PrepareNext()) {}
}

void Splash::EndSplasher() {}

void Splash::Poll() {
    if (!unk64 || unk60 && !gSplashing) {
        if (!UpdateThreadLoop()) {
            gSplashing = true;
            for (int i = 0; i < 2; i++) {
                TheRnd.BeginDrawing();
                TheRnd.EndDrawing();
            }
        }
    }
}

void Splash::BeginSplasher() {}

void Splash::Draw() {}

bool Splash::SetMutableState(Splash::SplashState state) {
    MILO_ASSERT(state <= kResumed, 0x13b);
    CritSecTracker tracker(&unk6c);
    if (mState <= kResumed) {
        mState = state;
        MainThread() ? unk90.Set() : unk8c.Set();
        return true;
    }
    else {
        return false;
    }
}

bool Splash::SetImmutableState(Splash::SplashState state) {
    MILO_ASSERT(state > kResumed, 0x150);
    CritSecTracker tracker(&unk6c);
    if (mState < kResumed || state <= mState) {
        if (state != kWaitingForTerminating || mState != kTerminating) {
            return false;
        }
    }
    else {
        mState = state;
        MainThread() ? unk90.Set() : unk8c.Set();
        return true;
    }
    return true;
}

void Splash::WaitForState(Splash::SplashState state) {
    if (unk64 == 0) {
        MILO_FAIL("Can\'t WaitForState");
    }
    while (mState != state && (state != kResumed || mState <= kResumed)) {
        MainThread() ? unk8c.Wait(-1) : unk90.Wait(-1);
    }
}

void Splash::CheckWorkerSuspend(bool) {}

bool Splash::ShowNext() {
    if (unk50) {
        unk50->SetShowing(false);
        unk50->GetMovie().SetPaused(true);
        unk50 = nullptr;
    }
    if (unk48) {
        unk48->Exit();
        unkc0.push_back(unk48);
        unk48 = nullptr;
    }
    unk4c = 0;
    unk54 = 0;
    CritSecTracker tracker(&unk98);
    FOREACH(it, mPreparedScreens) {
        // not really sure whats going on here
    }
    mPreparedScreens.clear();
    return Show();
}

bool Splash::Show() {
    CritSecTracker tracker(&unk98);
    MILO_ASSERT(!mPreparedScreens.empty(), 0x283);
    tracker.mCritSec->Exit();
    auto rndDir = mPreparedScreens.end()->unk0;
    rndDir->Exit();
    unk4c = unk48->Find<RndCam>(kSplashCam, true);
    unk50 = unk48->Find<TexMovie>(kSplashMovie, true);
    if (!unk50) {
        unk8 = mPreparedScreens.end()->unk4;
    }
    else {
        if (!unk64) {
            return ShowNext();
        }
        unk50->SetShowing(true);
        unk50->GetMovie().SetPaused(false);
        unk8 = ceil(unk50->GetMovie().MsPerFrame() * unk50->GetMovie().NumFrames());
    }
    unk54 = unk48->Find<EventTrigger>("splash.trig", false);
    if (unk54) {
        unk54->Trigger();
    }
    unk18.Restart();
    unk5c = false;
    return true;
}

bool Splash::UpdateThreadLoop() {
    if (unk18.SplitMs() <= unk8 || ShowNext()) {
        Draw();
        if (mState != kTerminating || unkc) {
            return true;
        }
        for (bool b = ShowNext(); b; b = ShowNext()) {}
    }
    return false;
}

void Splash::UpdateThread() {
    DWORD threadID = GetCurrentThreadId();
    MILO_ASSERT(!MainThread(), 0x21d);
    CritSecTracker tracker(&unk6c);
    MILO_ASSERT(mState == kResuming, 0x221);
    mState = kResumed;
    unk8c.Set();
    unk18.Start();
    Show();
    bool b = UpdateThreadLoop();
    while (b) {
        CheckWorkerSuspend(b);
    }
}

unsigned long Splash::ThreadStart(void *v) {
    static_cast<Splash *>(v)->UpdateThread();
    return 0;
}

void SuspendFunc() {
    TheSplasher->Suspend();
}

void ResumeFunc() {}

void PollFunc() { TheSplasher->Poll(); }