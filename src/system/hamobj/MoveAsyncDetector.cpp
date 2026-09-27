#include "MoveDir.h"
#include "hamobj/DancerSequence.h"
#include "hamobj/Difficulty.h"
#include "hamobj/FilterVersion.h"
#include "hamobj/HamDirector.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamMove.h"
#include "hamobj/MoveDetector.h"
#include "os/Debug.h"

MoveDetector::MoveDetector(
    const FilterVersion *fv, const HamMove *move, const DancerFrame *&dancer_frame
)
    : mMove(move), mActive(false), unk8(-1), unkc(-1) {
    MILO_ASSERT(mMove, 0x18);
    MoveMirrored mirrored = move->Mirrored();
    const std::vector<MoveFrame> &moveFrames = mMove->GetMoveFrames();
    mDancerFrames.resize(moveFrames.size());
    for (int i = 0; i < 2; i++) {
        mLastDetectFracs[i] = 0;
        mPlayerDetectFrames[i].resize(moveFrames.size());
    }
    for (int mf = 0; mf < moveFrames.size(); mf++, dancer_frame++) {
        if (dancer_frame->mMoveFrameIdx != mf) {
            MILO_FAIL(
                "HamMove '%s': dancer_frame->mMoveFrameIdx != mf",
                move ? PathName(move) : "NULL"
            );
        }
        DancerFrame &cur = mDancerFrames[mf];
        cur.mMoveIdx = -1;
        cur.mMoveFrameIdx = mf;
        cur.mSkeleton = dancer_frame->mSkeleton;
        for (int i = 0; i < 2; i++) {
            DetectFrame &curDetectFrame = mPlayerDetectFrames[i][mf];
            curDetectFrame.Reset(fv, -1, &moveFrames[mf], &cur, mirrored);
        }
    }
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 4; j++) {
            unk3c[i][j] = 0;
        }
    }
}

MoveDetector::~MoveDetector() {}

float MoveDetector::ActiveDetectFrac(int player, MoveDir *dir) {
    MILO_ASSERT(mActive, 0x42);
    MILO_ASSERT_RANGE(player, 0, 2, 0x43);
    auto &frames = mPlayerDetectFrames[player];
    return dir->DetectFrac(player, mMove, std::make_pair(frames.begin(), frames.end()));
}

float MoveDetector::LastDetectFrac(int player) const {
    MILO_ASSERT(mActive, 0x4D);
    MILO_ASSERT_RANGE(player, 0, 2, 0x4E);
    return mLastDetectFracs[player];
}

std::vector<DetectFrame> &MoveDetector::PlayerDetectFrames(int player) {
    MILO_ASSERT_RANGE(player, 0, 2, 0x3C);
    return mPlayerDetectFrames[player];
}

MoveAsyncDetector::MoveAsyncDetector(MoveDir *md) : mDir(md) {
    if (!TheGameData->GetSong().Null()) {
        MILO_ASSERT(md, 0xE9);
        DancerSequence *perfSeq = md->PerformanceSequence(kDifficultyExpert);
        if (!perfSeq) {
            MILO_NOTIFY("MoveAsyncDetector could not find expert performance sequence");
        } else {
            const std::vector<DancerFrame> &frames = perfSeq->GetDancerFrames();
            MILO_ASSERT(TheHamDirector, 0xF5);
            std::vector<HamMoveKey> keys;
            TheHamDirector->MoveKeys(kDifficultyExpert, md, keys);
            for (ObjDirItr<HamMove> it(md, true); it != nullptr; ++it) {
                if (it->Scored()) {
                    int keyIdx = -1;
                    for (int i = 0; i < keys.size(); i++) {
                        if (keys[i].move == it) {
                            keyIdx = i;
                            break;
                        }
                    };
                    if (keyIdx == -1) {
                        if (it->GetDancerSequence()) {
                            const DancerFrame *curFrame =
                                it->GetDancerSequence()->GetDancerFrames().begin();
                            const FilterVersion *curFv = it->FilterVer();
                            unk4.push_back(new MoveDetector(curFv, it, curFrame));
                        } else {
                            MILO_NOTIFY("Could not find %s in expert keys", PathName(it));
                        }
                    } else {
                        auto frameIt = frames.begin();
                        for (; frameIt != frames.end() && frameIt->mMoveIdx != keyIdx;
                             ++frameIt)
                            ;
                        if (frameIt != frames.end()) {
                            const FilterVersion *curFv = it->FilterVer();
                            unk4.push_back(new MoveDetector(curFv, it, frameIt));
                        }
                    }
                }
            }
            std::sort(unk4.begin(), unk4.end(), MoveDetectorCmp());
        }
    }
}

MoveAsyncDetector::~MoveAsyncDetector() {
    unk10.clear();
    DeleteAll(unk4);
}

void MoveAsyncDetector::EnqueueDetectFrames(int i1, int i2, float f3, int i4) {
    FOREACH (it, unk10) {
        MoveDetector *cur = *it;
        cur->Poll(i1, i2, mDir);
        auto &frames = cur->PlayerDetectFrames(i4);
        mDir->EnqueueDetectFrames(f3, i4, frames, cur->Move()->FilterVer());
    }
}

void MoveAsyncDetector::DisableAllDetectors() {
    unk10.clear();
    FOREACH (it, unk4) {
        (*it)->Reset();
    }
}
