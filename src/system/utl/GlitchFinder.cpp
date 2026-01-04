#include "GlitchFinder.h"

#include "obj/DataFunc.h"

GlitchFinder TheGlitchFinder;

GlitchPoker::GlitchPoker() {  }

GlitchPoker::~GlitchPoker() {  }

void GlitchPoker::ClearData() {
    unk40 = -1.0;
    unk0[0] = '\0';
    unk44 = -1.0;
    unk48.clear();
    unk58 = -1.0;
    unk54 = 0;
    mAvg = 0;
}

bool GlitchPoker::OverBudget() {
    if (unk58 > 0.0 && unk44 - unk40 > unk58) {
        return true;
    }
    for (int i = 0; i < unk48.size(); i++) {
        if (unk48[i]->OverBudget()) {
            return true;
        }
    }
    return false;
}

void GlitchPoker::PrintResult(TextStream &stream) {
    float temp = unk44 - unk40;
    if (unk48.size() > 0 || temp >= 0.0049999999f) {
        stream << "{ " << unk0 << " (" << temp << ") ";
    }
    else {
        stream << "[ " << unk0 << " ] ";
    }
    if (mAvg) {
        stream << "<" << mAvg->mAvg << " avg, " << mAvg->mGlitchAvg << " glitch avg, " << mAvg->mMax << " max> ";
    }
}

void GlitchPoker::PollAveragesRecurse(bool b) {
    if (mAvg) {
        mAvg->PushInstance(unk44 - unk40, b);
    }
    for (int i = 0; i < unk48.size(); i++) {
        unk48[i]->PollAveragesRecurse(b);
    }
}

void GlitchPoker::PrintNestedStartTimes(TextStream &stream, float f1) {
    if (!smDumpLeaves) {
        stream << f1 << " ";
        if (smNestedStartTimes.size() != 0) {
            for (int i = 0; i < smNestedStartTimes.size(); i++) {
                stream << " " << f1 - smNestedStartTimes[i] << " ";
            }
        }
    }
}

void GlitchPoker::Dump(TextStream &stream, int i1) {
    float f1 = 0.0049999999f;
    if (smLastDumpTime + 0.005 < unk40) {
        PrintNestedStartTimes(stream, smLastDumpTime);
        if (!smDumpLeaves) {
            stream << "TIME GAP (" << unk40 - smLastDumpTime << ")\n";
        }
        else if (smThreshold < unk40 - smLastDumpTime) {
            stream << "   TIME GAP (" << unk40 - smLastDumpTime << ") before " << unk54->unk0 << " : ";
        }
        stream << "\n";
        smTotalLeafTime = (unk40 - smLastDumpTime) + smTotalLeafTime;
    }
    PrintNestedStartTimes(stream, unk40);
    if (smDumpLeaves || !unk48.empty() || f1 <= unk44 - unk40) {
        if (smLastDumpTime && unk54) {
            f1 = smThreshold;
            if (f1 < unk44 - unk40) {
                if (unk48.empty()) {
                    stream << "   ";
                    PrintResult(stream);
                    stream << "}";
                    for (int i = 0; i < sizeof(unk54->unk0); i++) {
                        stream << " : " << unk54->unk0[i];
                    }
                }
                stream << "\n";
                smTotalLeafTime = (unk44 - unk40) + smTotalLeafTime;
            }
            else {
                for (int i = 0; i < unk48.size(); i++) {
                    Dump(stream, i1);
                }
                f1 = smThreshold;

                if (f1 < unk44 - smLastDumpTime) {
                    stream << "   TIME GAP (" << unk44 - smLastDumpTime << ") at end of " << unk0;
                    for (int i = 0; i < sizeof(unk54->unk0); i++) {
                        stream << " : " << unk54->unk0[i];
                    }
                    stream << "\n";
                    smTotalLeafTime = (unk44 - smLastDumpTime) + smTotalLeafTime;
                }
            }
        }
        smLastDumpTime = unk44;
    }
    PrintResult(stream);
    for (int i = 0; i < unk48.size(); i++) {
        stream << "\n";
        float lastDumpTime = smLastDumpTime;
        smLastDumpTime = unk40;
        smNestedStartTimes.push_back(lastDumpTime);
        for (int i = 0; i < unk48.size(); i++) {
            Dump(stream, i1 + 1);
        }
        if (smLastDumpTime + f1 < unk44) {
            PrintNestedStartTimes(stream, smLastDumpTime);
            if (!smDumpLeaves) {
                stream << "TIME GAP (" << unk44 - smLastDumpTime;
            }
            else {
                if (!(unk40 - smLastDumpTime <= smThreshold)) {
                    stream << "   TIME GAP (" << unk44 - smLastDumpTime << ") at end of " << unk0;
                    for (int i = 0; i < sizeof(unk54->unk0); i++) {
                        stream << " : " << unk54->unk0[i];
                    }
                }
                stream << "\n";
            }
        }
        smLastDumpTime = lastDumpTime;
        // something DAT here
        PrintNestedStartTimes(stream, unk44);
    }
    if (!smDumpLeaves) {
        if (!(unk48.empty() && unk44 - unk40 < f1)) {

        }
    }
}

