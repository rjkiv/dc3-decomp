#pragma once

// size 0x44
struct MemPointDelta {
    friend struct MemPoint;

public:
    MemPointDelta();
    MemPointDelta &operator+=(const MemPointDelta &);
    bool AnyGreaterThan(int) const;
    bool AnyGreaterThanKB(int) const;
    void AssertNoneGreaterThan(int) const;
    void AssertNoneGreaterThanKB(int) const;
    const char *ToString(int) const;
    const char *ToStringKB() const;

    static const char *HeaderString(const char *);
    static const char *HeaderStringKB();

private:
    int mHeapDelta[16]; // 0x0
    int mPhysDelta; // 0x40
};

struct MemPoint {
public:
    enum eInitType {
        kZeroed = 0,
        kSnapshot = 1
    };

    MemPoint(eInitType = kSnapshot);
    MemPointDelta operator-(const MemPoint &);

private:
    int mHeapFree[16]; // 0x0
    int mPhysUsed; // 0x40
};
