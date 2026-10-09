#include "hamobj/FreestyleMove.h"
#include "gesture/BaseSkeleton.h"
#include "hamobj/DancerSkeleton.h"

FreestyleMove::FreestyleMove()
    : mDepthFrames(0), mFrameCount(0), mCentering(0), mAverageDepth(0),
      mSkeletonFrames(0) {}

FreestyleMove::~FreestyleMove() {
    delete[] mDepthFrames;
    delete[] mSkeletonFrames;
}

void FreestyleMove::Clear() { mFrameCount = 0; }

void FreestyleMove::Free() {
    mFrameCount = 0;
    delete[] mDepthFrames;
    delete[] mSkeletonFrames;
    mDepthFrames = nullptr;
    mSkeletonFrames = nullptr;
}

void FreestyleMove::Init(int frames) {
    mFrameCount = 0;
    if (!mDepthFrames) {
        mDepthFrames = new DepthFrame[frames];
    }
    if (!mSkeletonFrames) {
        mSkeletonFrames = new FreestyleMoveFrame[frames];
    }
}

void FreestyleMove::RecordSkeletonFrame(BaseSkeleton *skeleton, int i2, float f3) {
    FreestyleMoveFrame frame;
    frame.mSkeleton.Init();
    frame.mTime = f3;
    if (skeleton && skeleton->IsTracked()) {
        frame.mSkeleton.Set(*skeleton);
    }
    mSkeletonFrames[i2] = frame;
}

void FreestyleMove::CalcCentering(int i1) {
    DepthFrame *whichFrame = &mDepthFrames[i1];
    float f12 = 0;
    int i10 = 0;
    int i170[80];
    memset(i170, 0, sizeof(i170));

    for (int i = 0; i < 80; i++) {
        for (int j = 0; j < 60; j++) {
            unsigned char b = whichFrame->mPixels[i + j * 80];
            if (b) {
                i10++;
                i170[i]++;
                f12 += b;
            }
        }
    }
    mAverageDepth = f12 / (float)i10;

    int i7 = 0;
    int i9 = 0;
    for (int i = 0; i < DIM(i170); i++) {
        i7 += i170[i];
        i9 += i170[i] * i;
    }
    if (i9 != 0) {
        i9 /= i7;
    }
    mCentering = i9 - 40;
}
