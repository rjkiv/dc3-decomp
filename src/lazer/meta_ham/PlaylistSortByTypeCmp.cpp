#include "meta_ham/PlaylistSortByTypeCmp.h"
#include "PlaylistSortNode.h"
#include "meta_ham/NavListNode.h"
#include "meta_ham/Playlist.h"
#include "os/Debug.h"
#include "utl/Symbol.h"

int PlaylistTypeCmp::Compare(NavListItemSortCmp const *cmp, NavListNodeType type) const {
    switch (type) {
    case kNodeShortcut:
        return 0;
        break;
    case kNodeHeader:
        return mType - cmp->GetPlaylistTypeCmp()->mType;
        break;
    case kNodeItem:
        cmp->GetPlaylistTypeCmp();
        return -1;
        break;
    default:
        MILO_FAIL("invalid type of node comparison.\n");
        break;
    }
    return 0;
}

NavListShortcutNode *PlaylistSortByType::NewShortcutNode(NavListItemNode *node) const {
    Playlist *p = ((PlaylistSortNode *)node)->GetPlaylist();
    int type;
    Symbol name;
    if (p->IsCustom()) {
        type = 1;
        static Symbol playlist_custom("playlist_custom");
        name = playlist_custom;
    } else if (p->IsFitness()) {
        type = 4;
        static Symbol playlist_fitness("playlist_fitness");
        name = playlist_fitness;
    } else if (p->GetUnk9()) {
        type = 2;
        static Symbol playlist_era("playlist_era");
        name = playlist_era;
    } else {
        type = 3;
        static Symbol playlist_crew("playlist_crew");
        name = playlist_crew;
    }

    PlaylistTypeCmp *cmp = new PlaylistTypeCmp(type, "");
    return new NavListShortcutNode(cmp, name, true);
}

NavListHeaderNode *PlaylistSortByType::NewHeaderNode(NavListItemNode *node) const {
    Playlist *p = ((PlaylistSortNode *)node)->GetPlaylist();
    int type;
    Symbol name;
    if (p->IsCustom()) {
        type = 1;
        static Symbol playlist_custom("playlist_custom");
        name = playlist_custom;
    } else if (p->IsFitness()) {
        type = 4;
        static Symbol playlist_fitness("playlist_fitness");
        name = playlist_fitness;
    } else if (p->GetUnk9()) {
        type = 2;
        static Symbol playlist_era("playlist_era");
        name = playlist_era;
    } else {
        type = 3;
        static Symbol playlist_crew("playlist_crew");
        name = playlist_crew;
    }

    PlaylistTypeCmp *cmp = new PlaylistTypeCmp(type, "");
    return new PlaylistHeaderNode(cmp, name, true);
}

NavListItemNode *PlaylistSortByType::NewItemNode(void *p1) const {
    Playlist *p = static_cast<Playlist *>(p1);
    int type;
    if (p->IsCustom()) {
        type = 1;
    } else if (p->IsFitness()) {
        type = 4;
    } else if (p->GetUnk9()) {
        type = 2;
    } else {
        type = 3;
    }
    Symbol name = p->GetName();
    PlaylistTypeCmp *cmp = new PlaylistTypeCmp(type, name.Str());
    return new PlaylistSortNode(cmp, p);
}
