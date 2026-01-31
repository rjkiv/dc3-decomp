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

    void SetArt(Symbol s) { mArt = s; }
    Symbol GetArtName() const { return mArt; }

private:
    Symbol mName; // 0x4
    int unk8; // 0x8
    bool mIsSecret; // 0xc
    bool mIsSilent; // 0xd
    std::list<AwardEntry> mAwardEntries; // 0x10
    Symbol mArt; // 0x18

protected:
    virtual void Configure(DataArray *);

    void GrantAward(AwardEntry const &, HamProfile *);
};