GlitchAverager::GlitchAverager() : mAvg(0.0), mMax(0.0), mCount(0), mGlitchAvg(0.0), mGlitchCount(0) {}

void GlitchAverager::PushInstance(float f1, bool b) {
    mCount += 1;
    mAvg = (f1 - mAvg) / mCount + mAvg;
    if (b) {
        mGlitchCount += 1;
        mGlitchAvg = (f1 - mGlitchAvg) / mGlitchCount + mGlitchAvg;
    }
    if (f1 <= mMax)
        return;

    mMax = f1;
}

GlitchFinder::GlitchFinder() : unk0(0), unk4(0), unk8(true), unk40(0.0), unk30044(-1), unk30048(0), unk3004c(0), unk30050(true), unk30051(false), unk30054(0.0), unk30058(0) {
    unk10.Start();
}

void GlitchFinder::Init() {
    DataRegisterFunc("glitch_find", OnGlitchFind);
    DataRegisterFunc("glitch_find_budget", OnGlitchFindBudget);
    DataRegisterFunc("glitch_find_leaves", OnGlitchFindLeaves);
    DataRegisterFunc("glitch_find_poke", OnGlitchFindPoke);
}

DataNode GlitchFinder::OnGlitchFindPoke(DataArray *da) {
    unsigned int mftb = __mftb();
    TheGlitchFinder.Poke(da->Node(1).Str(da), mftb);
    return 0;
}

DataNode GlitchFinder::OnGlitchFind(DataArray *da) {
    return GlitchFindScriptImpl(da, 3);
}

DataNode GlitchFinder::OnGlitchFindBudget(DataArray *da) {
    return GlitchFindScriptImpl(da, 4);
}

DataNode GlitchFinder::OnGlitchFindLeaves(DataArray *da) {
    return GlitchFindScriptImpl(da, 5);
}

void GlitchFinder::Poke(const char *c, unsigned int ui) {
    PokeStart(c, 0, -1.0, 0.0, 0);
    PokeEnd(ui);
}

GlitchPoker *GlitchFinder::NewPoker() {
    if (2048 <= unk30044) {
        MILO_FAIL("too many glitch pokers : %d\n", unk30044);
    }
    unk30044 += 1;
    mPokerPool[unk30044].ClearData(); // TODO: fixmeeee
    return 0;
}

void GlitchFinder::PokeStart(const char *c, unsigned int ui, float f1, float f2, GlitchAverager *avg) {
    if (unk30048 || f1 >= 0.0) {
        if (unk8) {
            unk8 = 0;
            unk10.Restart();
            unk3004c = 0;
            unk30048 = 0;
            unk30044 = 0;
        }
        GlitchPoker *poker = NewPoker();
        poker->SetUnk0(c);
        poker->SetUnk40(unk10.SplitMs());
        poker->SetUnk58(f1);
        poker->SetUnk54(unk3004c);
        poker->SetAverager(avg);
        if (!unk30048) {
            unk3004c = poker;
            unk30048 = poker;
            unk30054 = f2;
            unk30058 = 0;
        }
        else {
            unk3004c->Unk48PushBack(poker);
            if (ui) {
                unsigned int mftb = __mftb();
                unk30058 = unk30058 - ui + mftb;
            }
        }
    }
}

void GlitchFinder::PokeEnd(unsigned int ui) {
    if (unk3004c) {
        unk3004c->SetUnk44(unk10.SplitMs());
        if (!unk3004c->GetUnk54()) {
            CheckDump();
        }
    }
    if (ui) {
        unsigned int mftb = __mftb();
        unk30058 = unk30058 - ui + mftb;
    }
}