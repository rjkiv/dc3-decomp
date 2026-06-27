#pragma once
#include "utl/BinStream.h"

class HxGuid {
protected:
    int mData[4];
    friend BinStream &operator<<(BinStream &bs, const HxGuid &hx);
    friend BinStream &operator>>(BinStream &bs, HxGuid &hx);

public:
    HxGuid();

    void Generate();
    void Clear();
    bool IsNull() const;
    bool operator==(const HxGuid &) const;
    bool operator!=(const HxGuid &guid) const { return !(*this == guid); }

    bool operator<(const HxGuid &) const;
    int Chunk32(int) const;
    const char *ToString() const;
    const int *Data() const;

    static int SaveSize();
};

BinStream &operator<<(BinStream &, const HxGuid &);
BinStream &operator>>(BinStream &, HxGuid &);

class UserGuid : public HxGuid {};
