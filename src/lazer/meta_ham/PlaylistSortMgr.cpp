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
#include "hamobj/HamPlayerData.h"
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
    TheContentMgr.RegisterCallback(ThePlaylistSortMgr, false);
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
    CmdGetPlaylistFromRC *cmd = (CmdGetPlaylistFromRC *)unkc0.front();
    unkcc = new GetPlaylistJob(this, unkb8.c_str(), cmd->num);
    TheRockCentral.ManageJob(unkcc);
}

void PlaylistSortMgr::StartCmdAddPlaylistToRC() {
    CmdAddPlaylistToRC *cmd = (CmdAddPlaylistToRC *)unkc0.front();
    unkcc = new AddPlaylistJob(this, unkb8.c_str(), cmd->playlist);
    TheRockCentral.ManageJob(unkcc);
}

void PlaylistSortMgr::StartCmdDeletePlaylistFromRC() {
    CmdDeletePlaylistFromRC *cmd = (CmdDeletePlaylistFromRC *)unkc0.front();
    unkcc = new DeletePlaylistJob(this, unkb8.c_str(), cmd->num);
    TheRockCentral.ManageJob(unkcc);
}

void PlaylistSortMgr::StartCmdEditPlaylist() {
    CmdEditPlaylist *cmd = (CmdEditPlaylist *)unkc0.front();
    unkcc = new EditPlaylistJob(this, unkb8.c_str(), cmd->playlist);
    TheRockCentral.ManageJob(unkcc);
}

DataNode PlaylistSortMgr::OnMsg(SmartGlassMsg const &) {
    MILO_LOG("SmartGlass: I should update playlist options/song from RC\n");
    SendDataPoint("smartglass/playlist");
    QueueCmdGetPlaylistsFromRC();
    return 1;
}

void PlaylistSortMgr::QueueCmdAddPlaylistToRC(Playlist *pl) {
    CmdAddPlaylistToRC *cmd = new CmdAddPlaylistToRC((CustomPlaylist *)pl);
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
    CmdEditPlaylist *cmd = new CmdEditPlaylist((CustomPlaylist *)pl);
    unkc0.push_back(cmd);
    if (!unkc8) {
        ProcessNextCommand();
    }
}

void PlaylistSortMgr::QueueCmdResolvePlaylists() {
    CmdResolvePlaylists *cmd = new CmdResolvePlaylists();
    unkc0.push_back(cmd);
    if (!unkc8) {
        ProcessNextCommand();
    }
}

void PlaylistSortMgr::QueueCmdGetPlaylistFromRC(int i) {
    CmdGetPlaylistFromRC *cmd = new CmdGetPlaylistFromRC(i);
    unkc0.push_back(cmd);
    if (!unkc8) {
        ProcessNextCommand();
    }
}

void PlaylistSortMgr::QueueCmdGetPlaylistsFromRC() {
    CmdGetPlaylistsFromRC *cmd = new CmdGetPlaylistsFromRC();
    unkc0.push_back(cmd);
    if (!unkc8) {
        ProcessNextCommand();
    }
}

void PlaylistSortMgr::QueueCmdChangeProfileOnlineID(String str) {
    CmdChangeProfileOnlineID *cmd = new CmdChangeProfileOnlineID(str);
    unkc0.push_back(cmd);
    if (!unkc8) {
        ProcessNextCommand();
    }
}

