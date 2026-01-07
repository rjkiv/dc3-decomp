#include "meta/PreloadPanel.h"
#include "PreloadPanel.h"
#include "SongMgr.h"
#include "meta/SongMgr.h"
#include "obj/Data.h"
#include "obj/DirLoader.h"
#include "obj/Object.h"
#include "os/ContentMgr.h"
#include "os/Debug.h"
#include "os/File.h"
#include "os/FileCache.h"
#include "ui/UIPanel.h"
#include "utl/Loader.h"
#include "utl/Std.h"
#include "utl/Symbol.h"

#pragma region Hmx::Object

PreloadPanel::PreloadPanel()
    : mPreloadResult(kPreloadInProgress), mMounted(0), mAppReadFailureHandler(), unk60(0),
      unk6c(0), mMaxCacheSize(0x500000) {
    if (!sCache) {
        sCache = new FileCache(mMaxCacheSize, kLoadBack, true, true);
    }
}

PreloadPanel::~PreloadPanel() {}

BEGIN_HANDLERS(PreloadPanel)
    HANDLE_MESSAGE(ContentReadFailureMsg)
    HANDLE_MESSAGE(UITransitionCompleteMsg)
    HANDLE_SUPERCLASS(UIPanel)
END_HANDLERS

void PreloadPanel::SetTypeDef(DataArray *d) {
    UIPanel::SetTypeDef(d);
    d->FindData("max_cache_size", mMaxCacheSize, false);
    CheckTypeDef("song_mgr");
    CheckTypeDef("current_song");
    CheckTypeDef("on_preload_ok");
    CheckTypeDef("preload_files");
}

#pragma endregion
#pragma region UIPanel

void PreloadPanel::Load() {
    UIPanel::Load();
    TheLoadMgr.SetLoaderPeriod(14.0f);
    mPreloadResult = kPreloadInProgress;
    TheContentMgr.RegisterCallback(this, false);
    mAppReadFailureHandler = TheContentMgr.SetReadFailureHandler(this);
    MILO_ASSERT(mAppReadFailureHandler, 0x50);
    unk60 = false;
    unk64 = gNullStr;
    Symbol cur = CurrentSong();
    if (cur.Null()) {
        MILO_NOTIFY("Trying to preload null song");
    }
    SongMgr *song_mgr = FindSongMgr();
    MILO_ASSERT(song_mgr, 0x5E);
    mContentNames.clear();
    unk6c = false;
    if (!song_mgr->HasSong(cur, false)) {
        unk6c = true;
    } else {
        song_mgr->GetContentNames(cur, mContentNames);
        for (auto it = mContentNames.begin(); it != mContentNames.end();) {
            if (!TheContentMgr.MountContent(*it)) {
                ++it;
                mMounted = false;
            } else {
                it = mContentNames.erase(it);
            }
        }
    }
    if (mContentNames.empty()) {
        StartCache();
    }
}

bool PreloadPanel::IsLoaded() const {
    if (!UIPanel::IsLoaded())
        return false;
    else
        return mPreloadResult != kPreloadInProgress;
}

void PreloadPanel::Unload() {
    mContentNames.clear();
    UIPanel::Unload();
}

void PreloadPanel::PollForLoading() {
    UIPanel::PollForLoading();
    if (UIPanel::IsLoaded()) {
        if (!mMounted && mContentNames.empty()) {
            StartCache();
        }
        if (mPreloadResult == kPreloadInProgress && mMounted && sCache->DoneCaching()) {
            if (unk6c) {
                mPreloadResult = kPreloadFailure;
            } else {
                FileCache::PollAll();
                FOREACH (it, mPreloadedFiles) {
                    if (!CheckFileCached(it->c_str())) {
                        mPreloadResult = kPreloadFailure;
                    }
                }
            }
            if (mPreloadResult != kPreloadFailure) {
                mPreloadResult = kPreloadSuccess;
            }
        }
    }
}

