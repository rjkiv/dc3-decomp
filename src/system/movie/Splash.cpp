#include "movie/Splash.h"
#include "Splash.h"
#include "movie/Movie.h"
#include "movie/TexMovie.h"
#include "obj/Dir.h"
#include "obj/DirLoader.h"
#include "obj/Object.h"
#include "os/Archive.h"
#include "os/CritSec.h"
#include "os/Debug.h"
#include "os/OSFuncs.h"
#include "os/System.h"
#include "rndobj/Dir.h"
#include "rndobj/EventTrigger.h"
#include "rndobj/Movie.h"
#include "rndobj/Rnd.h"
#include "rndobj/Rnd_NG.h"
#include "rndobj/Utl.h"
#include "utl/FilePath.h"
#include "xdk/xapilibi/processthreadsapi.h"
#include "xdk/xapilibi/xbox.h"

bool gSplashing = false;
Splash *TheSplasher;

Splash::Splash()
    : mSplashTime(SystemConfig("ui")->FindFloat("splash_time") * 1000),
      mWaitForSplash(SystemConfig("ui")->FindInt("wait_for_splash")), unk48(0), unk4c(0),
      unk50(0), unk54(0), unk58(-1), mSuspendCount(0), unk64(1), mSplashThreadID(-1),
      mState() {}

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
            if (SetMutableState(kSuspending)) {
                WaitForState(kSuspended);
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
            WaitForState(kSuspended);
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
    ScreenParams next;
    {
        CritSecTracker tracker(&unk98);
        if (mScreens.empty()) {
            return false;
        }
        next = mScreens.front();
    }
    FilePath fp = next.fname;
    RndDir *dir = dynamic_cast<RndDir *>(DirLoader::LoadObjects(fp, nullptr, nullptr));
    MILO_ASSERT_FMT(dir, "Missing file %s", next.fname);
    TexMovie *splashMovie = dir->Find<TexMovie>(kSplashMovie, false);
    if (splashMovie) {
        Movie &movie = splashMovie->GetMovie();
        movie.CheckOpen(false);
    }
    PreparedScreenParams params;
    params.unk0 = dir;
    params.unk4 = next.msecs;
    {
        CritSecTracker tracker(&unk98);
        mPreparedScreens.push_back(params);
        mScreens.pop_front();
    }
    return true;
}

void Splash::PrepareRemaining() {
    while (PrepareNext())
        ;
}

unsigned long Splash::ThreadStart(void *v) {
    static_cast<Splash *>(v)->UpdateThread();
    return 0;
}

void SuspendFunc() { TheSplasher->Suspend(); }
void ResumeFunc() { TheSplasher->Resume(); }
void PollFunc() { TheSplasher->Poll(); }

void Splash::BeginSplasher() {
    if (unk64) {
        MILO_ASSERT(!gSplashing, 0x6B);
        gSplashing = true;
        MILO_ASSERT(!mPreparedScreens.empty(), 0x6D);
        MILO_ASSERT(SetMutableState(kResuming), 0x6F);
        HANDLE hThread = CreateThread(nullptr, 0, ThreadStart, this, 4, 0);
        XSetThreadProcessor(hThread, 5);
        SetThreadPriority(hThread, 1);
        ResumeThread(hThread);
        WaitForState(kResumed);
    } else {
        SetMutableState(kResumed);
        Show();
        Draw();
    }
    TheSplasher = this;
    SetRndSplasherCallback(PollFunc, SuspendFunc, ResumeFunc);
    TheRnd.SetUnk1b4(true);
}

void Splash::EndSplasher() {
    if (TheSplasher) {
        if (unk64) {
            MILO_ASSERT(mScreens.empty(), 0xA6);
            MILO_ASSERT(gSplashing, 0xA7);
            MILO_ASSERT(SetImmutableState(kTerminating), 0xA9);
            WaitForState(kTerminated);
            TheNgRnd.Resume();
            gSplashing = false;
        } else {
            while (ShowNext())
                ;
            MILO_ASSERT(SetImmutableState(kTerminated), 0xB6);
        }
        TheSplasher = nullptr;
        SetRndSplasherCallback(nullptr, nullptr, nullptr);
        TheRnd.SetUnk1b4(false);
        FOREACH (it, unkc0) {
            delete *it;
        }
        Movie::Validate();
    }
}