void PlaylistSortMgr::ProcessNextCommand() {
    if (unkc0.size() == 0) {
        unkc8 = false;
    } else {
        unkc8 = true;
        int i = unkc0.front()->GetType();
        switch (i) {
        case 0:
            HandleCmdChangeProfileOnlineID();
            break;
        case 1:
            StartCmdGetPlaylistsFromRC();
            break;
        case 2:
            HandleCmdResolvePlaylists();
            break;
        case 3:
            StartCmdGetPlaylistFromRC();
            break;
        case 4:
            StartCmdAddPlaylistToRC();
            break;
        case 5:
            StartCmdEditPlaylist();
            break;
        case 6:
            StartCmdDeletePlaylistFromRC();
            break;
        }
    }
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
    ((AddPlaylistJob *)unkcc)
        ->GetPlaylistID(((CmdAddPlaylistToRC *)unkc0.front())->playlist);
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

void PlaylistSortMgr::HandleCmdChangeProfileOnlineID() {
    MILO_LOG("===== HandleCmdChangeProfileOnlineID\n");
    unkb8 = ((CmdChangeProfileOnlineID *)unkc0.front())->str;
    RELEASE(unkc0.front());
    unkc0.pop_front();
    ProcessNextCommand();
}

void PlaylistSortMgr::HandleCmdGetPlaylistFromRC() {
    MILO_LOG("===== HandleCmdGetPlaylistFromRC\n");
    GetPlaylistJob *pJob = ((GetPlaylistJob *)unkcc);
    CmdGetPlaylistFromRC *cmd = ((CmdGetPlaylistFromRC *)unkc0.front());
    for (int i = 0; i < unkd0.size(); i++) {
        if (unkd0[i].GetOnlineID() == cmd->num) {
            pJob->GetPlaylist(&unkd0[i]);
            break;
        }
    }
    unkcc = nullptr;
    RELEASE(unkc0.front());
    unkc0.pop_front();
    ProcessNextCommand();
}

void PlaylistSortMgr::HandleCmdGetPlaylistsFromRC() {
    MILO_LOG("===== HandleCmdGetPlaylistsFromRC\n");
    GetPlaylistsJob *pJob = ((GetPlaylistsJob *)unkcc);
    unkd0.clear();
    pJob->GetPlaylists(&unkd0);
    unkcc = nullptr;
    MILO_LOG(">>>>>>>>>> there are %i of playlists on RC.\n", unkd0.size());
    if (unkd0.size() != 0) {
        for (int i = 0; i < unkd0.size(); i++) {
            QueueCmdGetPlaylistFromRC(unkd0[i].GetOnlineID());
        }
    }
    QueueCmdResolvePlaylists();
    RELEASE(unkc0.front());
    unkc0.pop_front();
    ProcessNextCommand();
}

void PlaylistSortMgr::SendPassiveMsg(Symbol s) {
    static Symbol p1("p1");
    static Symbol p2("p2");
    static Symbol none("none");

    Symbol player = none;
    for (int i = 0; i < 2; i++) {
        HamPlayerData *playerData = TheGameData->Player(i);
        MILO_ASSERT(playerData, 0xf6);
        if (playerData->GetPlayerName() == unkb0) {
            player = i == 0 ? p1 : p2;
            break;
        }
    }

    ThePassiveMessenger->TriggerGenericMsg(
        s, player, kPassiveMessageGeneral, gNullStr, -1
    );
}

DataNode PlaylistSortMgr::OnMsg(const RCJobCompleteMsg &msg) {
    if (!msg.Success()) {
        MILO_LOG("[PlaylistSortMgr::OnMsg] Playlist net API failed.\n");
        unkcc = nullptr;
        BroadcastSyncMsg("sync_failed");
        unkc8 = false;
        while (!unkc0.empty()) {
            RELEASE(unkc0.front());
            unkc0.pop_front();
        }
        return 1;
    } else {
        bool check = false;
        if (msg.Job() == unkcc) {
            switch (unkc0.front()->GetType()) {
            case 1:
                HandleCmdGetPlaylistsFromRC();
                break;
            case 3:
                HandleCmdGetPlaylistFromRC();
                break;
            case 4:
                HandleCmdAddPlaylistToRC();
                check = true;
                break;
            case 5:
                HandleCmdEditPlaylist();
                check = true;
                break;
            case 6:
                HandleCmdDeletePlaylistFromRC();
                check = true;
                break;
            }
        }
        if (check) {
            DataNode playlist("playlist");
            DataNode updated("updated");
            ThePlatformMgr.SmartGlassSend(0, DataArrayPtr(updated, playlist));
        }
        return 1;
    }
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