void PreloadPanel::FinishLoad() {
    UIPanel::FinishLoad();
    TheLoadMgr.SetLoaderPeriod(10);
    TheContentMgr.UnregisterCallback(this, true);
    ClearAndShrink(mPreloadedFiles);
    TheContentMgr.SetReadFailureHandler(mAppReadFailureHandler);
}

#pragma endregion
#pragma region ContentMgr::Callback

void PreloadPanel::ContentMounted(const char *c1, const char *c2) {
    OnContentMountedOrFailed(c1);
}

void PreloadPanel::ContentFailed(char const *c) {
    const char *cc20 = gNullStr;
    if (TheContentMgr.IsCorrupt(c, cc20)) {
        unk60 = true;
        unk64 = cc20;
    }
    OnContentMountedOrFailed(c);
}

#pragma endregion
#pragma region PreloadPanel

Symbol PreloadPanel::CurrentSong() const {
    static Symbol current_song("current_song");
    return TypeDef()->FindSym(current_song);
}

void PreloadPanel::CheckTypeDef(Symbol s) {
    if (!TypeDef()->FindArray(s, false))
        MILO_NOTIFY(
            "PreloadPanel %s missing %s handler (%s)", Name(), s, TypeDef()->File()
        );
}

bool PreloadPanel::CheckFileCached(const char *cc) {
    if (!*cc || sCache->FileCached(cc)) {
        return true;
    } else {
        MILO_NOTIFY("Could not cache %s", cc);
        return false;
    }
}

SongMgr *PreloadPanel::FindSongMgr() const {
    static Symbol song_mgr("song_mgr");
    return TypeDef()->FindArray(song_mgr)->Obj<SongMgr>(1);
}

DataNode PreloadPanel::OnMsg(const ContentReadFailureMsg &msg) {
    unk60 = msg->Int(2);
    unk64 = msg->Str(3);
    return 1;
}

DataNode PreloadPanel::OnMsg(const UITransitionCompleteMsg &msg) {
    MILO_ASSERT(mPreloadResult != kPreloadInProgress, 0x153);
    if (mPreloadResult == kPreloadSuccess) {
        static Message msg("on_preload_ok");
        HandleType(msg);
    } else {
        static Message msg("on_preload_failed");
        if (HandleType(msg).Equal(DATA_UNHANDLED, nullptr, true)) {
            MILO_ASSERT(mAppReadFailureHandler, 0x15F);
            static ContentReadFailureMsg msg(false, gNullStr);
            msg[0] = unk60;
            msg[1] = unk64;
            mAppReadFailureHandler->Handle(msg, true);
        }
    }
    mAppReadFailureHandler = nullptr;
    return DATA_UNHANDLED;
}

void PreloadPanel::OnContentMountedOrFailed(char const *contentName) {
    if (!mContentNames.empty()) {
        MILO_ASSERT(contentName, 0x12b);
        for (std::vector<Symbol>::iterator it = mContentNames.begin();
             it != mContentNames.end();) {
            Symbol s = *it;
            if (s == contentName) {
                it = mContentNames.erase(it);
            } else {
                it++;
            }
        }
    }
}

void PreloadPanel::StartCache() {
    MILO_ASSERT(mContentNames.empty(), 0xF8);
    mMounted = true;
    MILO_ASSERT(sCache, 0xFB);
    sCache->Clear();
    sCache->SetSize(mMaxCacheSize);
    sCache->StartSet(0);
    if (!unk6c) {
        static Symbol preload_files("preload_files");
        DataArray *files = TypeDef()->FindArray(preload_files);
        for (int i = 1; i < files->Size(); i++) {
            DataArray *arr = files->Array(i);
            const char *path = arr->Str(0);
            MILO_ASSERT(path, 0x109);
            bool b1 = arr->Int(1);
            if (!b1 || FileExists(DirLoader::CachedPath(path, false), 0, nullptr)) {
                sCache->Add(path, 1, path);
                mPreloadedFiles.push_back(path);
            }
        }
    }
    sCache->EndSet();
}
