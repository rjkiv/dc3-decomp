#include "meta_ham/Playlist.h"
#include "HamSongMgr.h"
#include "meta_ham/HamSongMgr.h"
#include "meta_ham/HamProfile.h"
#include "math/Rand.h"
#include "meta/FixedSizeSaveable.h"
#include "meta/FixedSizeSaveableStream.h"
#include "net_ham/DataMinerJobs.h"
#include "net_ham/RockCentral.h"
#include "os/Debug.h"
#include "utl/NetLoader.h"
#include "utl/Symbol.h"

#pragma region Playlist

Playlist::Playlist() : mName(gNullStr), unk8(0), unk9(0) { m_vSongs.clear(); }

Playlist::~Playlist() { m_vSongs.clear(); }

void Playlist::SwapSongs(int index1, int index2) {
    MILO_ASSERT_RANGE(index1, 0, GetNumSongs(), 0xB3);
    MILO_ASSERT_RANGE(index2, 0, GetNumSongs(), 0xB4);
    int i = m_vSongs[index1];
    m_vSongs[index1] = m_vSongs[index2];
    m_vSongs[index2] = i;
    HandleChange();
}

void Playlist::MoveSong(int from_index, int to_index) {
    MILO_ASSERT_RANGE(from_index, 0, GetNumSongs(), 0xCF);
    if (to_index - from_index < 1) {
    }
    HandleChange();
}

void Playlist::ShuffleSongs() {
    int numSongs = m_vSongs.size();
    for (int i = 0; i < numSongs; i++) {
        SwapSongs(i, rand() % numSongs);
    }
}

bool Playlist::IsValidSong(int i_iIndex) const {
    MILO_ASSERT_RANGE(i_iIndex, 0, m_vSongs.size(), 0x103);
    return TheHamSongMgr.HasSong(m_vSongs[i_iIndex]);
}

int Playlist::GetSong(int i_iIndex) const {
    MILO_ASSERT_RANGE(i_iIndex, 0, m_vSongs.size(), 0x10B);
    return m_vSongs[i_iIndex];
}

int Playlist::GetDuration() const {
    int totalDur = 0;
    for (int i = 0; i < GetNumSongs(); i++) {
        if (IsValidSong(i)) {
            int songID = GetSong(i);
            Symbol name = TheHamSongMgr.GetShortNameFromSongID(songID);
            totalDur += TheHamSongMgr.GetDuration(name);
        }
    }
    return totalDur;
}

int Playlist::GetSongDuration(int i_iIndex) const {
    MILO_ASSERT_RANGE(i_iIndex, 0, m_vSongs.size(), 0x125);
    int duration = 0;
    if (IsValidSong(i_iIndex)) {
        int songID = GetSong(i_iIndex);
        Symbol name = TheHamSongMgr.GetShortNameFromSongID(songID);
        duration = TheHamSongMgr.GetDuration(name);
    }
    return duration;
}

void Playlist::RemoveSong() {
    m_vSongs.pop_back();
    HandleChange();
}

int Playlist::GetLastValidSongIndex() const {
    int index = -1;
    int i = 0;
    int size = m_vSongs.size();
    for (; i < size; i++) {
        if (IsValidSong(i)) {
            index = i;
        }
    }
    return index;
}

void Playlist::RemoveSongAtIndex(int i) {
    m_vSongs.erase(m_vSongs.begin() + i);
    HandleChange();
}

void Playlist::AddSong(int index) {
    if (!IsCustom() || GetNumSongs() < 20) {
        m_vSongs.push_back(index);
    } else {
        MILO_NOTIFY("Trying to add too many songs to playlist!");
    }
    HandleChange();
}

void Playlist::Clear() { m_vSongs.clear(); }

void Playlist::InsertSong(int index1, int index2) {
    int size = m_vSongs.size();
    if (index2 >= size) {
        m_vSongs.push_back(index1);
    } else {
        m_vSongs.insert(m_vSongs.begin() + index2, index1);
    }
    HandleChange();
}

int Playlist::GetNumSongs() const { return m_vSongs.size(); }

#pragma endregion Playlist
#pragma region CustomPlaylist

CustomPlaylist::CustomPlaylist() : mProfile(0), unk24(false), mOnlineID(-1) {
    m_vSongs.clear();
    mSaveSizeMethod = SaveSize;
}

CustomPlaylist::~CustomPlaylist() { m_vSongs.clear(); }

void CustomPlaylist::SaveFixed(FixedSizeSaveableStream &fs) const {
    FixedSizeSaveable::SaveStd(fs, m_vSongs, 20, 4);
    fs << mOnlineID;
    if (unk24) {
        TheRockCentral.ManageJob(new PlaylistChangedJob(0, mName, GetNumSongs()));
    }
}

void CustomPlaylist::LoadFixed(FixedSizeSaveableStream &fs, int) {
    FixedSizeSaveable::LoadStd(fs, m_vSongs, 20, 4);
    fs >> mOnlineID;
    unk24 = false;
}

void CustomPlaylist::SetParentProfile(HamProfile *hp) { mProfile = hp; }

int CustomPlaylist::SaveSize(int x) {
    int i = 0x58;
    if (FixedSizeSaveable::sPrintoutsEnabled) {
        MILO_LOG("* %s = %i\n", "Playlist", i);
    }
    return i;
}

void CustomPlaylist::Copy(CustomPlaylist *customP) {
    mOnlineID = customP->mOnlineID;
    mProfile = customP->mProfile;
    mName = customP->mName;
    unk8 = customP->unk8;
    unk9 = customP->unk9;
    m_vSongs = customP->m_vSongs;
    HandleChange();
}

void CustomPlaylist::HandleChange() {
    if (mProfile) {
        mProfile->MakeDirty();
    }
    unk24 = true;
}

#pragma endregion CustomPlaylist

int GetDynamicPlaylistID(Symbol s) {
    int id = 0;
    if (s == "1960s_dynamic_playlist") {
        id = 1;
    } else if (s == "1970s_dynamic_playlist") {
        id = 2;
    } else if (s == "1980s_dynamic_playlist") {
        id = 4;
    } else if (s == "1990s_dynamic_playlist") {
        id = 8;
    } else if (s == "2000s_dynamic_playlist") {
        id = 0x10;
    } else if (s == "2010s_dynamic_playlist") {
        id = 0x20;
    } else if (s == "2020s_dynamic_playlist") {
        id = 0x40;
    } else if (s == "crew01_dynamic_playlist") {
        id = 0x80;
    } else if (s == "crew02_dynamic_playlist") {
        id = 0x100;
    } else if (s == "crew03_dynamic_playlist") {
        id = 0x200;
    } else if (s == "crew04_dynamic_playlist") {
        id = 0x400;
    } else if (s == "crew05_dynamic_playlist") {
        id = 0x800;
    } else if (s == "crew06_dynamic_playlist") {
        id = 0x1000;
    } else if (s == "crew07_dynamic_playlist") {
        id = 0x2000;
    } else if (s == "crew08_dynamic_playlist") {
        id = 0x4000;
    } else if (s == "crew09_dynamic_playlist") {
        id = 0x8000;
    } else if (s == "crew10_dynamic_playlist") {
        id = 0x10000;
    } else if (s == "crew11_dynamic_playlist") {
        id = 0x20000;
    } else {
        MILO_ASSERT(id != 0, 0x77);
    }
    return id;
}
