#pragma once

// size 0x44
struct MemPointDelta {
public:
    MemPointDelta();
    MemPointDelta &operator+=(const MemPointDelta &);
    bool AnyGreaterThan(int) const;
    const char *ToString(int) const;

    static const char *HeaderString(const char *);

    int unk0[16]; // 0x0
    int unk40; // 0x40
};

struct MemPoint {
public:
    enum eInitType {
        kInitType0,
        kInitType1
    };

    MemPoint(eInitType = kInitType1);
    MemPointDelta operator-(const MemPoint &);

    int unk0[16]; // 0x0
    int unk40; // 0x40
};
