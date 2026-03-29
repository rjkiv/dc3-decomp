#include "utl/GlitchFinder.h"
#include "obj/DataFunc.h"
#include "os/Debug.h"
#include "ppcintrinsics.h"

GlitchFinder TheGlitchFinder;
std::vector<float> GlitchPoker::smNestedStartTimes;

DataNode GlitchFindScriptImpl(DataArray *a, int i2) {
    unsigned int prevMFTB = __mftb();
    DataNode n;
    if (a->Node(2).NotNull()) {
        switch (i2) {
        case 3:
            TheGlitchFinder.PokeStart(a->Str(1), prevMFTB, -1, 0, nullptr);
            break;
        case 4:
            TheGlitchFinder.PokeStart(a->Str(1), prevMFTB, a->Float(3), 0, nullptr);
            break;
        case 5:
            TheGlitchFinder.PokeStart(
                a->Str(1), prevMFTB, a->Float(3), a->Float(4), nullptr
            );
            break;
        default:
            MILO_FAIL("improper use of internal glitch finder code");
            break;
        }
        for (; i2 < a->Size(); i2++) {
            n = a->Command(i2)->Execute();
        }
        TheGlitchFinder.PokeEnd(__mftb());
        return DataNode(n);
    } else {
        TheGlitchFinder.unk30058 = (TheGlitchFinder.unk30058 - prevMFTB) + __mftb();
        for (; i2 < a->Size(); i2++) {
            n = a->Command(i2)->Execute();
        }
        return DataNode(n);
    }
}

#pragma region GlitchAverager

GlitchAverager::GlitchAverager()
    : mAvg(0), mMax(0), mCount(0), mGlitchAvg(0), mGlitchCount(0) {}

void GlitchAverager::PushInstance(float f1, bool b) {
    mAvg = (f1 - mAvg) / ++mCount + mAvg;
    if (b) {
        mGlitchAvg = (f1 - mGlitchAvg) / ++mGlitchCount + mGlitchAvg;
    }
    if (f1 > mMax) {
        mMax = f1;
    }
}

#pragma endregion
#pragma region GlitchPoker

GlitchPoker::GlitchPoker() {}

GlitchPoker::~GlitchPoker() {}

void GlitchPoker::ClearData() {
    mTime = -1;
    mName[0] = '\0';
    mTimeEnd = -1;
    mChildren.clear();
    mBudget = -1;
    mParent = 0;
    mAvg = 0;
}

bool GlitchPoker::OverBudget() {
    if (mBudget > 0 && mTimeEnd - mTime > mBudget) {
        return true;
    }
    for (int i = 0; i < mChildren.size(); i++) {
        if (mChildren[i]->OverBudget()) {
            return true;
        }
    }
    return false;
}

void GlitchPoker::PrintResult(TextStream &stream) {
    float diff = mTimeEnd - mTime;
    if (mChildren.size() > 0 || diff >= 0.005f) {
        stream << "{ " << mName << " (" << diff << ") ";
    } else {
        stream << "[ " << mName << " ] ";
    }
    if (mAvg) {
        stream << "<" << mAvg->mAvg << " avg, " << mAvg->mGlitchAvg << " glitch avg, "
               << mAvg->mMax << " max> ";
    }
}

