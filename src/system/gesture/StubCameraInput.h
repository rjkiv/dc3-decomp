#pragma once
#include "CameraInput.h"

class StubCameraInput : public CameraInput {
public:
    StubCameraInput();
    static void StubSkeletonFrame(SkeletonFrame &);
    static void StubSkeletonData(SkeletonData &, const Vector3 &);

protected:
    struct SkeletonStatus {
        bool mTracked; // 0x0
        Vector3 mCenterOffsetMeters; // 0x4
    };

    const SkeletonFrame *PollNewFrame();

    SkeletonFrame mFrame; // 0x11d4
    SkeletonStatus mSkeletonOffsets[6]; // 0x239c
};
