#include "lazer/meta_ham/PlaylistSongProvider.h"
#include "Playlist.h"
#include "macros.h"
#include "meta_ham/AppLabel.h"
#include "meta_ham/HamSongMgr.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "ui/UILabel.h"
#include "ui/UIListLabel.h"
#include "ui/UIListProvider.h"
#include "utl/Symbol.h"

PlaylistSongProvider::PlaylistSongProvider() : m_pPlaylist(0), unk34(false) {}

int PlaylistSongProvider::NumData() const {
    if (m_pPlaylist == nullptr) {
        return 0;
    }
    return m_pPlaylist->GetNumSongs();
}

Symbol PlaylistSongProvider::DataSymbol(int i) const {
    MILO_ASSERT(m_pPlaylist, 0x6d);
    if (i >= 0 && i < NumData() && m_pPlaylist && m_pPlaylist->IsValidSong(i)) {
        int songID = m_pPlaylist->GetSong(i);
        Symbol shortName = TheHamSongMgr.GetShortNameFromSongID(songID);
        return shortName;
    }
    return gNullStr;
}

void PlaylistSongProvider::Text(
    int, int i_iData, UIListLabel *uiListLabel, UILabel *uiLabel
) const {
    MILO_ASSERT(i_iData < NumData(), 0x22);
    Symbol dataSym = DataSymbol(i_iData);
    if (uiListLabel->Matches("song")) {
        static Symbol playlist_addsong("playlist_addsong");
        if (dataSym == playlist_addsong) {
            static Symbol songname_numbered("songname_numbered");
            uiLabel->SetTokenFmt(songname_numbered, i_iData + 1, playlist_addsong);
        } else {
            AppLabel *pAppLabel = dynamic_cast<AppLabel *>(uiLabel);
            MILO_ASSERT(pAppLabel, 0x31);
            if (NumData() <= 20 || (i_iData < 0x13)) {
                pAppLabel->SetSongName(dataSym, i_iData + 1, false);
                return;
            }
            static Symbol ellipsis("ellipsis");
            uiLabel->SetTextToken(ellipsis);
        }
    } else if (uiListLabel->Matches("song_length")) {
        static Symbol playlist_addsong("playlist_addsong");
        if (dataSym != playlist_addsong) {
            if (NumData() <= 20 || i_iData < 19) {
                AppLabel *pAppLabel = dynamic_cast<AppLabel *>(uiLabel);
                MILO_ASSERT(pAppLabel, 0x4d);
                pAppLabel->SetSongDuration(dataSym);
                return;
            } else {
                static Symbol ellipsis("ellipsis");
                uiLabel->SetTextToken(gNullStr);
            }
        } else {
            uiLabel->SetTextToken(gNullStr);
        }
    } else {
        uiLabel->SetTextToken(gNullStr);
    }
}

void PlaylistSongProvider::UpdateList(Playlist const *p, bool b) {
    unk34 = b;
    m_pPlaylist = p;
}

BEGIN_HANDLERS(PlaylistSongProvider)
    HANDLE_SUPERCLASS(UIListProvider)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS
