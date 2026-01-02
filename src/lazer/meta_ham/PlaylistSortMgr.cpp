#include "meta_ham/PlaylistSortMgr.h"
#include "HamProfile.h"
#include "NavListSortMgr.h"
#include "PlaylistSort.h"
#include "ProfileMgr.h"
#include "meta/SongPreview.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "os/ContentMgr.h"
#include "os/Debug.h"
#include "os/PlatformMgr.h"
#include "utl/Symbol.h"

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
