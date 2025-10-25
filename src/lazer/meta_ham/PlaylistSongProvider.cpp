#include "lazer/meta_ham/PlaylistSongProvider.h"
#include "Playlist.h"
#include "macros.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "ui/UILabel.h"
#include "ui/UIListLabel.h"
#include "ui/UIListProvider.h"
#include "utl/Symbol.h"

PlaylistSongProvider::PlaylistSongProvider() : unk30(0), unk34(false) {}

PlaylistSongProvider::~PlaylistSongProvider() {}

int PlaylistSongProvider::NumData() const {
    if (unk30 == nullptr) {
        return 0;
    }
    return unk30->GetNumSongs();
}

Symbol PlaylistSongProvider::DataSymbol(int m_pPlaylist) const {
    MILO_ASSERT(m_pPlaylist, 0x6d);
    return Symbol(0);
}

void PlaylistSongProvider::Text(
    int, int i_iData, UIListLabel *uiLabel, UILabel *pAppLabel
) const {
    MILO_ASSERT(i_iData < NumData(), 0x22);
    if (uiLabel->Matches("song")) {
        static Symbol playlist_addsong("playlist_addsong");
        if (DataSymbol(i_iData) == playlist_addsong) {
            static Symbol songname_numbered("songname_numbered");
            pAppLabel->SetTokenFmt(songname_numbered, i_iData + playlist_addsong);
            return;
        }

        MILO_ASSERT(pAppLabel, 0x31);
        if (NumData() < 0x15 || (i_iData < 0x13)) {
            // pAppLabel->SetSongName();
            return;
        }

        static Symbol ellipsis("ellipsis");
    } else if (uiLabel->Matches("song_length")) {
        static Symbol playlist_addsong("playlist_addsong");
        if (DataSymbol(i_iData) != playlist_addsong) {
            MILO_ASSERT(pAppLabel, 0x4d);
            // pAppLabel->SetSongDuration();
            return;
        }
        static Symbol ellipsis("ellipsis"); // gets declared then never used ?
    }
}

void PlaylistSongProvider::UpdateList(Playlist const *p, bool b) {
    unk34 = b;
    unk30 = p;
}

BEGIN_HANDLERS(PlaylistSongProvider)
    HANDLE_SUPERCLASS(UIListProvider)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS
