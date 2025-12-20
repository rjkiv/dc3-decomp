#include "meta_ham/PlaylistSortMgr.h"
#include "NavListSortMgr.h"
#include "meta/SongPreview.h"
#include "obj/Dir.h"
#include "obj/Object.h"

PlaylistSortMgr *ThePlaylistSortMgr;

PlaylistSortMgr::PlaylistSortMgr(SongPreview &sp) : NavListSortMgr(sp) {}

PlaylistSortMgr::~PlaylistSortMgr() {}
