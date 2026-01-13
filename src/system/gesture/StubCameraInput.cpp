#include "StubCameraInput.h"

StubCameraInput::StubCameraInput() {
    mCachedFrame.unk8.Set(0.0, 1.0, 0.0);
    mCachedFrame.unk18.Set(0.04584, 0.991929, 0.118222, 0.786);
    mCachedFrame.unk0 = 0;
    mCachedFrame.mElapsedMs = 33;
}

void StubCameraInput::StubSkeletonFrame(SkeletonFrame &frame) {
    frame.unk8.Set(0.0, 1.0, 0.0);
    frame.unk18.Set(0.04584, 0.991929, 0.118222, 0.786);
    frame.unk0 = 0;
    frame.mElapsedMs = 33;
}

void StubCameraInput::StubSkeletonData(SkeletonData &data, const Vector3 &vec) {
    data.mQualityFlags = 0;
    data.mTrackingID = 0;
    data.unk2dc = 0;
    data.mTracking = kSkeletonTracked;
    data.unk2e0.Set(0.0, 0.5, 2.3);
    data.unk2e0.Set(data.unk2e0.x + vec.x, vec.y + 0.5f, vec.z + 2.3f);
    data.unk144[0].Set(0.126847f, 0.111759f, 2.264718f);
    data.unk144[1].Set(0.127627f, 0.178946f, 2.32085f);
    data.unk144[2].Set(0.13937f, 0.554082f, 2.307118f);
    data.unk144[3].Set(0.140168f, 0.734258f, 2.252467f);
    data.unk144[4].Set(-0.046645f, 0.45371f, 2.323363f);
    data.unk144[5].Set(-0.135267f, 0.1511f, 2.341656f);
    data.unk144[6].Set(-0.180013f, -0.101013f, 2.281592f);
    data.unk144[7].Set(-0.185137f, -0.198124f, 2.261986f);
    data.unk144[8].Set(0.3222f, 0.435505f, 2.330572f);
    data.unk144[9].Set(0.393239f, 0.135313f, 2.341487f);
    data.unk144[10].Set(0.435418f, -0.116715f, 2.272335f);
    data.unk144[11].Set(0.440167f, -0.230645f, 2.230295f);
    data.unk144[12].Set(0.041341f, 0.033312f, 2.248427f);
    data.unk144[13].Set(-0.006339f, -0.477785f, 2.283425f);
    data.unk144[14].Set(-0.041381f, -0.871678f, 2.312362f);
    data.unk144[15].Set(0.207382f, 0.025153f, 2.258746f);
    data.unk144[16].Set(0.221731f, -0.484165f, 2.313381f);
    data.unk144[17].Set(0.22748f, -0.893713f, 2.355198f);
    data.unk144[18].Set(-0.043792f, -0.917228f, 2.308891f);
    data.unk144[19].Set(0.216633f, -0.932548f, 2.347959f);
    for (int i = 0; i < 20; i++) {
        data.unk144[i].Set(data.unk144[i].x + vec.x, data.unk144[i].y + vec.y, data.unk144[i].z + vec.z);
    }
}