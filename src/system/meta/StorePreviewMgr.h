#pragma once

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
    bool AllowPreviewDownload(String const &);
    void Poll();

    float unk2c;
    bool unk30;
    String unk34;
    StreamPlayer *mStreamPlayer; // 0x3c
    NetCacheLoader *unk40;
    NetCacheMgrFailType unk44;
    bool unk48;
    TexMovie *unk4c;
    std::list<String> unk50;

protected:
    void PlayCurrentPreview();
    void AddToDownloadQueue(String const &);
};
