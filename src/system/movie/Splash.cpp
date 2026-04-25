#include "movie/Splash.h"
#include "Splash.h"
#include "obj/Object.h"
#include "os/CritSec.h"
#include "os/Debug.h"
#include "os/OSFuncs.h"
#include "os/System.h"
#include "rndobj/EventTrigger.h"
#include "rndobj/Movie.h"
#include "rndobj/Rnd_NG.h"

bool gSplashing = false;
Splash *TheSplasher;

Splash::Splash()
    : mSplashTime(SystemConfig("ui")->FindFloat("splash_time") * 1000),
      mWaitForSplash(SystemConfig("ui")->FindInt("wait_for_splash")), unk48(0), unk4c(0),
      unk50(0), unk54(0), unk58(-1), mSuspendCount(0), unk64(1), mSplashThreadID(-1),
      mState(0) {}

Splash::~Splash() { MILO_ASSERT(!gSplashing, 0x57); }

void Splash::SetWaitForSplash(bool wait) {
    MILO_ASSERT(!gSplashing, 0x16e);
    mWaitForSplash = wait;
}

void Splash::Suspend() {
    MILO_ASSERT(MainThread(), 0xcf);
    mSuspendCount++;
    if (mSuspendCount <= 1) {
        if (unk64) {
            if (SetMutableState(SplashState::s1)) {
                WaitForState(SplashState::s2);
                TheNgRnd.Resume();
                if (unk50) {
                    unk50->SetShowing(true);
                    unk50->GetMovie().LockThread();
                }
                unk5c = false;
                Draw();
            } else {
                MILO_ASSERT(mState == kWaitingForTerminating, 0xEB);
                TheNgRnd.Resume();
                if (unk50) {
                    unk50->SetShowing(true);
                    unk50->GetMovie().LockThread();
                }
            }
        } else {
            WaitForState(SplashState::s2);
        }
        unk200.Reset();
    }
}

void Splash::Resume() {
    MILO_ASSERT(MainThread(), 0x106);
    mSuspendCount--;
    if (mSuspendCount <= 0) {
        MILO_ASSERT(mSuspendCount == 0, 0x10D);
        if (unk64) {
            if (SetMutableState(s3)) {
                if (unk50) {
                    unk50->SetShowing(false);
                    unk50->GetMovie().UnlockThread();
                }
                TheNgRnd.Suspend();
                MILO_ASSERT(SetMutableState(kResuming), 0x11C);
                WaitForState(kResumed);
            } else {
                MILO_ASSERT(mState == kWaitingForTerminating, 0x122);
                if (unk50) {
                    unk50->SetShowing(false);
                    unk50->GetMovie().UnlockThread();
                }
                TheNgRnd.Suspend();
            }
        } else if (SetMutableState(kResumed)) {
            unk5c = false;
            Draw();
        }
    }
}

void Splash::AddScreen(char const *c, int i) {
    MILO_ASSERT(!gSplashing, 0x175);
    ScreenParams sp;
    sp.fname = c;
    sp.msecs = i;
    CritSecTracker tracker(&unk98);
    mScreens.push_back(sp);
}

bool Splash::PrepareNext() {
    // CriticalSection *cs = &unk98;
    CritSecTracker tracker(&unk98);
    if (mScreens.empty()) {
        return false;
    } else {
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
        PreparedScreenParams psp = { rndDir };
        mPreparedScreens.push_back(psp);
        mScreens.clear();
        return true;
    }
}

// void Splash::PrepareRemaining() {
//     for (bool b = PrepareNext(); b; b = PrepareNext()) {
//     }
// }

// void Splash::EndSplasher() {}

void Splash::Poll() {
    if (!unk64 || mSuspendCount && !gSplashing) {
        if (!UpdateThreadLoop()) {
            gSplashing = true;
            for (int i = 0; i < 2; i++) {
                TheRnd.BeginDrawing();
                TheRnd.EndDrawing();
            }
        }
    }
}

// void Splash::BeginSplasher() {}

// void Splash::Draw() {}

bool Splash::SetMutableState(Splash::SplashState state) {
    MILO_ASSERT(state <= kResumed, 0x13b);
    CritSecTracker tracker(&unk6c);
    if (mState <= kResumed) {
        mState = state;
        MainThread() ? unk90.Set() : unk8c.Set();
        return true;
    } else {
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
    } else {
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

// void Splash::CheckWorkerSuspend(bool) {}

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
    FOREACH (it, mPreparedScreens) {
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
        mSplashTime = mPreparedScreens.end()->unk4;
    } else {
        if (!unk64) {
            return ShowNext();
        }
        unk50->SetShowing(true);
        unk50->GetMovie().SetPaused(false);
        mSplashTime =
            ceil(unk50->GetMovie().MsPerFrame() * unk50->GetMovie().NumFrames());
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
    if (unk18.SplitMs() <= mSplashTime || ShowNext()) {
        Draw();
        if (mState != kTerminating || mWaitForSplash) {
            return true;
        }
        for (bool b = ShowNext(); b; b = ShowNext()) {
        }
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

void SuspendFunc() { TheSplasher->Suspend(); }

// void ResumeFunc() {}

void PollFunc() { TheSplasher->Poll(); }
