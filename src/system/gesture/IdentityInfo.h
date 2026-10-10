#pragma once

class IdentityInfo {
public:
    IdentityInfo()
        : unk0(0), mEnrollmentIdx(-1), mProfileMatched(0), unk9(0), mSkeletonIdx(-1) {}
    void PostUpdate();

    bool ProfileMatched() const { return mProfileMatched; }
    int EnrollmentIndex() const { return mEnrollmentIdx; }
    void Init(int skeletonIdx) { mSkeletonIdx = skeletonIdx; }
    void SetUnk0(bool b1) { unk0 = b1; }
    void SetProfileMatched(bool b) { mProfileMatched = b; }
    void SetEnrollmentIndex(int i) { mEnrollmentIdx = i; }
    void SetUnk9(bool b) { unk9 = b; }

private:
    void Identified(unsigned int);

    bool unk0;
    int mEnrollmentIdx; // 0x4
    bool mProfileMatched; // 0x8
    bool unk9; // 0x9
    int mSkeletonIdx; // 0xc
};
