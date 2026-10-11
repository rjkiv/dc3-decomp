#pragma once
#include "MQSongSort.h"
#include "NavListSortMgr.h"
#include "ui/UIListProvider.h"
#include "utl/Symbol.h"

class MQSongSortMgr : public NavListSortMgr {
public:
    static void Init(SongPreview &);
    virtual bool SelectionIs(Symbol);
    virtual Symbol MoveOn();
    virtual void OnEnter();

    void UpdateList();
    bool IsCharacter(Symbol) const;
    bool IsSong(Symbol) const;

    std::map<Symbol, std::vector<Symbol> > m_songsPerCharacter; // 0x78
    std::vector<Symbol> m_vSongs; // 0x90

private:
    MQSongSortMgr(SongPreview &sp);
    virtual ~MQSongSortMgr();
};

extern MQSongSortMgr *TheMQSongSortMgr;
