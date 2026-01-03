#pragma once
#include "ChallengeSortMgr.h"
#include "NavListNode.h"

class ChallengeHeaderNode : public NavListHeaderNode {
public:
    ChallengeHeaderNode(NavListItemSortCmp *, Symbol, bool);
    virtual void Text(UIListLabel *, UILabel *) const;
    virtual DataNode Handle(DataArray *, bool);
    virtual Symbol OnSelect();
    virtual Symbol Select();
    virtual Symbol OnSelectDone();
    virtual void OnHighlight();
    virtual NavListSortNode *GetFirstActive();
    virtual bool IsActive() const;
    virtual const char *GetAlbumArtPath();
    virtual void SetCollapseStateIcon(bool) const;
    virtual void Renumber(stlpmtx_std::vector<NavListSortNode *> &);

    int GetChallengeExp();
    int GetPotentialChallengeExp(NavListSortNode *);
    int GetTotalEarnedExp(int);
    int GetSongID();
    Symbol GetSongShortName();
    String GetSongShortTitle();

protected:
    int unk58; // 0x58
};

class ChallengeSortNode : public NavListItemNode {
public:
    ChallengeSortNode(NavListItemSortCmp *, ChallengeRecord *); // impl in ChallengeSortByScore
    virtual Symbol GetToken() const;
    virtual void Text(UIListLabel *, UILabel *) const;
    virtual void Custom(UIListCustom *, Hmx::Object *) const;
    virtual Symbol OnSelect();
    virtual Symbol Select();
    virtual void OnContentMounted(const char *, const char *);


    int GetChallengeExp();
    const char *GetChallengerGamertag();
    int GetChallengeScore();
    int GetSongID();
    int GetChallengerXp();
    void SetMedalIcon(UILabel *) const;
    void SetNewIcon(UILabel *) const;
    void SetBuyIcon(UILabel *) const;

    ChallengeRecord *GetChallengeRecord() { return mChallengeRecord; };

private:
    int GetPlayerSide() const;

protected:
    ChallengeRecord *mChallengeRecord; // 0x48
};

