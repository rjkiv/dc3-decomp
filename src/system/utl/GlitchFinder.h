#pragma once
#include "os/Timer.h"

class GlitchAverager {
public:
    GlitchAverager();
    ~GlitchAverager() {}

    void PushInstance(float, bool);

    // yes, this is public according to RBVR
    float mAvg; // 0x0
    float mMax; // 0x4
    int mCount; // 0x8
    float mGlitchAvg; // 0xc
    int mGlitchCount; // 0x10
};

class GlitchPoker {
public:
    GlitchPoker();
    ~GlitchPoker();

    bool OverBudget();
    void PollAveragesRecurse(bool);
    void Dump(TextStream &, int);
    void ClearData();

    static float smLastDumpTime;
    static bool smDumpLeaves;
    static float smThreshold;
    static float smTotalLeafTime;

    // yep, according to RBVR these are all public too
    char mName[64]; // 0x0
    float mTime; // 0x40
    float mTimeEnd; // 0x44
    std::vector<GlitchPoker *> mChildren; // 0x48
    GlitchPoker *mParent; // 0x54
    float mBudget; // 0x58
    GlitchAverager *mAvg; // 0x5c

private:
    static std::vector<float> smNestedStartTimes;
    void PrintResult(TextStream &);
    void PrintNestedStartTimes(TextStream &, float);
};

class GlitchFinder {
public:
    friend DataNode GlitchFindScriptImpl(DataArray *a, int i2);
    GlitchFinder();
    ~GlitchFinder();

    void CheckDump();
    void PokeEnd(unsigned int);
    void PokeStart(const char *, unsigned int, float, float, GlitchAverager *);
    void Poke(const char *, unsigned int);
    static void Init();

private:
    GlitchPoker *NewPoker();
    static DataNode OnGlitchFind(DataArray *);
    static DataNode OnGlitchFindBudget(DataArray *);
    static DataNode OnGlitchFindLeaves(DataArray *);
    static DataNode OnGlitchFindPoke(DataArray *);

    // surprisingly all private
    int mFrameCount; // 0x0
    int mGlitchCount; // 0x0
    bool mStop; // 0x8
    Timer mTime; // 0x10
    float mLastTime; // 0x40
    GlitchPoker mPokerPool[2048]; // 0x44
    int mPokerIndex; // 0x30044
    GlitchPoker *mStartPoker; // 0x30048
    GlitchPoker *mCurPoker; // 0x3004c
    bool unk30050; // 0x30050
    bool unk30051; // 0x30051
    float unk30054; // 0x30054
    unsigned int unk30058; // 0x30058

    // RBVR has bool mActive and int64_t mOverheadCycles
};

extern GlitchFinder TheGlitchFinder;

class AutoGlitchPoker {
public:
    AutoGlitchPoker(const char *func, float f1, float f2, GlitchAverager *avg) {
        unsigned int time = __mftb();
        mActive = true;
        TheGlitchFinder.PokeStart(func, time, f1, f2, avg);
    }
    ~AutoGlitchPoker();

    bool mActive; // 0x0
};
