#include "rnddx9/Cam.h"
#include "rnddx9/Rnd.h"

DxCam::DxCam() {}

uint DxCam::ProjectZ(float f) {
    float neared = f - mNearPlane;
    float delta = mFarPlane - mNearPlane;
    float z_range_delta = mZRange.y - mZRange.x;
    neared /= f;
    float f0 = mFarPlane / mNearPlane;
    f0 *= neared;
    f0 *= z_range_delta;
    f0 += mZRange.x;
    if (TheDxRnd.Unk301() != 0) {
        f0 = 1.0f - f0;
    }
    return f0 * 16777215.0f;
}
