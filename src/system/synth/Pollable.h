#pragma once
#include <list>

class SynthPollable {
public:
    SynthPollable();
    virtual ~SynthPollable();
    virtual const char *GetSoundDisplayName() { return ""; }
    virtual void SynthPoll() = 0;

    void StartPolling();
    void CancelPolling();

    static void PollAll();
    static std::list<SynthPollable *> &Pollables() { return sPollables; }

private:
    static std::list<SynthPollable *> sPollables;

protected:
    std::list<SynthPollable *>::iterator mItr; // 0x4
    bool mIsActive; // 0x8
};
