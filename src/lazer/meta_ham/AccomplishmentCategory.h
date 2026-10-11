#pragma once

#include "lazer/meta_ham/Award.h"
#include "obj/Data.h"
#include "utl/Symbol.h"
class AccomplishmentCategory {
public:
    virtual ~AccomplishmentCategory();

    AccomplishmentCategory(DataArray const *, int);
    Symbol GetName() const;
    Symbol GetAward() const;
    Symbol GetGroup() const;
    bool HasAward() const;

protected:
    void Configure(DataArray const *);

private:
    int m_iIndex; // 0x4
    Symbol m_symName; // 0x8
    Symbol m_symGroup; // 0xc
    Symbol m_symAward; // 0x10
};
