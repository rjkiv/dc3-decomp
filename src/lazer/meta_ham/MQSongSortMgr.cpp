#include "MQSongSortMgr.h"

#include "Campaign.h"
#include "hamobj/HamGameData.h"
#include "HamSongMgr.h"
#include "MQSongSortByCharacter.h"
#include "MQSongSortNode.h"
#include "ProfileMgr.h"

// MQSongSortMgr::MQSongSortMgr(SongPreview &) {};

MQSongSortByCharacter::MQSongSortByCharacter() {
    static Symbol by_character("by_character");
    mSortName = by_character;
}

MQSongSortMgr::~MQSongSortMgr() {};

MQSongSort::MQSongSort() {};

void MQSongSortMgr::Init(SongPreview &preview) {
    MILO_ASSERT(!TheMQSongSortMgr, 0x18);
    TheMQSongSortMgr = new MQSongSortMgr(preview);
    TheContentMgr.RegisterCallback(TheMQSongSortMgr, false);
}

void MQSongSortMgr::OnEnter() {
    mHeadersSelectable = true;
    UpdateList();
    FOREACH(it, mSorts) {
        (*it)->BuildTree();
    }
    NavListSort *sort = mSorts[mCurrentSortIdx];
    sort->BuildItemList();
    if (unk48) {
        sort->SetHighlightID(unk44);
        unk48 = false;
    }
    sort->UpdateHighlight();
}

Symbol MQSongSortMgr::MoveOn() {
    MILO_ASSERT(0, 0x45);
    return gNullStr;
}

bool MQSongSortMgr::SelectionIs(Symbol sym) {
    static Symbol challenge("challenge");
    static Symbol header("header");
    if (sym == challenge) {
        NavListSortNode *highlightItem = GetHighlightItem();
        return dynamic_cast<MQSongSortNode *>(highlightItem) == 0;
    }
    if (sym != header) {
        return false;
    }
    NavListSortNode *highlightItem = GetHighlightItem();
    return dynamic_cast<MQSongHeaderNode *>(highlightItem) == 0;
}

bool MQSongSortMgr::IsCharacter(Symbol sym) const {
    FOREACH(it, unk78) {
        if (it->first == sym) {
            return true;
        }
    }
    return false;
}

void MQSongSortMgr::UpdateList() {
    MILO_ASSERT(TheCampaign, 0x6e);
    if (!unk90.empty()) {
        unk90.clear();
    }
    Symbol mqCrew = TheCampaign->GetMQCrew();
    unk78.clear();
    std::vector<int> rankedSongs = TheHamSongMgr.RankedSongs((SongType)1);
    FOREACH(it, rankedSongs) {
        const HamSongMetadata *metadata = TheHamSongMgr.Data(*it);
        Symbol character = GetOutfitCharacter(metadata->Outfit(), true);
        Symbol crew = GetCrewForCharacter(character, true);
        Symbol mqHeader = MakeString<char>("mqheader_%s", character);
        if (!metadata->IsFake() && crew == mqCrew && TheProfileMgr.IsContentUnlocked(metadata->ShortName())) {
            unk78[mqHeader].push_back(TheHamSongMgr.GetShortNameFromSongID(*it));
        }
    }
    FOREACH(it, unk78) {
        unk90.push_back(it->first);
        FOREACH(it2, it->second) {
            unk90.push_back(*it2);
        }
    }
}

bool MQSongSortMgr::IsSong(Symbol sym) const {
    for (auto it = unk78.begin(); it != unk78.end() && it->first != sym; ++it) {
        std::vector<Symbol> syms = it->second;
        FOREACH(it2, syms) {
            if (*it2 == sym) {
                return true;
            }
        }
    }
    return false;
}