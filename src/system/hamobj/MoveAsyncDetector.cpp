#include "MoveDir.h"
#include "hamobj/DancerSequence.h"
#include "hamobj/Difficulty.h"
#include "hamobj/FilterVersion.h"
#include "hamobj/HamDirector.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamMove.h"
#include "hamobj/MoveDetector.h"
#include "os/Debug.h"
#include "utl/TimeConversion.h"

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

float MoveDetector::Last4BeatsDetectFrac(int player) const {
    MILO_ASSERT(mActive, 0x54);
    MILO_ASSERT_RANGE(player, 0, 2, 0x55);

    int i8 = 0;
    float f3 = 0;
    for (int i = 0; i < 4; i++) {
        f3 += unk3c[player][i];
        if (unk3c[player][i] < 0.13f) {
            i8++;
        }
    }
    if (i8 <= 1) {
        float f2 = (f3 / 4) * 1.15f;
        f3 = 0;
        for (int i = 0; i < 4; i++) {
            f3 += Clamp(0.0f, f2, unk3c[player][i]);
        }
        return Clamp(0.0f, 1.0f, f3);
    } else {
        return 0;
    }
}

void MoveDetector::Poll(int i1, int i2, MoveDir *moveDir) {
    if (mActive) {
        if (i2 != unkc) {
            if (unkc != -1) {
                for (int i = 0; i < 2; i++) {
                    float f12 = ActiveDetectFrac(i, moveDir);
                    if (unk3c[i][3] < f12) {
                        f12 -= unk3c[i][3];
                    }
                    for (int j = 1; j < 4; j++) {
                        unk3c[i][j - 1] = unk3c[i][j];
                    }
                    unk3c[i][3] = f12;
                }
            }
            unkc = i2;
        }
        if (i1 != unk8) {
            if (unk8 != -1) {
                for (int i = 0; i < 2; i++) {
                    mLastDetectFracs[i] = ActiveDetectFrac(i, moveDir);
                }
            }
            unk8 = i1;
            float f11 = (float)i1 * 4.0f;
            FOREACH (it, mDancerFrames) {
                it->mMoveIdx = i1;
            }
            int numFrames = mPlayerDetectFrames[0].size();
            for (int i = 0; i < numFrames; i++) {
                float f12 =
                    BeatToSeconds(mPlayerDetectFrames[0][i].GetMoveFrame()->Beat() + f11);
                for (int j = 0; j < 2; j++) {
                    mPlayerDetectFrames[j][i].SetSecondsAndReset(f12);
                }
            }
        }
    }
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
                            mDetectors.push_back(new MoveDetector(curFv, it, curFrame));
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
                            mDetectors.push_back(new MoveDetector(curFv, it, frameIt));
                        }
                    }
                }
            }
            std::sort(mDetectors.begin(), mDetectors.end(), MoveDetectorCmp());
        }
    }
}

MoveAsyncDetector::~MoveAsyncDetector() {
    mActiveDetectors.clear();
    DeleteAll(mDetectors);
}

void MoveAsyncDetector::EnqueueDetectFrames(int i1, int i2, float f3, int i4) {
    FOREACH (it, mActiveDetectors) {
        MoveDetector *cur = *it;
        cur->Poll(i1, i2, mDir);
        auto &frames = cur->PlayerDetectFrames(i4);
        mDir->EnqueueDetectFrames(f3, i4, frames, cur->GetFilterVersion());
    }
}

void MoveAsyncDetector::DisableAllDetectors() {
    mActiveDetectors.clear();
    FOREACH (it, mDetectors) {
        (*it)->Disable();
    }
}

MoveDetector *MoveAsyncDetector::FindDetector(const HamMove *move) {
    auto range =
        std::equal_range(mDetectors.begin(), mDetectors.end(), move, MoveDetectorCmp());
    if (range.first == range.second) {
        if (move->GetDancerSequence()) {
            auto it = move->GetDancerSequence()->GetDancerFrames().begin();
            MoveDetector *md = new MoveDetector(move->FilterVer(), move, it);
            mDetectors.push_back(md);
            std::sort(mDetectors.begin(), mDetectors.end(), MoveDetectorCmp());
            return md;
        } else {
            return nullptr;
        }
    } else {
        return *(range.first);
    }
}

void MoveAsyncDetector::EnableDetector(HamMove *move) {
    if (move) {
        MoveDetector *md = FindDetector(move);
        if (md) {
            md->Enable();
            mActiveDetectors.insert(md);
        } else {
            MILO_NOTIFY("Could not enable detector for %s", move->Name());
        }
    }
}

void MoveAsyncDetector::DisableDetector(HamMove *move) {
    if (move) {
        MoveDetector *md = FindDetector(move);
        if (md) {
            md->Disable();
            mActiveDetectors.erase(md);
        } else {
            MILO_NOTIFY("Could not disable detector for %s", move->Name());
        }
    }
}

void MoveAsyncDetector::ClearLoopedRatingFrac(const HamMove *move) {
    MoveDetector *md = FindDetector(move);
    if (md) {
        md->ClearLoopedRatingFrac();
    }
}

float MoveAsyncDetector::MoveRatingFrac(
    int player, RatingBar rating, const HamMove *move
) {
    if (move && move->Scored()) {
        MILO_ASSERT_RANGE(player, 0, 2, 0x144);
        MoveDetector *md = FindDetector(move);
        if (md) {
            if (!md->Active()) {
                MILO_NOTIFY_ONCE(
                    "MoveRatingFrac for %s called, but it's disabled", move->Name()
                );
            } else {
                md->Poll(mDir->MoveIdx(), mDir->MoveBeat(), mDir);
                if (rating == 0) {
                    return md->ActiveDetectFrac(player, mDir);
                } else if (rating == 2) {
                    return md->Last4BeatsDetectFrac(player);
                } else {
                    return md->LastDetectFrac(player);
                }
            }
        } else {
            MILO_NOTIFY("Could not find rating for %s", move->Name());
        }
    }
    return 0;
}
