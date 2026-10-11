#pragma once

#include "obj/Data.h"
#include "utl/Symbol.h"
class AccomplishmentGroup {
public:
    virtual ~AccomplishmentGroup();

    AccomplishmentGroup(DataArray *, int);
    bool HasAward() const;
    Symbol GetName() const;
    Symbol GetAward() const;

protected:
    virtual void Configure(DataArray *);

private:
    Symbol m_symName; // 0x4
    int m_iIndex; // 0x8
    Symbol m_symAward; // 0xc
};
