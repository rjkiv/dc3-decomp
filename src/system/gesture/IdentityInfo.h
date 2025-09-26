#pragma once

class IdentityInfo {
public:
    IdentityInfo() : unk0(0), mEnrollmentIdx(-1), mProfileMatched(0), unk9(0), unkc(-1) {}
    void PostUpdate();

    bool ProfileMatched() const { return mProfileMatched; }
    int EnrollmentIndex() const { return mEnrollmentIdx; }
    void Init() { unkc = 0; }

private:
    void Identified(unsigned int);

    bool unk0;
    int mEnrollmentIdx; // 0x4
    bool mProfileMatched; // 0x8
    bool unk9; // 0x9
    int unkc; // 0xc
};
