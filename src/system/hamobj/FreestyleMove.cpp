#include "hamobj/FreestyleMove.h"
#include "gesture/BaseSkeleton.h"
#include "hamobj/DancerSkeleton.h"

FreestyleMove::FreestyleMove()
    : mDepthFrames(0), mNumFrames(0), unk10(0), unk14(0), mFreestyleMoveFrames(0) {}

FreestyleMove::~FreestyleMove() {
    delete[] mDepthFrames;
    delete[] mFreestyleMoveFrames;
}

void FreestyleMove::Clear() { mNumFrames = 0; }

void FreestyleMove::Free() {
    mNumFrames = 0;
    delete[] mDepthFrames;
    delete[] mFreestyleMoveFrames;
    mDepthFrames = nullptr;
    mFreestyleMoveFrames = nullptr;
}

void FreestyleMove::Init(int frames) {
    mNumFrames = 0;
    if (!mDepthFrames) {
        mDepthFrames = new DepthFrame[frames];
    }
    if (!mFreestyleMoveFrames) {
        mFreestyleMoveFrames = new FreestyleMoveFrame[frames];
    }
}

void FreestyleMove::RecordSkeletonFrame(BaseSkeleton *skeleton, int i2, float f3) {
    FreestyleMoveFrame frame;
    frame.skeleton.Init();
    frame.unk2d8 = f3;
    if (skeleton && skeleton->IsTracked()) {
        frame.skeleton.Set(*skeleton);
    }
    mFreestyleMoveFrames[i2] = frame;
}

void FreestyleMove::CalcCentering(int i1) {
    DepthFrame *whichFrame = &mDepthFrames[i1];
    float f12 = 0;
    int i10 = 0;
    int i170[80];
    memset(i170, 0, sizeof(i170));

    for (int i = 0; i < 80; i++) {
        for (int j = 0; j < 60; j++) {
            bool b = whichFrame[i].structs[j].unk0;
            if (b) {
                i10++;
                i170[i]++;
                f12 += b;
            }
        }
    }
    unk14 = f12 / (float)i10;

    int i7 = 0;
    int i9 = 0;
    for (int i = 0; i < DIM(i170); i++) {
        i7 += i170[i];
        i9 += i170[i] * i;
    }
    if (i9 != 0) {
        i9 /= i7;
    }
    unk10 = i9 - 40;
}
