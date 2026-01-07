#pragma once
#include "MQSongSort.h"
#include "NavListSortMgr.h"
#include "ui/UIListProvider.h"

class MQSongSortMgr : public NavListSortMgr {
public:
    static void Init(SongPreview &);
    virtual bool SelectionIs(Symbol);
    virtual Symbol MoveOn();
    virtual void OnEnter();

    void UpdateList();
    bool IsCharacter(Symbol) const;
    bool IsSong(Symbol) const;

    std::map<Symbol, std::vector<Symbol> > GetUnk78() { return unk78; }

private:
    MQSongSortMgr(SongPreview &);
    virtual ~MQSongSortMgr();

protected:
    // std::set<Symbol> unk78; // 0x78
    std::map<Symbol, std::vector<Symbol> > unk78; // 0x78
    // double *unk7c; // 0x7c
    // double *unk80;
    // double *unk84;
    // double *unk88;
    // bool unk8c; // 0x8c
    std::vector<Symbol> unk90; // 0x90
};

extern MQSongSortMgr *TheMQSongSortMgr;
