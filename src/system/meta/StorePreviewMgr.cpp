#include "meta/StorePreviewMgr.h"
#include "StorePreviewMgr.h"

#include "meta/StreamPlayer.h"
#include "movie/TexMovie.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/System.h"
#include "rndobj/MetaMaterial.h"
#include "synth/MoggClip.h"
#include "utl/NetCacheLoader.h"
#include "utl/NetCacheMgr.h"
#include "utl/Str.h"
#include "utl/Symbol.h"

StorePreviewMgr::StorePreviewMgr()
    : unk2c(0.0f), unk30(1), mStreamPlayer(nullptr), unk40(0), unk48(0) {
    mStreamPlayer = new StreamPlayer();
    MILO_ASSERT(mStreamPlayer, 0x1d);
    DataArray *d = SystemConfig("song_select", "sound");
    d->FindData("loop_forever", unk30);
    d->FindData("attenuation", unk2c);
    SetName("store_preview_mgr", ObjectDir::Main());
}

StorePreviewMgr::~StorePreviewMgr() {
    RELEASE(mStreamPlayer);
    if (unk40) {
        TheNetCacheMgr->DeleteNetCacheLoader(unk40);
        unk40 = 0;
    }
}

bool StorePreviewMgr::GetLastFailure(NetCacheMgrFailType &t) {
    if (unk48) {
        t = unk44;
        unk48 = false;
        return true;
    }
    return false;
}

bool StorePreviewMgr::IsPlaying() const {
    return (!unk34.empty() && TheNetCacheMgr->IsLocalFile(unk34.c_str()));
}

void StorePreviewMgr::ClearCurrentPreview() {
    if (!unk34.empty()) {
        unk34 = gNullStr;
        PlayCurrentPreview();
    }
}

void StorePreviewMgr::SetCurrentPreviewFile(String const &str, TexMovie *tex) {
    if (unk34 == str && unk4c == tex)
        return;
    unk4c = tex;
    unk34 = str;
    PlayCurrentPreview();
}

bool StorePreviewMgr::IsDownloadingFile(String const &str) {
    if (unk40) {
        if (str == unk40->GetRemotePath()) {
            return true;
        }
    }
    return unk50.end() != std::find(unk50.begin(), unk50.end(), str);
}

bool StorePreviewMgr::AllowPreviewDownload(String const &str) {
    if (unk40) {
        if (str == unk40->GetRemotePath())
            return false;
    }
    if (TheNetCacheMgr->IsLocalFile(str.c_str()))
        return false;
    else
        return std::find(unk50.begin(), unk50.end(), str) == unk50.end();
}

void StorePreviewMgr::PlayCurrentPreview() {
    MILO_ASSERT(mStreamPlayer, 0xd8);
    if (unk34.empty()) {
    }
}

void StorePreviewMgr::AddToDownloadQueue(String const &str) {
    if (unk40) {
        if (str == unk40->GetRemotePath()) {
            return;
        }
    }
    if (!TheNetCacheMgr->IsLocalFile(str.c_str())) {
        if (std::find(unk50.begin(), unk50.end(), str) == unk50.end())
            unk50.push_back(str);
    }
}

BEGIN_HANDLERS(StorePreviewMgr)
HANDLE_ACTION(clear_current_preview, ClearCurrentPreview())
HANDLE_ACTION(set_current_preview_file, SetCurrentPreviewFile(_msg->Str(2), nullptr))
HANDLE_ACTION(set_current_preview_movie, SetCurrentPreviewFile(_msg->Str(2), _msg->Obj<TexMovie>(3)))
HANDLE_ACTION(download_preview_file, AddToDownloadQueue(_msg->Str(2)))
HANDLE_EXPR(is_downloading_file, IsDownloadingFile(_msg->Str(2)))
HANDLE_EXPR(allow_preview_download, AllowPreviewDownload(_msg->Str(2)))
HANDLE_EXPR(is_playing, IsPlaying())
HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS