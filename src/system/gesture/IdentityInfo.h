#pragma once

class IdentityInfo {
public:
    IdentityInfo()
        : mJustIdentified(0), mEnrollmentIndex(-1), mProfileMatched(0),
          mEnrollmentUpdated(0), mSkeletonIdx(-1) {}
    void PostUpdate();

    bool mJustIdentified; // 0x0
    int mEnrollmentIndex; // 0x4
    bool mProfileMatched; // 0x8
    bool mEnrollmentUpdated; // 0x9
    int mSkeletonIdx; // 0xc

private:
    void Identified(unsigned int enrollmentIndex);
};
