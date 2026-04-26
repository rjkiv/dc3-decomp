#pragma once
#include "movie/TexMovie.h"
#include "os/CritSec.h"
#include "os/SynchronizationEvent.h"
#include "rndobj/Dir.h"
#include "rndobj/EventTrigger.h"

class Splash {
public:
    enum SplashState {
        s0,
        kSuspending = 1,
        kSuspended = 2,
        s3,
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
    Timer unk18;
    RndDir *unk48;
    RndCam *unk4c;
    TexMovie *unk50;
    EventTrigger *unk54;
    int unk58;
    bool unk5c;
    int mSuspendCount; // 0x60
    bool unk64;
    DWORD mSplashThreadID; // 0x68
    CriticalSection unk6c;
    SynchronizationEvent unk8c; // 0x8c
    SynchronizationEvent unk90; // 0x90
    SplashState mState; // 0x94
    CriticalSection unk98;
    std::list<PreparedScreenParams> mPreparedScreens; // 0xb8
    std::list<RndDir *> unkc0;
    Timer unk200;
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