void Splash::Poll() {
    if ((!unk64 || mSuspendCount) && !gSplashing) {
        if (!UpdateThreadLoop()) {
            gSplashing = true;
            for (int i = 0; i < 2; i++) {
                TheRnd.BeginDrawing();
                TheRnd.EndDrawing();
            }
        }
    }
}

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
    MILO_ASSERT_FMT(unk64, "Can't WaitForState");
    while (mState != state && (state != kResumed || mState <= kResumed)) {
        MainThread() ? unk8c.Wait(-1) : unk90.Wait(-1);
    }
}

void Splash::CheckWorkerSuspend(bool b1) {
    MILO_ASSERT(!MainThread(), 0x1F0);
    while (mState == kSuspending) {
        TheNgRnd.Suspend();
        if (unk50) {
            unk50->SetShowing(false);
            unk50->GetMovie().UnlockThread();
        }
        {
            CritSecTracker tracker(&unk6c);
            MILO_ASSERT(mState == kSuspending, 0x1FF);
            mState = kSuspended;
            unk8c.Set();
        }
        WaitForState(kResuming);
        TheNgRnd.Resume();
        {
            CritSecTracker tracker(&unk6c);
            MILO_ASSERT(mState == kResuming, 0x209);
            mState = kResumed;
            unk8c.Set();
        }
        if (unk50) {
            unk50->SetShowing(true);
            unk50->GetMovie().LockThread();
        }
        if (b1) {
            unk5c = false;
            Draw();
        }
    }
}

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
    {
        CritSecTracker tracker(&unk98);
        if (mPreparedScreens.size() == 1) {
            return !mScreens.empty();
        }
        mPreparedScreens.pop_front();
    }
    return Show();
}

bool Splash::Show() {
    PreparedScreenParams params;
    {
        CritSecTracker tracker(&unk98);
        MILO_ASSERT(!mPreparedScreens.empty(), 0x283);
        params = mPreparedScreens.front();
    }
    unk48 = params.unk0;
    params.unk0->Enter();
    unk4c = unk48->Find<RndCam>(kSplashCam);
    unk50 = unk48->Find<TexMovie>(kSplashMovie, false);
    if (unk50) {
        if (unk64) {
            Movie &movie = unk50->GetMovie();
            unk50->SetShowing(true);
            movie.SetPaused(false);
            int time = ceilf(movie.MsPerFrame() * (float)movie.NumFrames());
            mSplashTime = time * 2;
        } else {
            return ShowNext();
        }
    } else {
        mSplashTime = params.unk4;
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
    if (unk18.SplitMs() > mSplashTime && !ShowNext()) {
        return false;
    } else {
        Draw();
        if (mState != kTerminating || mWaitForSplash) {
            return true;
        }
        while (ShowNext())
            ;
    }
    return false;
}

void Splash::UpdateThread() {
    mSplashThreadID = GetCurrentThreadId();
    MILO_ASSERT(!MainThread(), 0x21d);
    {
        CritSecTracker tracker(&unk6c);
        MILO_ASSERT(mState == kResuming, 0x221);
        mState = kResumed;
        unk8c.Set();
    }
    Timer timer;
    timer.Start();
    Show();
    while (UpdateThreadLoop()) {
        CheckWorkerSuspend(true);
    }
    MILO_ASSERT(mScreens.empty(), 0x23A);
    for (int i = 0; i < 2; i++) {
        TheRnd.BeginDrawing();
        TheRnd.EndDrawing();
    }
    while (!SetImmutableState(kWaitingForTerminating)) {
        MILO_ASSERT(mState == kSuspending, 0x246);
        CheckWorkerSuspend(false);
    }
    TheNgRnd.Suspend();
    float ms = timer.SplitMs();
    if (TheArchive && Archive::DebugArkOrder()) {
        MILO_LOG("Splash Time: %f\n", ms);
    }
    WaitForState(kTerminating);
    MILO_ASSERT(SetImmutableState(kTerminated), 0x257);
}
