#pragma once
#include "movie/TexMovie.h"
#include "os/CritSec.h"
#include "os/SynchronizationEvent.h"
#include "rndobj/Dir.h"
#include "rndobj/EventTrigger.h"

class Splash {
public:
    enum SplashState {
        kInvalid,
        kSuspending = 1,
        kSuspended = 2,
        kResumingTest,
        kResuming = 4,
        kResumed = 5,
        kWaitingForTerminating = 6,
        kTerminating = 7,
        kTerminated = 8
    };

    struct ScreenParams {
        const char *fname; // 0x0 - file name
        int msecs;
    };

    struct PreparedScreenParams {
        RndDir *unk0; // 0x0 - object dir
        int unk4; // 0x4
    };

    Splash();
    virtual ~Splash();

    void SetWaitForSplash(bool);
    void Suspend();
    void Resume();
    void AddScreen(char const *, int);
    bool PrepareNext();
    void PrepareRemaining();
    void EndSplasher();
    void Poll();
    void BeginSplasher();
    DWORD SplashThreadId() const { return mSplashThreadID; }

    int mSplashTime; // 0x8
    bool mWaitForSplash; // 0xc
    std::list<ScreenParams> mScreens; // 0x10
    Timer mTimer; // 0x18
    RndDir *mActiveSplash; // 0x48
    RndCam *mActiveCam; // 0x4C
    TexMovie *mActiveMovie; // 0x50
    EventTrigger *mActiveTrigger; // 0x54
    int mLastMovieFrame; // 0x58
    bool mDrawn; // 0x5C
    int mSuspendCount; // 0x60
    bool /* const */ mThreadedSplash; // 0x64
    DWORD mSplashThreadID; // 0x68
    CriticalSection mStateCrit; // 0x6C
    SynchronizationEvent unk8c; // 0x8c
    SynchronizationEvent unk90; // 0x90
    SplashState mState; // 0x94
    CriticalSection mPrepareCrit; // 0x98
    std::list<PreparedScreenParams> mPreparedScreens; // 0xb8
    std::list<RndDir *> mPastScreens;
    Timer mMainThreadRedraw; // 0xC8
    void *mThreadStack;

protected:
    virtual void Draw();

    bool SetMutableState(Splash::SplashState);
    bool SetImmutableState(Splash::SplashState);
    void WaitForState(Splash::SplashState);
    void CheckWorkerSuspend(bool);
    bool ShowNext();
    bool Show();
    bool UpdateThreadLoop();
    void UpdateThread();

    static DWORD ThreadStart(void *);
};

extern Splash *TheSplasher;

void SuspendFunc();
void ResumeFunc();
void PollFunc();

const char *kSplashMovie = "s_splash.tmov";
const char *kSplashCam = "s_splash.cam";
