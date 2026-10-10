#pragma once
#include "gesture/BaseSkeleton.h"
#include "hamobj/DancerSkeleton.h"
#include "utl/MemMgr.h"

struct FreestyleMoveFrame {
    DancerSkeleton mSkeleton; // 0x0
    float mTime; // 0x2d8

    static void *operator new[](unsigned int s) {
        return _MemAllocTemp(s, __FILE__, 0x10, "FreestyleMoveFrame", 0);
    }
    static void operator delete(void *v) {
        MemFree(v, __FILE__, 0x10, "FreestyleMoveFrame");
    }
    static void operator delete[](void *v) {
        MemFree(v, __FILE__, 0x10, "FreestyleMoveFrame");
    }
};

// size 0x1c
class FreestyleMove {
public:
    // size 0x12c0
    struct DepthFrame {
        unsigned char mPixels[4800];

        static void *operator new[](unsigned int s) {
            return _MemAllocTemp(s, __FILE__, 0x26, "DepthFrame", 0);
        }
        static void operator delete[](void *v) {
            MemFree(v, __FILE__, 0x26, "DepthFrame");
        }
    };

    FreestyleMove();
    virtual ~FreestyleMove();

    void Clear();
    void Init(int);
    void CalcCentering(int);
    void Free();
    void RecordSkeletonFrame(BaseSkeleton *, int, float);

    MEM_OVERLOAD(FreestyleMove, 0x18);

    DepthFrame *mDepthFrames; // 0x4
    int mFrameCount; // 0x8
    int mSkeletonIndex; // 0xc
    int mCentering; // 0x10
    int mAverageDepth; // 0x14
    FreestyleMoveFrame *mSkeletonFrames; // 0x18
};

// size 0x10
struct FreestyleFrameScores {
    FreestyleFrameScores() {
        mFrameScores.resize(60);
        Clear();
    }

    void Clear() {
        mNumFrames = 0;
        for (int i = 0; i < 60; i++) {
            mFrameScores[i] = 0;
        }
    }

    std::vector<float> mFrameScores; // 0x0
    int mNumFrames; // 0xc
};
