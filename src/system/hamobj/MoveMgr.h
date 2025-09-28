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
    int unk0;
    int unk4;
    int unk8;
    int unkc;
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
    const std::map<Symbol, MoveParent *> &MoveParents() const {
        return mMoveGraph.MoveParents();
    }
    const std::map<Symbol, MoveVariant *> &MoveVariants() const {
        return mMoveGraph.MoveVariants();
    }
    void Clear();
    bool HasRoutine() const;
    void InsertMoveInSong(const MoveVariant *, int, int);
    void SaveRoutine(DataArray *) const;

    static void Init(const char *);

protected:
    int unk2c[3];
    int unk38;
    int unk3c;
    SongLayout *unk40;
    SongLayout *unk44;
    int unk48[3];
    std::map<int, MoveVariant *> unk54[3];
    int unk9c; // 0x9c - MoveDir*
    int unka0;
    MoveGraph mMoveGraph; // 0xa4
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
