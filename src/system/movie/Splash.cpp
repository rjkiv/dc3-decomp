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
      mWaitForSplash(SystemConfig("ui")->FindInt("wait_for_splash")), mActiveSplash(0), mActiveCam(0),
      mActiveMovie(0), mActiveTrigger(0), mLastMovieFrame(-1), mSuspendCount(0), mThreadedSplash(1), mSplashThreadID(-1),
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
        if (mThreadedSplash) {
            if (SetMutableState(kSuspending)) {
                WaitForState(kSuspended);
                TheNgRnd.Resume();
                if (mActiveMovie) {
                    mActiveMovie->SetShowing(true);
                    mActiveMovie->GetMovie().LockThread();
                }
                mDrawn = false;
                Draw();
            } else {
                MILO_ASSERT(mState == kWaitingForTerminating, 0xEB);
                TheNgRnd.Resume();
                if (mActiveMovie) {
                    mActiveMovie->SetShowing(true);
                    mActiveMovie->GetMovie().LockThread();
                }
            }
        } else {
            WaitForState(kSuspended);
        }
        mMainThreadRedraw.Reset();
    }
}

void Splash::Resume() {
    MILO_ASSERT(MainThread(), 0x106);
    mSuspendCount--;
    if (mSuspendCount <= 0) {
        MILO_ASSERT(mSuspendCount == 0, 0x10D);
        if (mThreadedSplash) {
            if (SetMutableState(kResumingTest)) {
                if (mActiveMovie) {
                    mActiveMovie->SetShowing(false);
                    mActiveMovie->GetMovie().UnlockThread();
                }
                TheNgRnd.Suspend();
                MILO_ASSERT(SetMutableState(kResuming), 0x11C);
                WaitForState(kResumed);
            } else {
                MILO_ASSERT(mState == kWaitingForTerminating, 0x122);
                if (mActiveMovie) {
                    mActiveMovie->SetShowing(false);
                    mActiveMovie->GetMovie().UnlockThread();
                }
                TheNgRnd.Suspend();
            }
        } else if (SetMutableState(kResumed)) {
            mDrawn = false;
            Draw();
        }
    }
}

void Splash::AddScreen(char const *c, int i) {
    MILO_ASSERT(!gSplashing, 0x175);
    ScreenParams sp;
    sp.fname = c;
    sp.msecs = i;
    CritSecTracker tracker(&mPrepareCrit);
    mScreens.push_back(sp);
}

