#pragma once
#include "CameraInput.h"

class StubCameraInput : public CameraInput {
public:
    StubCameraInput();
    static void StubSkeletonFrame(SkeletonFrame &);
    static void StubSkeletonData(SkeletonData &, const Vector3 &);

protected:
    const SkeletonFrame *PollNewFrame();
    const SkeletonFrame unk11d4; // 0x11d4
};
