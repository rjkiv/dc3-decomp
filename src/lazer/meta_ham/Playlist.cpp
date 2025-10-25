#include "lazer/meta_ham/Playlist.h"
#include "math/Rand.h"
#include "meta/FixedSizeSaveable.h"
#include "meta/FixedSizeSaveableStream.h"
#include "os/Debug.h"
#include "utl/NetLoader.h"
#include "utl/Symbol.h"

#pragma region Playlist

Playlist::Playlist() : unk4(gNullStr), unk8(0), unk9(0) { m_vSongs.clear(); }

Playlist::~Playlist() { m_vSongs.clear(); }

void Playlist::SwapSongs(int index1, int index2) {
    MILO_ASSERT((0) <= (index1) && (index1) < (GetNumSongs()), 0xb3);
    MILO_ASSERT((0) <= (index2) && (index2) < (GetNumSongs()), 0xb4);
    int i = m_vSongs[index1];
    m_vSongs[index1] = m_vSongs[index2];
    m_vSongs[index2] = i;
}

void Playlist::MoveSong(int from_index, int to_index) {
    MILO_ASSERT((0) <= (from_index) && (from_index) < (GetNumSongs()), 0xcf);
}

void Playlist::ShuffleSongs() {
    for (int i = 0; i < m_vSongs.size(); i++) {
    }
}

bool Playlist::IsValidSong(int i_iIndex) const {
    MILO_ASSERT((0) <= (i_iIndex) && (i_iIndex) < (m_vSongs.size()), 0x103);
    return false;
}

int Playlist::GetSong(int i_iIndex) const {
    MILO_ASSERT((0) <= (i_iIndex) && (i_iIndex) < (m_vSongs.size()), 0x10b);
    return m_vSongs[i_iIndex];
}

int Playlist::GetDuration() const { return 1; }

int Playlist::GetSongDuration(int i_iIndex) const {
    MILO_ASSERT((0) <= (i_iIndex) && (i_iIndex) < (m_vSongs.size()), 0x125);
    int i = 0;
    if (IsValidSong(i_iIndex)) {
        i = GetSong(i_iIndex);
    }
    return 1;
}

void Playlist::RemoveSong() { m_vSongs.pop_back(); }

int Playlist::GetLastValidSongIndex() const {
    int index = -1;
    if (0 < m_vSongs.size()) {
        for (int i = 0; i < m_vSongs.size(); i++) {
            if (IsValidSong(i)) {
                index = i;
            }
        }
    }
    return index;
}

void Playlist::RemoveSongAtIndex(int i) {}

void Playlist::AddSong(int index) {
    if (!m_vSongs.empty()) {
        m_vSongs.push_back(index);
    } else {
        MILO_NOTIFY("Trying to add too many songs to playlist!");
    }
}

void Playlist::Clear() { m_vSongs.clear(); }

void Playlist::InsertSong(int index1, int index2) {
    if (index2 < m_vSongs.size()) {
        m_vSongs.insert(m_vSongs.begin() + index2, index1);
    } else {
        m_vSongs.push_back(index1);
    }
}

int Playlist::GetNumSongs() const { return m_vSongs.size(); }

#pragma endregion Playlist
#pragma region CustomPlaylist

CustomPlaylist::CustomPlaylist() : unk20(0), unk24(false), unk28(-1) { m_vSongs.clear(); }

CustomPlaylist::~CustomPlaylist() { m_vSongs.clear(); }

void CustomPlaylist::SaveFixed(FixedSizeSaveableStream &) const {}

void CustomPlaylist::LoadFixed(FixedSizeSaveableStream &, int) {}

void CustomPlaylist::SetParentProfile(class HamProfile *hp) { unk20 = hp; }

int CustomPlaylist::SaveSize(int) {
    if (FixedSizeSaveable::sPrintoutsEnabled) {
        MILO_LOG("* %s = %i\n", "Playlist", 0x58);
    }
    return 0x58;
}

void CustomPlaylist::Copy(CustomPlaylist *customP) {
    unk28 = customP->unk28;
    unk20 = customP->unk20;
    unk4 = customP->unk4;
    unk8 = customP->unk8;
    unk9 = customP->unk9;
    m_vSongs = customP->m_vSongs;
}

void CustomPlaylist::HandleChange() {
    if (unk20) {
        // HamProfile does not exist yet
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
