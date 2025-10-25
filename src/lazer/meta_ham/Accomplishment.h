#pragma once

#include "obj/Data.h"
#include "ui/UILabel.h"
#include "utl/Symbol.h"
class Accomplishment {
public:
    virtual ~Accomplishment();
    virtual bool ShowBestAfterEarn() const;
    virtual void UpdateIncrementalEntryName(UILabel *, Symbol);
    virtual bool CanBeLaunched() const;

    Accomplishment(DataArray *, int);
    Symbol GetCategory() const;
    bool HasGamerpicReward() const;
    bool HasAvatarAssetReward() const;
    Symbol GetAward() const;
    bool HasAward() const;
    bool IsSecondaryGoal() const;
    bool IsDynamic() const;
    char const *GetIconArt() const;

    Symbol unk4;
    std::vector<Symbol> unk8;
    int unk14;
    Symbol mCategory; // 0x18
    Symbol mAward; // 0x1c
    Symbol unk20;
    int unk24;
    Symbol unk28;
    int unk2c;
    bool unk30;
    int unk34;
    int unk38;
    int unk3c;
    std::vector<Symbol> unk40;
    int unk4c;
    int unk50;
    int unk54;
    bool unk58;
    bool unk59;
    int unk5c;
    int unk60;
    bool unk64;
    bool unk65;
    bool isSecondaryGoal; // 0x66
    bool unk67;

private:
    void Configure(DataArray *);
};