void GlitchPoker::PollAveragesRecurse(bool b) {
    if (mAvg) {
        mAvg->PushInstance(mTimeEnd - mTime, b);
    }
    for (int i = 0; i < mChildren.size(); i++) {
        mChildren[i]->PollAveragesRecurse(b);
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
    float f1 = 0.005f;
    if (mTime > smLastDumpTime + 0.005f) {
        PrintNestedStartTimes(stream, smLastDumpTime);
        if (!smDumpLeaves) {
            stream << "TIME GAP (" << mTime - smLastDumpTime << ")\n";
        } else if (mTime - smLastDumpTime > smThreshold) {
            stream << "   TIME GAP (" << mTime - smLastDumpTime;
            stream << ") before " << mName;
            for (GlitchPoker *it = mParent; it != nullptr; it = it->mParent) {
                stream << " : " << it->mName;
            }
            stream << "\n";
            smTotalLeafTime += mTime - smLastDumpTime;
        }
    }
    PrintNestedStartTimes(stream, mTime);
    if (!smDumpLeaves && mChildren.size() <= 0 && mTimeEnd - mTime < 0.005f) {
        stream << "[ " << mName << " ]";
        if (mAvg) {
            stream << " (" << mAvg->mAvg << " avg)";
        }
    } else {
        if (smDumpLeaves && mParent) {
            if (mTimeEnd - mTime > smThreshold) {
                if (mChildren.size() > 0) {
                    smLastDumpTime = mTime;
                    for (int i = 0; i < mChildren.size(); i++) {
                        mChildren[i]->Dump(stream, i1 + 1);
                    }
                    if (smThreshold < mTimeEnd - smLastDumpTime) {
                        stream << "   TIME GAP (" << mTimeEnd - smLastDumpTime
                               << ") at end of " << mName;
                        for (GlitchPoker *it = mParent; it != nullptr; it = it->mParent) {
                            stream << " : " << it->mName;
                        }
                        stream << "\n";
                        smTotalLeafTime += mTimeEnd - smLastDumpTime;
                    }
                } else {
                    stream << "   ";
                    PrintResult(stream);
                    stream << "}";
                    for (GlitchPoker *it = mParent; it != nullptr; it = it->mParent) {
                        stream << " : " << it->mName;
                    }
                    stream << "\n";
                    smTotalLeafTime += mTimeEnd - mTime;
                }
            }
            smLastDumpTime = mTimeEnd;
            return;
        }
        PrintResult(stream);
        if (mChildren.size() > 0) {
            stream << "\n";
            smLastDumpTime = mTime;
            smNestedStartTimes.push_back(mTime);
            for (int i = 0; i < mChildren.size(); i++) {
                mChildren[i]->Dump(stream, i1 + 1);
            }
            if (smLastDumpTime + 0.005f < mTimeEnd) {
                PrintNestedStartTimes(stream, smLastDumpTime);
                if (!smDumpLeaves) {
                    stream << "TIME GAP (" << mTimeEnd - smLastDumpTime << ")\n";
                } else if (mTime - smLastDumpTime > smThreshold) {
                    stream << "TIME GAP (" << mTimeEnd - smLastDumpTime << ") at end of "
                           << mName;
                    for (GlitchPoker *it = mParent; it != nullptr; it = it->mParent) {
                        stream << " : " << it->mName;
                    }
                    stream << "\n";
                }
            }
            smNestedStartTimes.pop_back();
            PrintNestedStartTimes(stream, mTimeEnd);
        }
        if (smDumpLeaves) {
            stream << "} total leaf time: " << smTotalLeafTime << " ("
                   << (smTotalLeafTime / (mTimeEnd - mTime)) * 100.0f << "pct)";
        } else if (mChildren.size() > 0 || mTimeEnd - mTime >= 0.005f) {
            stream << "}";
        }
    }
    stream << "\n";
    smLastDumpTime = mTimeEnd;
}

#pragma endregion
#pragma region GlitchFinder

GlitchFinder::GlitchFinder()
    : mFrameCount(0), mGlitchCount(0), mStop(true), mLastTime(0.0), mPokerIndex(-1),
      mStartPoker(0), mCurPoker(0), unk30050(true), unk30051(false), unk30054(0.0),
      unk30058(0) {
    mTime.Start();
}

GlitchFinder::~GlitchFinder() {}

void GlitchFinder::Init() {
    DataRegisterFunc("glitch_find", OnGlitchFind);
    DataRegisterFunc("glitch_find_budget", OnGlitchFindBudget);
    DataRegisterFunc("glitch_find_leaves", OnGlitchFindLeaves);
    DataRegisterFunc("glitch_find_poke", OnGlitchFindPoke);
}

DataNode GlitchFinder::OnGlitchFindPoke(DataArray *da) {
    unsigned int mftb = __mftb();
    TheGlitchFinder.Poke(da->Str(1), mftb);
    return 0;
}

DataNode GlitchFinder::OnGlitchFind(DataArray *da) { return GlitchFindScriptImpl(da, 3); }

DataNode GlitchFinder::OnGlitchFindBudget(DataArray *da) {
    return GlitchFindScriptImpl(da, 4);
}

DataNode GlitchFinder::OnGlitchFindLeaves(DataArray *da) {
    return GlitchFindScriptImpl(da, 5);
}

void GlitchFinder::Poke(const char *c, unsigned int ui) {
    PokeStart(c, 0, -1, 0, 0);
    PokeEnd(ui);
}

GlitchPoker *GlitchFinder::NewPoker() {
    MILO_ASSERT_FMT(mPokerIndex < 2048, "too many glitch pokers : %d\n", mPokerIndex);
    int old = mPokerIndex++;
    mPokerPool[old].ClearData();
    return &mPokerPool[old];
}

void GlitchFinder::PokeStart(
    const char *c, unsigned int ui, float f1, float f2, GlitchAverager *avg
) {
    if (mStartPoker || f1 >= 0.0) {
        if (mStop) {
            mStop = 0;
            mTime.Restart();
            mCurPoker = 0;
            mStartPoker = 0;
            mPokerIndex = 0;
        }
        GlitchPoker *poker = NewPoker();
        strncpy(poker->mName, c, 64);
        poker->mTime = mTime.SplitMs();
        poker->mBudget = f1;
        poker->mParent = mCurPoker;
        poker->mAvg = avg;
        if (!mStartPoker) {
            mCurPoker = poker;
            mStartPoker = poker;
            unk30054 = f2;
            unk30058 = 0;
        } else {
            mCurPoker->mChildren.push_back(poker);
            if (ui) {
                unsigned int mftb = __mftb();
                unk30058 = unk30058 - ui + mftb;
            }
        }
    }
}

void GlitchFinder::PokeEnd(unsigned int ui) {
    if (mCurPoker) {
        mCurPoker->mTimeEnd = mTime.SplitMs();
        if (!mCurPoker->mParent) {
            CheckDump();
        }
    }
    if (ui) {
        unsigned int mftb = __mftb();
        unk30058 = unk30058 - ui + mftb;
    }
}

void GlitchFinder::CheckDump() {
    if (!mStop && mStartPoker) {
        mStop = true;
        mStartPoker->mTimeEnd = mTime.SplitMs();
        static unsigned int sMFTB = 0;
        if (sMFTB == 0) {
            sMFTB = __mftb();
        }
        bool u7 = unk30050 && mStartPoker->OverBudget();
        mStartPoker->PollAveragesRecurse(u7);
        if (u7) {
            GlitchPoker::smDumpLeaves = unk30054 > 0;
            GlitchPoker::smThreshold = unk30054;
            GlitchPoker::smTotalLeafTime = 0;
            String str(0x2000, '\0');
            str << "-------- GLITCH #" << mGlitchCount << " -------- Frame "
                << mFrameCount << " -----\n";
            GlitchPoker::smLastDumpTime = mStartPoker->mTime;
            mStartPoker->Dump(str, 0);
            str << "Overhead: " << Timer::CyclesToMs(unk30058) << "\n";
            str << "-------- GLITCH END --------\n";
            int len = str.length();
            if (len > 0x400) {
                char buffer[0x400];
                int i = 0;
                for (; i + 0x400 < len; i += 0x400) {
                    strncpy(buffer, &str.c_str()[i], 0x400);
                    buffer[0x400] = '\0';
                    MILO_LOG(buffer);
                }
                strncpy(buffer, &str.c_str()[i], len - i);
                buffer[len - i] = '\0';
                MILO_LOG(buffer);
            } else {
                MILO_LOG(str.c_str());
            }
            mGlitchCount++;
        }
        if (unk30050) {
            mFrameCount++;
        }
        mStartPoker = nullptr;
        mPokerIndex = 0;
        mCurPoker = nullptr;
    }
}

#pragma endregion

AutoGlitchPoker::~AutoGlitchPoker() {
    if (mActive) {
        unsigned int time = __mftb();
        TheGlitchFinder.PokeEnd(time);
    }
}
