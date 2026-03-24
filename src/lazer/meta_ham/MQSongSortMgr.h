#pragma once
#include "MQSongSort.h"
#include "NavListSortMgr.h"
#include "stl/_vector.h"
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
    std::vector<Symbol> &GetUnk90() { return unk90; }
    const std::vector<Symbol> &GetVecAt(Symbol key) { return unk78[key]; }

private:
    MQSongSortMgr(SongPreview &sp);
    virtual ~MQSongSortMgr();

protected:
    std::map<Symbol, std::vector<Symbol> > unk78; // 0x78
    std::vector<Symbol> unk90; // 0x90
};

extern MQSongSortMgr *TheMQSongSortMgr;
