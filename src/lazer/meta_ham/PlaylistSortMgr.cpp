#include "meta_ham/PlaylistSortMgr.h"

#include "hamobj/HamGameData.h"
#include "HamProfile.h"
#include "NavListSort.h"
#include "NavListSortMgr.h"
#include "PassiveMessenger.h"
#include "PlaylistSort.h"
#include "ProfileMgr.h"
#include "SaveLoadManager.h"
#include "game/PartyModeMgr.h"
#include "macros.h"
#include "meta/SongPreview.h"
#include "meta_ham/FitnessGoalMgr.h"
#include "meta_ham/Playlist.h"
#include "net_ham/PlaylistJobs.h"
#include "net_ham/RCJobDingo.h"
#include "net_ham/RockCentral.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "os/ContentMgr.h"
#include "os/Debug.h"
#include "os/PlatformMgr.h"
#include "ui/UI.h"
#include "utl/DataPointMgr.h"
#include "utl/Std.h"
#include "utl/Symbol.h"
#include <list>

bool CompareType(const Playlist *p1, const Playlist *p2) {
    int p1type = p1->GetType();
    int p2type = p2->GetType();
    if (p1type == p2type) {
        bool p1custom = p1->IsCustom();
        bool p2custom = p2->IsCustom();
        for (int i = 0; i == 0; i = p1custom - p2custom) {
            if (p1custom == 0)
                break;
            p1custom++;
            p2custom++;
        }
    } else {
        int p1type = p1->GetType();
        int p2type = p2->GetType();
        return p2type <= p1type ? false : true;
    }
}

PlaylistSortMgr *ThePlaylistSortMgr;

PlaylistSortMgr::PlaylistSortMgr(SongPreview &sp) : NavListSortMgr(sp) {
    SetName("playlist_sort_mgr", ObjectDir::Main());
    mSorts.push_back(new PlaylistSortByType());
    static Symbol never_use("never_use");
    unk84.SetName(never_use);
    unkb0 = gNullStr;
    unkb8 = gNullStr;
    unkc8 = false;
}

PlaylistSortMgr::~PlaylistSortMgr() {}

void PlaylistSortMgr::Init(SongPreview &sp) {
    MILO_ASSERT(!ThePlaylistSortMgr, 0x1e);
    ThePlaylistSortMgr = new PlaylistSortMgr(sp);
    Callback *c;
    if (!ThePlaylistSortMgr) {
    }
    TheContentMgr.RegisterCallback(0, false);
}

bool PlaylistSortMgr::IsProfileChanged() {
    HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
    const char *name;
    if (pProfile) {
        name = pProfile->GetName();
    } else {
        name = gNullStr;
    }
    return unkb0 != name;
}

void PlaylistSortMgr::OnSmartGlassListen(int i) {
    if (i != 0) {
        ThePlatformMgr.AddSink(this, "smart_glass_msg");
    } else {
        ThePlatformMgr.RemoveSink(this, "smart_glass_msg");
    }
}

bool PlaylistSortMgr::HasValidProfile() {
    HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
    if (pProfile) {
        pProfile->UpdateOnlineID();
        if (pProfile->IsSignedIn()) {
            int padNum = pProfile->GetPadNum();
            if (ThePlatformMgr.IsSignedIntoLive(padNum) && TheRockCentral.IsOnline()) {
                unkb0 = pProfile->GetName();
                QueueCmdChangeProfileOnlineID(pProfile->GetOnlineID()->ToString());
                return true;
            }
        }
    }
    unkb0 = gNullStr;
    QueueCmdChangeProfileOnlineID(gNullStr);
    return false;
}

void PlaylistSortMgr::StartCmdGetPlaylistsFromRC() {
    HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
    if (pProfile && pProfile->IsSignedIn()) {
        int padNum = pProfile->GetPadNum();
        if (ThePlatformMgr.IsSignedIntoLive(padNum) && TheRockCentral.IsOnline()) {
            MILO_LOG("MY PORFILE ID: %s\n", unkb8.c_str());
            MILO_LOG("ACTUAL PORFILE ID: %s\n", pProfile->GetOnlineID()->ToString());
        }
    }
    unkcc = new GetPlaylistsJob(this, unkb8.c_str());
    TheRockCentral.ManageJob(unkcc);
}

