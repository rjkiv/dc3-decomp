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
        s1,
        s2,
        s3,
        kResuming,
        kResumed,
        kWaitingForTerminating,
        kTerminating,
        kTerminated
    };

    struct ScreenParams {
        const char *fname;
        int msecs;
    };

    struct PreparedScreenParams {
        RndDir *unk0;
        int unk4;
    };
    virtual ~Splash();

    Splash();
    void SetWaitForSplash(bool);
    void Suspend();
    void Resume();
    void AddScreen(char const *, int);
    bool PrepareNext();
    void PrepareRemaining();
    void EndSplasher();
    void Poll();
    void BeginSplasher();
    DWORD SplashThreadId() const { return unk68; }

    float unk8;
    bool unkc;
    std::list<ScreenParams> mScreens; // 0x10
    Timer unk18;
    RndDir *unk48;
    RndCam *unk4c;
    TexMovie *unk50;
    EventTrigger *unk54;
    int unk58;
    u32 unk5c;
    int unk60;
    bool unk64;
    DWORD unk68;
    CriticalSection unk6c;
    SynchronizationEvent unk8c;
    SynchronizationEvent unk90;
    int mState; // 0x94
    CriticalSection unk98;
    std::list<PreparedScreenParams> mPreparedScreens;
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
    static unsigned long ThreadStart(void *);
};

extern Splash *TheSplasher;

void SuspendFunc();
void ResumeFunc();
void PollFunc();

const char *kSplashMovie = "s_splash.tmov";
const char *kSplashCam = "s_splash.cam";