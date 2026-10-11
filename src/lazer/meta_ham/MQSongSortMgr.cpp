#include "MQSongSortMgr.h"

#include "Campaign.h"
#include "NavListSortMgr.h"
#include "hamobj/HamGameData.h"
#include "HamSongMgr.h"
#include "MQSongSortByCharacter.h"
#include "MQSongSortNode.h"
#include "ProfileMgr.h"
#include "meta_ham/ProfileMgr.h"
#include "obj/Dir.h"
#include "utl/Std.h"
#include "utl/Symbol.h"

MQSongSortMgr::MQSongSortMgr(SongPreview &sp) : NavListSortMgr(sp) {
    SetName("mq_song_provider", ObjectDir::Main());
    mSorts.push_back(new MQSongSortByCharacter());
}

MQSongSortMgr::~MQSongSortMgr() {};

MQSongSort::MQSongSort() {};

void MQSongSortMgr::Init(SongPreview &preview) {
    MILO_ASSERT(!TheMQSongSortMgr, 0x18);
    TheMQSongSortMgr = new MQSongSortMgr(preview);
    TheContentMgr.RegisterCallback(TheMQSongSortMgr, false);
}

void MQSongSortMgr::OnEnter() {
    mSortWithHeaders = true;
    UpdateList();
    FOREACH (it, mSorts) {
        (*it)->BuildTree();
    }
    NavListSort *sort = mSorts[mSortIndex];
    sort->BuildItemList();
    if (mSavedHighlight) {
        sort->SetHighlightID(mSavedHighlightID);
        mSavedHighlight = false;
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
        return dynamic_cast<MQSongSortNode *>(GetHighlightItem()) != nullptr;
    } else if (sym == header) {
        return dynamic_cast<MQSongHeaderNode *>(GetHighlightItem()) != nullptr;
    }
    return false;
}

bool MQSongSortMgr::IsCharacter(Symbol sym) const {
    FOREACH (it, m_songsPerCharacter) {
        if (it->first == sym) {
            return true;
        }
    }
    return false;
}

void MQSongSortMgr::UpdateList() {
    MILO_ASSERT(TheCampaign, 0x6e);
    m_vSongs.clear();
    Symbol mqCrew = TheCampaign->GetMasterQuestCrew();
    m_songsPerCharacter.clear();
    const std::vector<int> &rankedSongs = TheHamSongMgr.RankedSongs((SongType)1);
    FOREACH_CONST (it, rankedSongs) {
        const HamSongMetadata *metadata = TheHamSongMgr.Data(*it);
        Symbol character = GetOutfitCharacter(metadata->Outfit());
        Symbol crew = GetCrewForCharacter(character);
        Symbol temp = character;
        Symbol mqHeader = MakeString("mqheader_%s", temp);
        if (!metadata->IsFake() && crew == mqCrew
            && TheProfileMgr.IsContentUnlocked(metadata->ShortName())) {
            m_songsPerCharacter[mqHeader].push_back(
                TheHamSongMgr.GetShortNameFromSongID(*it)
            );
        }
    }
    FOREACH (it, m_songsPerCharacter) {
        auto &vec = it->second;
        m_vSongs.push_back(it->first);
        FOREACH (it2, vec) {
            m_vSongs.push_back(*it2);
        }
    }
}

bool MQSongSortMgr::IsSong(Symbol sym) const {
    for (auto it = m_songsPerCharacter.begin();
         it != m_songsPerCharacter.end() && it->first != sym;
         ++it) {
        std::vector<Symbol> syms = it->second;
        FOREACH (it2, syms) {
            if (*it2 == sym) {
                return true;
            }
        }
    }
    return false;
}
