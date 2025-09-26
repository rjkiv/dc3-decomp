#pragma once
#include "MoveGraph.h"
#include "hamobj/SongLayout.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include <map>
#include <set>

class CategoryData {
public:
    Symbol unk0;
    Symbol unk4;
};

class MoveChoiceSet {
public:
};

class MoveMgr : public Hmx::Object {
protected:
    MoveMgr();
    // Hmx::Object
    virtual ~MoveMgr();

public:
    virtual DataNode Handle(DataArray *, bool);

    void RegisterSongLayout(SongLayout *);
    void UnRegisterSongLayout(SongLayout *);
    Symbol PickRandomCategory();
    void
    GenerateMoveChoice(Symbol, std::vector<const MoveVariant *> &, std::vector<const MoveVariant *> &);

protected:
    int unk2c;
    int unk30;
    int unk34;
    int unk38;
    int unk3c;
    SongLayout *unk40;
    SongLayout *unk44;
    int unk48;
    int unk4c;
    int unk50;
    std::map<int, MoveVariant *> unk54[3];
    int unk9c; // 0x9c - MoveDir*
    int unka0;
    MoveGraph unka4;
    std::set<const MoveVariant *> unk104;
    std::vector<const MoveParent *> unk11c[2];
    std::vector<const MoveVariant *> unk134[2];
    Symbol unk14c;
    std::vector<std::pair<const MoveVariant *, const MoveVariant *> > unk150[2];
    bool unk168;
    std::vector<MoveChoiceSet> unk16c;
    std::vector<CategoryData> unk178;
    std::vector<CategoryData> unk184;
    std::vector<CategoryData> unk190;
    std::vector<CategoryData> unk19c;
    ObjectDir *unk1a8;
    int unk1ac; // 0x1ac - SuperEasyRemixer*
};

extern MoveMgr *TheMoveMgr;
