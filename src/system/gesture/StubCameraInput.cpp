#include "StubCameraInput.h"

StubCameraInput::StubCameraInput() {
    mCachedFrame.mUpVector.Set(0.0, 1.0, 0.0);
    mCachedFrame.mFloorPlane.Set(0.04584, 0.991929, 0.118222, 0.786);
    mCachedFrame.mFrameNumber = 0;
    mCachedFrame.mElapsedMs = 33;
}

void StubCameraInput::StubSkeletonFrame(SkeletonFrame &frame) {
    frame.mUpVector.Set(0.0, 1.0, 0.0);
    frame.mFloorPlane.Set(0.04584, 0.991929, 0.118222, 0.786);
    frame.mFrameNumber = 0;
    frame.mElapsedMs = 33;
}

void StubCameraInput::StubSkeletonData(SkeletonData &data, const Vector3 &vec) {
    data.mQualityFlags = 0;
    data.mTrackingID = 0;
    data.mEnrollmentIndex = 0;
    data.mTracking = kSkeletonTracked;
    data.mCenter.Set(0.0, 0.5, 2.3);
    data.mCenter.Set(data.mCenter.x + vec.x, vec.y + 0.5f, vec.z + 2.3f);
    data.mJointPositions[0].Set(0.126847f, 0.111759f, 2.264718f);
    data.mJointPositions[1].Set(0.127627f, 0.178946f, 2.32085f);
    data.mJointPositions[2].Set(0.13937f, 0.554082f, 2.307118f);
    data.mJointPositions[3].Set(0.140168f, 0.734258f, 2.252467f);
    data.mJointPositions[4].Set(-0.046645f, 0.45371f, 2.323363f);
    data.mJointPositions[5].Set(-0.135267f, 0.1511f, 2.341656f);
    data.mJointPositions[6].Set(-0.180013f, -0.101013f, 2.281592f);
    data.mJointPositions[7].Set(-0.185137f, -0.198124f, 2.261986f);
    data.mJointPositions[8].Set(0.3222f, 0.435505f, 2.330572f);
    data.mJointPositions[9].Set(0.393239f, 0.135313f, 2.341487f);
    data.mJointPositions[10].Set(0.435418f, -0.116715f, 2.272335f);
    data.mJointPositions[11].Set(0.440167f, -0.230645f, 2.230295f);
    data.mJointPositions[12].Set(0.041341f, 0.033312f, 2.248427f);
    data.mJointPositions[13].Set(-0.006339f, -0.477785f, 2.283425f);
    data.mJointPositions[14].Set(-0.041381f, -0.871678f, 2.312362f);
    data.mJointPositions[15].Set(0.207382f, 0.025153f, 2.258746f);
    data.mJointPositions[16].Set(0.221731f, -0.484165f, 2.313381f);
    data.mJointPositions[17].Set(0.22748f, -0.893713f, 2.355198f);
    data.mJointPositions[18].Set(-0.043792f, -0.917228f, 2.308891f);
    data.mJointPositions[19].Set(0.216633f, -0.932548f, 2.347959f);
    for (int i = 0; i < 20; i++) {
        data.mJointPositions[i].Set(
            data.mJointPositions[i].x + vec.x,
            data.mJointPositions[i].y + vec.y,
            data.mJointPositions[i].z + vec.z
        );
    }
}
