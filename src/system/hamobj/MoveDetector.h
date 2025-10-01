#pragma once
#include "hamobj/DetectFrame.h"
#include "hamobj/DancerSequence.h"
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
    void Reset() {
        if (mActive) {
            mLastDetectFracs[0] = 0;
            mLastDetectFracs[1] = 0;
            unk8 = -1;
            unkc = -1;
            mActive = false;
        }
    }

protected:
    const HamMove *mMove; // 0x0
    bool mActive; // 0x4
    int unk8; // 0x8
    int unkc; // 0xc
    std::vector<DancerFrame> unk10; // 0x10
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
    std::vector<MoveDetector *> unk4; // 0x4
    std::set<MoveDetector *> unk10; // 0x10
};

struct MoveDetectorCmp {
    bool operator()(MoveDetector *md1, MoveDetector *md2) const {
        return md1->Move() < md2->Move();
    }
};
