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
    : mAtten(0.0f), mLoopForever(1), mStreamPlayer(nullptr), mNetCacheLoader(0),
      mHasFailed(0) {
    mStreamPlayer = new StreamPlayer();
    MILO_ASSERT(mStreamPlayer, 0x1d);
    DataArray *d = SystemConfig("song_select", "sound");
    d->FindData("loop_forever", mLoopForever);
    d->FindData("attenuation", mAtten);
    SetName("store_preview_mgr", ObjectDir::Main());
}

StorePreviewMgr::~StorePreviewMgr() {
    RELEASE(mStreamPlayer);
    if (mNetCacheLoader) {
        TheNetCacheMgr->DeleteNetCacheLoader(mNetCacheLoader);
        mNetCacheLoader = 0;
    }
}

bool StorePreviewMgr::GetLastFailure(NetCacheMgrFailType &t) {
    if (mHasFailed) {
        t = mLastFailure;
        mHasFailed = false;
        return true;
    }
    return false;
}

bool StorePreviewMgr::IsPlaying() const {
    return (!mStrPathCur.empty() && TheNetCacheMgr->IsLocalFile(mStrPathCur.c_str()));
}

void StorePreviewMgr::ClearCurrentPreview() {
    if (!mStrPathCur.empty()) {
        mStrPathCur = gNullStr;
        PlayCurrentPreview();
    }
}

void StorePreviewMgr::SetCurrentPreviewFile(String const &str, TexMovie *tex) {
    if (mStrPathCur == str && mTexMovie == tex)
        return;
    mTexMovie = tex;
    mStrPathCur = str;
    PlayCurrentPreview();
}

bool StorePreviewMgr::IsDownloadingFile(String const &str) {
    if (mNetCacheLoader) {
        if (str == mNetCacheLoader->GetRemotePath()) {
            return true;
        }
    }
    return mDownloadQueue.end()
        != std::find(mDownloadQueue.begin(), mDownloadQueue.end(), str);
}

bool StorePreviewMgr::AllowPreviewDownload(String const &str) {
    if (mNetCacheLoader) {
        if (str == mNetCacheLoader->GetRemotePath())
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
    if (mStrPathCur.empty() || !TheNetCacheMgr->IsLocalFile(mStrPathCur.c_str())) {
        mStreamPlayer->StopPlaying();
        if (mTexMovie) {
            mTexMovie->SetFile(gNullStr);
        }
    } else {
        FilePath temp_str(mStrPathCur.c_str());
        if (mTexMovie) {
            mStreamPlayer->StopPlaying();
            mTexMovie->SetFile(mStrPathCur.c_str());
            mTexMovie->SetVolume(-mAtten);
        } else {
            int length = temp_str.length() - 5;
            if (temp_str.find(".mogg", length) != String::npos) {
                temp_str.erase(length);
            }
            mStreamPlayer->PlayFile(temp_str.c_str(), -mAtten, 0.0f, mLoopForever);
        }
    }
}

void StorePreviewMgr::AddToDownloadQueue(String const &str) {
    if (mNetCacheLoader) {
        if (str == mNetCacheLoader->GetRemotePath()) {
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
    if (mNetCacheLoader) {
        bool rightPath = mStrPathCur == mNetCacheLoader->GetRemotePath();
        if (mNetCacheLoader->IsLoaded()) {
            TheNetCacheMgr->IsLocalFile(mNetCacheLoader->GetRemotePath());
            TheNetCacheMgr->DeleteNetCacheLoader(mNetCacheLoader);
            mNetCacheLoader = nullptr;
            if (rightPath) {
                PlayCurrentPreview();
            }
            static PreviewDownloadCompleteMsg msg(true, false);
            msg[1] = rightPath;
            Hmx::Object::Handle(msg, false);
        } else if (mNetCacheLoader->HasFailed()) {
            mHasFailed = true;
            mLastFailure = mNetCacheLoader->GetFailType();
            TheNetCacheMgr->DeleteNetCacheLoader(mNetCacheLoader);
            mNetCacheLoader = nullptr;
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

    if (!mNetCacheLoader && !mDownloadQueue.empty()) {
        MILO_ASSERT(!TheNetCacheMgr->IsLocalFile(mDownloadQueue.front().c_str()), 0xa5);
        mNetCacheLoader = TheNetCacheMgr->AddNetCacheLoader(
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
