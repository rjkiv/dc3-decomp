#include "meta/StorePreviewMgr.h"

#include "meta/StreamPlayer.h"
#include "movie/TexMovie.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/System.h"
#include "rndobj/MetaMaterial.h"
#include "synth/MoggClip.h"
#include "utl/FilePath.h"
#include "utl/NetCacheLoader.h"
#include "utl/NetCacheMgr.h"
#include "utl/Std.h"
#include "utl/Str.h"
#include "utl/Symbol.h"
#include <cstring>

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
    return mDownloadQueue.end()
        != std::find(mDownloadQueue.begin(), mDownloadQueue.end(), str);
}

bool StorePreviewMgr::AllowPreviewDownload(String const &str) {
    if (unk40) {
        if (str == unk40->GetRemotePath())
            return false;
    }
    if (TheNetCacheMgr->IsLocalFile(str.c_str()))
        return false;
    else
        return std::find(mDownloadQueue.begin(), mDownloadQueue.end(), str)
            == mDownloadQueue.end();
}

void StorePreviewMgr::PlayCurrentPreview() {
    MILO_ASSERT(mStreamPlayer, 0xd8);
    if (unk34.empty() || !TheNetCacheMgr->IsLocalFile(unk34.c_str())) {
        mStreamPlayer->StopPlaying();
        if (unk4c) {
            unk4c->SetFile(gNullStr);
        }
    } else {
        FilePath temp_str(unk34.c_str());
        if (unk4c) {
            mStreamPlayer->StopPlaying();
            unk4c->SetFile(unk34.c_str());
            unk4c->SetVolume(-unk2c);
        } else {
            int length = temp_str.length() - 5;
            if (temp_str.find(".mogg", length) != String::npos) {
                temp_str.erase(length);
            }
            mStreamPlayer->PlayFile(temp_str.c_str(), -unk2c, 0.0f, unk30);
        }
    }
}

void StorePreviewMgr::AddToDownloadQueue(String const &str) {
    if (unk40) {
        if (str == unk40->GetRemotePath()) {
            return;
        }
    }
    if (!TheNetCacheMgr->IsLocalFile(str.c_str())) {
        if (std::find(mDownloadQueue.begin(), mDownloadQueue.end(), str)
            == mDownloadQueue.end())
            mDownloadQueue.push_back(str);
    }
}

void StorePreviewMgr::Poll() {
    MILO_ASSERT(mStreamPlayer, 0x6f);
    mStreamPlayer->Poll();
    if (unk40) {
        bool rightPath = unk34 == unk40->GetRemotePath();
        if (unk40->IsLoaded()) {
            TheNetCacheMgr->IsLocalFile(unk40->GetRemotePath());
            TheNetCacheMgr->DeleteNetCacheLoader(unk40);
            unk40 = nullptr;
            if (rightPath) {
                PlayCurrentPreview();
            }
            static PreviewDownloadCompleteMsg msg(true, false);
            msg[1] = rightPath;
            Hmx::Object::Handle(msg, false);
        } else if (unk40->HasFailed()) {
            unk48 = true;
            unk44 = unk40->GetFailType();
            TheNetCacheMgr->DeleteNetCacheLoader(unk40);
            unk40 = nullptr;
            static PreviewDownloadCompleteMsg msg(false, false);
            msg[1] = rightPath;
            Hmx::Object::Handle(msg, false);
        }
    }

    for (auto it = mDownloadQueue.begin(); it != mDownloadQueue.end()
         && TheNetCacheMgr->IsLocalFile(mDownloadQueue.front().c_str());
         it = ++mDownloadQueue.end()) {
        mDownloadQueue.pop_front();
    }

    if (!unk40 && !mDownloadQueue.empty()) {
        MILO_ASSERT(!TheNetCacheMgr->IsLocalFile(mDownloadQueue.front().c_str()), 0xa5);
        unk40 = TheNetCacheMgr->AddNetCacheLoader(
            mDownloadQueue.front().c_str(), (NetLoaderPos)1
        );
        mDownloadQueue.pop_front();
    }
}

BEGIN_HANDLERS(StorePreviewMgr)
    HANDLE_ACTION(clear_current_preview, ClearCurrentPreview())
    HANDLE_ACTION(set_current_preview_file, SetCurrentPreviewFile(_msg->Str(2), nullptr))
    HANDLE_ACTION(
        set_current_preview_movie,
        SetCurrentPreviewFile(_msg->Str(2), _msg->Obj<TexMovie>(3))
    )
    HANDLE_ACTION(download_preview_file, DownloadPreviewFile(_msg->Str(2)))
    HANDLE_EXPR(is_downloading_file, IsDownloadingFile(_msg->Str(2)))
    HANDLE_EXPR(allow_preview_download, AllowPreviewDownload(_msg->Str(2)))
    HANDLE_EXPR(is_playing, IsPlaying())
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS
