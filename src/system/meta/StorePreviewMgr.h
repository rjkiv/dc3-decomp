#pragma once

#include "obj/Msg.h"
#include "meta/StreamPlayer.h"
#include "movie/TexMovie.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "utl/NetCacheLoader.h"
#include "utl/NetCacheMgr.h"
#include "utl/Str.h"

class StorePreviewMgr : public Hmx::Object {
public:
    // Hmx::Object
    virtual ~StorePreviewMgr();
    virtual DataNode Handle(DataArray *, bool);

    StorePreviewMgr();
    bool GetLastFailure(NetCacheMgrFailType &);
    bool IsPlaying() const;
    void ClearCurrentPreview();
    void SetCurrentPreviewFile(String const &, TexMovie *);
    bool IsDownloadingFile(String const &);
    void DownloadPreviewFile(String const &s) { AddToDownloadQueue(s); }
    bool AllowPreviewDownload(String const &);
    void Poll();

protected:
    void PlayCurrentPreview();
    void AddToDownloadQueue(String const &);

private:
    float mAtten; // 0x2c
    bool mLoopForever; // 0x30
    String mStrPathCur; // 0x34
    StreamPlayer *mStreamPlayer; // 0x3c
    NetCacheLoader *mNetCacheLoader; // 0x40
    NetCacheMgrFailType mLastFailure; // 0x44
    bool mHasFailed; // 0x48
    TexMovie *mTexMovie; // 0x4c
    std::list<String> mDownloadQueue; // 0x50
};

DECLARE_MESSAGE(PreviewDownloadCompleteMsg, "preview_download_complete_msg")
PreviewDownloadCompleteMsg(bool b1, bool b2) : Message(Type(), b1, b2) {};
END_MESSAGE