void PlaylistSortMgr::FakeAddPlaylistsToRC() {
    HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
    if (pProfile) {
        pProfile->UpdateOnlineID();
        if (pProfile->IsSignedIn()) {
            int padNum = pProfile->GetPadNum();
            if (ThePlatformMgr.IsSignedIntoLive(padNum) && TheRockCentral.IsOnline()) {
                TheRockCentral.ManageJob(new AddPlaylistJob(
                    nullptr,
                    pProfile->GetOnlineID()->ToString(),
                    "test_babygotback",
                    "6025"
                ));
                TheRockCentral.ManageJob(new AddPlaylistJob(
                    nullptr,
                    pProfile->GetOnlineID()->ToString(),
                    "test_badromance",
                    "6020,6025"
                ));
                TheRockCentral.ManageJob(new AddPlaylistJob(
                    nullptr,
                    pProfile->GetOnlineID()->ToString(),
                    "test_ymca",
                    "7011,6020,6025"
                ));
                return;
            }
        }
    }
    MILO_LOG(
        "[PlaylistSortMgr::FakeAddPlaylistsToRC] No Valid Profile Available. Skipping This Cheat.\n"
    );
}

int PlaylistSortMgr::ConvertListIndexToPlaylistIndex(int listIndex) {
    int playlistIndex = 0;
    for (int i = 0; i < mHeadersB.size(); i++) {
        if (listIndex >= mHeadersB[i])
            playlistIndex--;
    }

    return playlistIndex + listIndex;
}

void PlaylistSortMgr::BroadcastSyncMsg(Symbol s) {
    Symbol sym = s;
    MILO_LOG("[PlaylistSortMgr::BroadcastSyncMsg] Broadcasting msg (%s).\n", sym);
    Message msg(sym);
    HandleType(msg);
    TheUI->Handle(msg, false);
}