bool Splash::PrepareNext() {
    ScreenParams next;
    {
        CritSecTracker tracker(&mPrepareCrit);
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
        CritSecTracker tracker(&mPrepareCrit);
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
    if (mThreadedSplash) {
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
    TheRnd.SetSplashing(true);
}

void Splash::EndSplasher() {
    if (TheSplasher) {
        if (mThreadedSplash) {
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
        TheRnd.SetSplashing(false);
        FOREACH (it, mPastScreens) {
            delete *it;
        }
        Movie::Validate();
    }
}

void Splash::Poll() {
    if ((!mThreadedSplash || mSuspendCount) && !gSplashing) {
        if (!UpdateThreadLoop()) {
            gSplashing = true;
            for (int i = 0; i < 2; i++) {
                TheRnd.BeginDrawing();
                TheRnd.EndDrawing();
            }
        }
    }
}

void Splash::Draw() {
    if (mTimer.SplitMs() <= mSplashTime) {
        if (!mDrawn || mActiveMovie != nullptr || mActiveTrigger != nullptr) {
            if (mActiveTrigger != nullptr) {
                TheTaskMgr.Poll();
                mActiveSplash->Poll();
            }
            if (mActiveMovie != nullptr) {
                if (MainThread()) {
                    float allowance = mActiveMovie->GetMovie().MsPerFrame() - 1.0f;
                    if (mMainThreadRedraw.Running() != 0 && mMainThreadRedraw.SplitMs() < allowance) {
                        return;
                    }
                    mMainThreadRedraw.Restart();
                }
                if (mActiveMovie->GetMovie().Poll() == false) {
                    mSplashTime = 0;
                    return;
                }
            }
            for (int i = 0; i < 2; i++) {
                TheRnd.BeginDrawing();
                mActiveCam->Select();
                mActiveSplash->DrawShowing();
                TheRnd.EndDrawing();
                if (mActiveMovie != nullptr)
                    break;
                if (mActiveTrigger != nullptr)
                    break;
            }
            if (mActiveMovie == nullptr && mActiveTrigger == nullptr) {
                TheNgRnd.Suspend();
            }
            mDrawn = true;
        }
        int synctime = 0;
        if (!MainThread()) {
            synctime = mSplashTime - int(mTimer.SplitMs());
            if (mActiveMovie != nullptr) {
                float allowance = mActiveMovie->GetMovie().MsPerFrame() - 1.0f;
                if (int(allowance) < synctime) {
                    synctime = allowance;
                }
                if (synctime < 0) {
                    synctime = 0;
                }
            } else if (mActiveTrigger != nullptr) {
                synctime = 16;
            }
            unk90.Wait(synctime);
        }
    }
}

bool Splash::SetMutableState(Splash::SplashState state) {
    MILO_ASSERT(state <= kResumed, 0x13b);
    CritSecTracker tracker(&mStateCrit);
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
    CritSecTracker tracker(&mStateCrit);
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
    MILO_ASSERT_FMT(mThreadedSplash, "Can't WaitForState");
    while (mState != state
           && (state != kResumed || (volatile SplashState)mState <= kResumed)) {
        MainThread() ? unk8c.Wait(-1) : unk90.Wait(-1);
    }
}

void Splash::CheckWorkerSuspend(bool b1) {
    MILO_ASSERT(!MainThread(), 0x1F0);
    while (mState == kSuspending) {
        TheNgRnd.Suspend();
        if (mActiveMovie) {
            mActiveMovie->SetShowing(false);
            mActiveMovie->GetMovie().UnlockThread();
        }
        {
            CritSecTracker tracker(&mStateCrit);
            MILO_ASSERT(mState == kSuspending, 0x1FF);
            mState = kSuspended;
            unk8c.Set();
        }
        WaitForState(kResuming);
        TheNgRnd.Resume();
        {
            CritSecTracker tracker(&mStateCrit);
            MILO_ASSERT(mState == kResuming, 0x209);
            mState = kResumed;
            unk8c.Set();
        }
        if (mActiveMovie) {
            mActiveMovie->SetShowing(true);
            mActiveMovie->GetMovie().LockThread();
        }
        if (b1) {
            mDrawn = false;
            Draw();
        }
    }
}

bool Splash::ShowNext() {
    if (mActiveMovie) {
        mActiveMovie->SetShowing(false);
        mActiveMovie->GetMovie().SetPaused(true);
        mActiveMovie = nullptr;
    }
    if (mActiveSplash) {
        mActiveSplash->Exit();
        mPastScreens.push_back(mActiveSplash);
        mActiveSplash = nullptr;
    }
    mActiveCam = 0;
    mActiveTrigger = 0;
    {
        CritSecTracker tracker(&mPrepareCrit);
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
        CritSecTracker tracker(&mPrepareCrit);
        MILO_ASSERT(!mPreparedScreens.empty(), 0x283);
        params = mPreparedScreens.front();
    }
    mActiveSplash = params.unk0;
    params.unk0->Enter();
    mActiveCam = mActiveSplash->Find<RndCam>(kSplashCam);
    mActiveMovie = mActiveSplash->Find<TexMovie>(kSplashMovie, false);
    if (mActiveMovie) {
        if (mThreadedSplash) {
            Movie &movie = mActiveMovie->GetMovie();
            mActiveMovie->SetShowing(true);
            movie.SetPaused(false);
            int time = ceilf(movie.MsPerFrame() * (float)movie.NumFrames());
            mSplashTime = time * 2;
        } else {
            return ShowNext();
        }
    } else {
        mSplashTime = params.unk4;
    }
    mActiveTrigger = mActiveSplash->Find<EventTrigger>("splash.trig", false);
    if (mActiveTrigger) {
        mActiveTrigger->Trigger();
    }
    mTimer.Restart();
    mDrawn = false;
    return true;
}

bool Splash::UpdateThreadLoop() {
    if (mTimer.SplitMs() > mSplashTime && !ShowNext()) {
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
        CritSecTracker tracker(&mStateCrit);
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
