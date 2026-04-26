#include "meta/SongMetadata.h"
#include "os/Debug.h"

int SongMetadata::sSaveVer = 6;

void SongMetadata::InitSongMetadata() {
    static Symbol rb3("rb3");
    mID = 0;
    mIsOnDisc = false;
    mGameOrigin = rb3;
    mPreviewStartTime = 0.0f;
    mPreviewEndTime = 0.0f;
    mVersion = -1;
    mSongInfo = 0;
    mShortName = gNullStr;
    mAge = 0;
}

SongMetadata::~SongMetadata() { delete mSongInfo; }

void SongMetadata::PreviewTimes(float &start, float &end) const {
    start = mPreviewStartTime;
    end = mPreviewEndTime;
}

SongMetadata::SongMetadata() { InitSongMetadata(); }

SongMetadata::SongMetadata(DataArray *main_arr, DataArray *backup_arr, bool onDisc)
    : mShortName(), mGameOrigin() {
    InitSongMetadata();
    mIsOnDisc = onDisc;

    DataArray *member_arr;

    static Symbol song_id("song_id");
    if (FIND_WITH_BACKUP(song_id))
        mID = member_arr->Int(1);
    else
        MILO_FAIL("song data for song '%s' has no song_id", main_arr->Sym(0).Str());

    static Symbol game_origin("game_origin");
    if (FIND_WITH_BACKUP(game_origin))
        mGameOrigin = member_arr->Sym(1);

    static Symbol preview("preview");
    if (FIND_WITH_BACKUP(preview)) {
        mPreviewStartTime = member_arr->Float(1);
        mPreviewEndTime = member_arr->Float(2);
    }
    static Symbol version("version");
    if (FIND_WITH_BACKUP(version)) {
        mVersion = member_arr->Int(1);
    }

    mShortName = main_arr->Sym(0);

    static Symbol song("song");
    DataArray *main_song_array = main_arr->FindArray(song, false);
    DataArray *backup_song_array = (!backup_arr) ? 0 : backup_arr->FindArray(song, false);
    if (main_song_array)
        mSongInfo = new DataArraySongInfo(main_song_array, backup_song_array, mShortName);
    else if (backup_song_array)
        mSongInfo = new DataArraySongInfo(backup_song_array, 0, mShortName);
    else
        MILO_FAIL(main_arr->Sym(0).Str());
}

void SongMetadata::Save(BinStream &d) {
    d << sSaveVer;
    d << mVersion;
    d << mID;
    d << (unsigned char)mIsOnDisc;
    d << mGameOrigin;
    d << mPreviewStartTime;
    d << mPreviewEndTime;
    d << mShortName;
    d << mAge;
    MILO_ASSERT(mSongInfo, 0x82);
    d << *mSongInfo;
}

void SongMetadata::Load(BinStream &d) {
    int version;
    d >> version;
    d >> mVersion;
    d >> mID;
    if (version < 2) {
        bool c;
        d >> c;
    }
    if (version < 3) {
        int dummy;
        d >> dummy;
    }
    if (3 <= version) {
        d >> mIsOnDisc;
    }
    d >> mGameOrigin;
    d >> mPreviewStartTime;
    d >> mPreviewEndTime;
    if (version < 1) {
        class String lol;
        d >> lol;
    }
    if (version < 3) {
        class String lmao;
        d >> lmao;
    }
    if (version < 5) {
        DataArray *arr;
        d >> arr;
        if (arr)
            arr->Release();
        d >> arr;
        if (arr)
            arr->Release();
    }
    if (1 <= version) {
        d >> mShortName;
    }
    if (4 <= version) {
        d >> mAge;
    }
    if (5 <= version) {
        MILO_ASSERT(!mSongInfo, 0xC6);
        mSongInfo = new DataArraySongInfo();
        d >> *mSongInfo;
        if (version < 6) {
            bool c;
            d >> c;
            if (c) {
                DataArraySongInfo *info = new DataArraySongInfo();
                d >> *info;
                delete info;
            }
        }
    }
}

BEGIN_HANDLERS(SongMetadata)
    HANDLE_EXPR(version, mVersion)
    HANDLE_EXPR(id, mID)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

DataArraySongInfo *SongMetadata::SongBlock() const { return mSongInfo; }