void PlaylistSortMgr::OnEnter() {
    UpdateList();
    FOREACH (it, mSorts) {
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

void PlaylistSortMgr::StartCmdGetPlaylistFromRC() {
    QueueableCommand *cmd = unkc0.front();
    unkcc = new GetPlaylistJob(this, unkb8.c_str(), cmd->unk4.i);
    TheRockCentral.ManageJob(unkcc);
}

void PlaylistSortMgr::StartCmdAddPlaylistToRC() {
    QueueableCommand *cmd = unkc0.front();
    unkcc = new AddPlaylistJob(this, unkb8.c_str(), cmd->unk4.playlist);
    TheRockCentral.ManageJob(unkcc);
}

void PlaylistSortMgr::StartCmdDeletePlaylistFromRC() {
    QueueableCommand *cmd = unkc0.front();
    unkcc = new DeletePlaylistJob(this, unkb8.c_str(), cmd->unk4.i);
    TheRockCentral.ManageJob(unkcc);
}

void PlaylistSortMgr::StartCmdEditPlaylist() {
    QueueableCommand *cmd = unkc0.front();
    unkcc = new EditPlaylistJob(this, unkb8.c_str(), cmd->unk4.playlist);
    TheRockCentral.ManageJob(unkcc);
}

DataNode PlaylistSortMgr::OnMsg(SmartGlassMsg const &) {
    MILO_LOG("SmartGlass: I should update playlist options/song from RC\n");
    SendDataPoint("smartglass/playlist");
    QueueCmdGetPlaylistsFromRC();
    return 1;
}

void PlaylistSortMgr::QueueCmdAddPlaylistToRC(Playlist *pl) {
    CmdAddPlayListToRC *cmd = new CmdAddPlayListToRC(pl);
    unkc0.push_back(cmd);
    if (!unkc8) {
        ProcessNextCommand();
    }
}

void PlaylistSortMgr::QueueCmdDeletePlaylistFromRC(int i) {
    CmdDeletePlaylistFromRC *cmd = new CmdDeletePlaylistFromRC(i);
    unkc0.push_back(cmd);
    if (!unkc8) {
        ProcessNextCommand();
    }
}

void PlaylistSortMgr::QueueCmdEditPlaylist(Playlist *pl) {
    CmdEditPlaylist *cmd = new CmdEditPlaylist(pl);
    unkc0.push_back(cmd);
    if (!unkc8) {
        ProcessNextCommand();
    }
}

void PlaylistSortMgr::ProcessNextCommand() {
    int i = 0;
    for (auto it = (unkc0).begin(); it != (unkc0).end(); (++it), i++) {
        if (i != 0) {
            unkc8 = true;
            i = (*it)->unk4.i;
            if (i == 0) {
                HandleCmdChangeProfileOnlineID();
                return;
            }
            if (i == 1) {
                StartCmdGetPlaylistsFromRC();
                return;
            }
            if (i == 2) {
                HandleCmdResolvePlaylists();
                return;
            }
            if (i == 3) {
                StartCmdGetPlaylistFromRC();
                return;
            }
            if (i == 4) {
                StartCmdAddPlaylistToRC();
                return;
            }
            if (i == 5) {
                StartCmdEditPlaylist();
                return;
            }
            if (i == 6) {
                StartCmdDeletePlaylistFromRC();
                return;
            }
        }
    }
    unkc8 = false;
}

void PlaylistSortMgr::ResolvePlaylists() {
    auto activeProfile = TheProfileMgr.GetActiveProfile(true);
    Playlist *playlist;
    if (activeProfile) {
        const char *profileName = activeProfile->GetName();
        bool flag = unkb0 != profileName;
        if (!flag) {
            for (int i = 0; i < unkd0.size(); i++) {
                *playlist = activeProfile->GetPlaylist(i);
                auto cusPlaylist = dynamic_cast<CustomPlaylist *>(playlist);
                cusPlaylist->Copy(&unkd0[i]);
                cusPlaylist->SetParentProfile(activeProfile);
            }
        }
        for (int i = 0; i < 5; i++) {
            *playlist = activeProfile->GetPlaylist(i);
            int numSongs = playlist->GetNumSongs();
            for (int i = 0; i < numSongs; i++) {
                playlist->RemoveSong();
            }
        }
        if (TheSaveLoadMgr) {
            TheSaveLoadMgr->AutoSave();
        }
        BroadcastSyncMsg("playlists_synced");
        if (unkd0.size() == 0) {
            return;
        }
        SendPassiveMsg("playlist_syned_with_rc");
        return;
    }
    BroadcastSyncMsg("sync_failed");
}

void PlaylistSortMgr::HandleCmdDeletePlaylistFromRC() {
    MILO_LOG("===== HandleCmdDeletePlaylistFromRC\n");
    unkcc = nullptr;
    RELEASE(unkc0.front());
    unkc0.pop_front();
    ProcessNextCommand();
}

void PlaylistSortMgr::HandleCmdAddPlaylistToRC() {
    MILO_LOG("===== HandleCmdAddPlaylistToRC\n");
    ((AddPlaylistJob *)unkcc)->GetPlaylistID(unkc0.front()->unk4.customPlaylist);
    unkcc = nullptr;
    RELEASE(unkc0.front());
    unkc0.pop_front();
    ProcessNextCommand();
}

void PlaylistSortMgr::HandleCmdResolvePlaylists() {
    MILO_LOG("===== HandleCmdResolvePlaylists\n");
    ResolvePlaylists();
    RELEASE(unkc0.front());
    unkc0.pop_front();
    ProcessNextCommand();
}

void PlaylistSortMgr::HandleCmdEditPlaylist() {
    unkcc = nullptr;
    RELEASE(unkc0.front());
    unkc0.pop_front();
    ProcessNextCommand();
}

BEGIN_HANDLERS(PlaylistSortMgr)
    HANDLE_EXPR(has_valid_profile, HasValidProfile())
    HANDLE_EXPR(is_profile_changed, IsProfileChanged())
    HANDLE_ACTION(get_playlists_from_rc, QueueCmdGetPlaylistsFromRC())
    HANDLE_ACTION(update_curr_playlist_with_rc, UpdateCurrPlaylistWithRC())
    HANDLE_ACTION(fake_add_playlists_to_rc, FakeAddPlaylistsToRC())
    HANDLE_ACTION(smart_glass_listen, OnSmartGlassListen(_msg->Int(2)))
    HANDLE_MESSAGE(RCJobCompleteMsg)
    HANDLE_MESSAGE(SmartGlassMsg) HANDLE_SUPERCLASS(NavListSortMgr)
END_HANDLERS
