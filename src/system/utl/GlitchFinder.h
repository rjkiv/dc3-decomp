#pragma once
#include "os/Timer.h"

DataNode GlitchFindScriptImpl(DataArray *, int);

class GlitchAverager {
public:
    GlitchAverager();
    ~GlitchAverager();

    void PushInstance(float, bool);

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

    GlitchAverager *GetAverager() { return mAvg; };
    void SetUnk0(const char *c) { strncpy(unk0, c, 0x40); }
    void SetUnk40(float f) { unk40 = f; }
    void SetUnk58(float f) { unk58 = f; }
    GlitchPoker *GetUnk54() { return unk54; }
    void SetUnk54(GlitchPoker *poker) { unk54 = poker; }
    void SetAverager(GlitchAverager *avg) {  mAvg = avg; }
    std::vector<GlitchPoker *> GetUnk48() { return unk48;}
    void Unk48PushBack(GlitchPoker *poker) { unk48.push_back(poker); }
    void SetUnk44(float f) { unk44 = f; }

private:
    static std::vector<float> smNestedStartTimes;
    void PrintResult(TextStream &);
    void PrintNestedStartTimes(TextStream &, float);

protected:
    char unk0[64]; // 0x0
    float unk40; // 0x40
    float unk44; // 0x44
    std::vector<GlitchPoker *> unk48; // 0x48
    GlitchPoker *unk54; // 0x54
    float unk58; // 0x58
    GlitchAverager *mAvg; // 0x5c
};

class GlitchFinder {
public:
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

protected:
    int unk0; // 0x0
    int unk4; // 0x0
    bool unk8; // 0x8
    Timer unk10; // 0x10
    //int unk34; // 0x34
    float unk40; // 0x40
    GlitchPoker mPokerPool[2048]; // 0x44
    int unk30044; // 0x30044
    GlitchPoker *unk30048; // 0x30048
    GlitchPoker *unk3004c; // 0x3004c
    bool unk30050; // 0x30050
    bool unk30051; // 0x30051
    float unk30054; // 0x30054
    double *unk30058; // 0x30058
};

extern GlitchFinder TheGlitchFinder;