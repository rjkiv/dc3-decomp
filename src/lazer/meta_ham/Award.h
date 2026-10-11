#pragma once
#include "obj/Data.h"
#include "utl/Symbol.h"

struct AwardEntry {
    Symbol m_symAwardCategory; // 0x0
    Symbol m_symAward; // 0x4
};

class Award {
public:
    Award(DataArray *, int);
    virtual ~Award();

    const char *GetArt() const;
    void GrantAwards(class HamProfile *);
    Symbol GetDisplayName() const;
    Symbol GetName() const;
    bool IsSilent() const;

    bool HasArt() const { return m_symArt != gNullStr; }

protected:
    virtual void Configure(DataArray *);

    void GrantAward(AwardEntry const &, HamProfile *);

private:
    Symbol m_symName; // 0x4
    int m_iIndex; // 0x8
    bool m_bIsSecret; // 0xc
    bool m_bIsSilent; // 0xd
    std::list<AwardEntry> m_lAwards; // 0x10
    Symbol m_symArt; // 0x18
};
