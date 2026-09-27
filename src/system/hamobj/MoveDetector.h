#pragma once
#include "hamobj/DetectFrame.h"
#include "hamobj/DancerSequence.h"
#include "hamobj/FilterVersion.h"
#include "hamobj/HamMove.h"
#include <set>

class MoveDir;

// size 0x5c
class MoveDetector {
public:
    MoveDetector(const FilterVersion *, const HamMove *, const DancerFrame *&);
    ~MoveDetector();

    float ActiveDetectFrac(int, MoveDir *);
    float LastDetectFrac(int) const;
    std::vector<DetectFrame> &PlayerDetectFrames(int);
    float Last4BeatsDetectFrac(int) const;
    void Poll(int, int, MoveDir *);
    const HamMove *Move() const { return mMove; }
    const FilterVersion *GetFilterVersion() const { return mMove->FilterVer(); }
    bool Active() const { return mActive; }
    void Enable() {
        if (mActive != true) {
            for (int i = 0; i < 2; i++) {
                mLastDetectFracs[i] = 0;
            }
            unk8 = -1;
            unkc = -1;
            mActive = true;
        }
    }
    void Disable() {
        if (mActive != false) {
            for (int i = 0; i < 2; i++) {
                mLastDetectFracs[i] = 0;
            }
            unk8 = -1;
            unkc = -1;
            mActive = false;
        }
    }
    void ClearLoopedRatingFrac() {
        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < 4; j++) {
                unk3c[i][j] = 0;
            }
        }
    }

protected:
    const HamMove *mMove; // 0x0
    bool mActive; // 0x4
    int unk8; // 0x8
    int unkc; // 0xc
    std::vector<DancerFrame> mDancerFrames; // 0x10
    std::vector<DetectFrame> mPlayerDetectFrames[2]; // 0x1c
    float mLastDetectFracs[2]; // 0x34
    float unk3c[2][4]; // 0x3c
};

// size 0x28
class MoveAsyncDetector {
public:
    enum RatingBar {
    };
    MoveAsyncDetector(MoveDir *);
    ~MoveAsyncDetector();

    void EnqueueDetectFrames(int, int, float, int);
    void DisableAllDetectors();
    void EnableDetector(HamMove *);
    void DisableDetector(HamMove *);
    void ClearLoopedRatingFrac(const HamMove *);
    float MoveRatingFrac(int, RatingBar, const HamMove *);

private:
    MoveDetector *FindDetector(const HamMove *);

    MoveDir *mDir; // 0x0
    std::vector<MoveDetector *> mDetectors; // 0x4
    std::set<MoveDetector *> mActiveDetectors; // 0x10
};

struct MoveDetectorCmp {
    bool operator()(MoveDetector *md1, MoveDetector *md2) const {
        return md1->Move() < md2->Move();
    }
    bool operator()(MoveDetector *md, const HamMove *move) const {
        return md->Move() < move;
    }
    bool operator()(const HamMove *move, MoveDetector *md) const {
        return move < md->Move();
    }
};
