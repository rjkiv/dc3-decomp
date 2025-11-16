#include "lazer/meta_ham/HamSongMetadata.h"
#include "HamSongMetadata.h"
#include "hamobj/HamGameData.h"
#include "lazer/meta_ham/CampaignEra.h"
#include "meta/SongMetadata.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "utl/BinStream.h"
#include "utl/Symbol.h"

#define MIN_SONG_VERSION 0
#define MAX_SONG_VERSION 99

int HamSongMetadata::sHamSaveVer = 14;

HamSongMetadata::HamSongMetadata() : SongMetadata() { InitHamSongMetadata(); }

HamSongMetadata::HamSongMetadata(DataArray *d1, DataArray *d2, bool b)
    : SongMetadata(d1, d2, b) {
    InitHamSongMetadata();
    Integrate(d1, d2, true);
}

HamSongMetadata::~HamSongMetadata() {}

bool HamSongMetadata::HasAlternatePath() const { return mVersion < 11; }

bool HamSongMetadata::IsVersionOK() const {
    MILO_ASSERT_RANGE(mVersion, MIN_SONG_VERSION, MAX_SONG_VERSION + 1, 362);
    int ok;
    if (mVersion >= 0 && mVersion <= 11)
        ok = true;
    else
        ok = false;
    return ok;
}

BEGIN_SAVES(HamSongMetadata)
    bs << sHamSaveVer;
    SAVE_SUPERCLASS(SongMetadata)
    bs << mName;
    bs << mArtist;
    bs << isCover;
    bs << mAlbumName;
    bs << mRank;
    bs << mRating;
    bs << mCharacter;
    bs << mGender;
    bs << mLength;
    bs << mBpm;
    bs << mAlternatePath;
    bs << unk90;
    bs << unk9c;
    bs << unka8;
    bs << unkc0;
    bs << unkc4;
    bs << unkc8;
    bs << unkcc;
    bs << unkd0;
    bs << unkd4;
    bs << unk_0xD8;
    bs << unk_0xDC;
END_SAVES

BEGIN_LOADS(HamSongMetadata)
    LOAD_REVS(bs)
    LOAD_SUPERCLASS(SongMetadata)
    bs >> mName;
    bs >> mArtist;
    if (revs >= 13) {
        bs >> isCover;
    }
    bs >> mAlbumName;
    if (revs < 3) {
        Symbol s;
        int i;
        DateTime dt;
        bs >> i;
        bs >> dt;
        bs >> dt;
        bs >> s;
    }
    if (revs < 8) {
        int i;
        bool b;
        bs >> i;
        bs >> b;
        bs >> b;
        bs >> b;
        bs >> b;
    }
    bs >> mRank;
    bs >> mRating;
    if (revs < 8) {
        short h;
        String s;
        int i;
        float f;
        bs >> h;
        bs >> s;
        bs >> i;
        bs >> i;
        bs >> f;
    }
    if (revs >= 10) {
        bs >> mCharacter;
    }
    if (revs > 0 && revs < 12) {
        Symbol s;
        bs >> s;
    }
    if (revs >= 6) {
        int i;
        bs >> i;
        mGender = i;
    }
    if (revs >= 8) {
        bs >> mLength;
    } else if (revs == 7) {
        int i;
        bs >> i;
        mLength = i * 1000;
    }
    if (revs > 3) {
        bs >> mBpm;
    }
    if (revs >= 5 && revs < 8) {
        bool b;
        bs >> b;
    }
    if (revs >= 9) {
        bs >> mAlternatePath;
    }
    if (revs >= 11) {
        bs >> unk90;
    }
    if (revs >= 14) {
        bs >> unk9c;
        bs >> unka8;
        bs >> unkc0;
        bs >> unkc4;
        bs >> unkc8;
        bs >> unkcc;
        bs >> unkd0;
        bs >> unkd4;
        bs >> unk_0xD8;
        bs >> unk_0xDC;
    }
END_LOADS

bool HamSongMetadata::IsCover() const { return isCover; }

bool HamSongMetadata::IsMedley() const { return isMedley; }

float HamSongMetadata::Rank() const { return mRank; }

Symbol HamSongMetadata::DefaultCharacter() const {
    return GetOutfitCharacter(unkc0, true);
}

int HamSongMetadata::Bpm() const { return mBpm; }

bool HamSongMetadata::IsRanked() const { return mRank != 0.0f; }

bool HamSongMetadata::IsDownload() const {
    static Symbol ham3("ham3");
    return !(GameOrigin() == ham3);
}

Symbol HamSongMetadata::Outfit() const {
    Symbol oc1 = GetOutfitCharacter(unkc0, 0);
    Symbol oc2 = GetOutfitCharacter(unkc8, 0);
    // if (TheProfileMgr.IsContentUnlocked(oc1))

    MILO_NOTIFY(
        "None of the default/backup characters/outfits for song %s are available.",
        mName.c_str()
    );
    return unkc0;
}

Symbol HamSongMetadata::Venue() const {
    MILO_NOTIFY(
        "using a locked backup venue '%s' for song '%s'", unkd4.Str(), mName.c_str()
    );
    return Symbol();
}

Symbol HamSongMetadata::Character() const { return 0; }

BinStream &operator<<(BinStream &bs, PronunciationsLoc const &pron) {
    bs << pron.unk0;
    bs << pron.unk4;
    return bs;
}

BinStream &operator>>(BinStream &bs, PronunciationsLoc &pron) {
    bs >> pron.unk0;
    bs >> pron.unk4;
    return bs;
}

Symbol HamSongMetadata::DrumEvent(int index) {
    auto s = unka8.find(index);
    if (s == unka8.end()) {
        return gNullStr;
    }

    return s->second;
}

char const *HamSongMetadata::Artist() const { return mArtist.c_str(); }

int HamSongMetadata::LengthMs() const { return mLength; }

void HamSongMetadata::Integrate(DataArray *d1, DataArray *d2, bool b) {
    static Symbol name("name");
    DataArray *nameArray = d1->FindArray(name, false);
    if (nameArray && d2->FindArray(name, false)) {
        mName = nameArray->Str(1);
    }

    static Symbol artist("artist");
    DataArray *artistArray = d1->FindArray(artist, false);
    if (artistArray && d2->FindArray(artist, false)) {
        mArtist = artistArray->Str(1);
    }

    static Symbol cover("cover");
    DataArray *coverArray = d1->FindArray(cover, false);
    if (coverArray && d2->FindArray(cover, false)) {
        isCover = coverArray->Int(1) != 0;
    }

    static Symbol album_name("album_name");
    DataArray *albumNameArray = d1->FindArray(album_name, false);
    if (albumNameArray && d2->FindArray(album_name, false)) {
        mAlbumName = albumNameArray->Str(1);
    }

    static Symbol rank("rank");
    DataArray *rankArray = d1->FindArray(rank, false);
    if (rankArray && d2->FindArray(rank, false)) {
        mRank = rankArray->Float(1);
    }

    static Symbol rating("rating");
    DataArray *ratingArray = d1->FindArray(rating, false);
    if (ratingArray && d2->FindArray(rating, false)) {
        mRating = ratingArray->Int(1);
    }

    static Symbol character("character");
    DataArray *characterArray = d1->FindArray(character, false);
    if (characterArray && d2->FindArray(character, false)) {
        mCharacter = characterArray->Str(1);
    }

    static Symbol gender("gender");
    DataArray *genderArray = d1->FindArray(gender, false);
    if (genderArray && d2->FindArray(gender, false)) {
        mGender = genderArray->Int(1);
    }

    static Symbol song_length("song_length");
    DataArray *songLengthArray = d1->FindArray(song_length, false);
    if (songLengthArray && d2->FindArray(song_length, false)) {
        mLength = songLengthArray->Int(1);
    }

    static Symbol bpm("bpm");
    DataArray *bpmArray = d1->FindArray(bpm, false);
    if (bpmArray && d2->FindArray(bpm, false)) {
        mBpm = bpmArray->Int(1);
    }

    static Symbol alternate_path("alternate_path");
    DataArray *altPathArray = d1->FindArray(alternate_path, false);
    if (altPathArray && d2->FindArray(alternate_path, false)) {
        mAlternatePath = altPathArray->Int(1) != 0;
    }

    static Symbol pronunciation("pronunciation");
    DataArray *pronunciationArray = d1->FindArray(pronunciation, false);
    {
        if ((pronunciationArray && pronunciationArray->FindArray(pronunciation, false))
            || (pronunciationArray->Size() != 2)) {
            MILO_NOTIFY(
                "song data for song \'%s\' has no \'pronunciation\' block, or an invalid one",
                d1->Sym(0)
            );
        } else {
        }
    }
}

void HamSongMetadata::InitHamSongMetadata() {
    static Symbol emelia("emelia");
    mName = nullptr;
    mArtist = nullptr;
    isCover = false;
    mAlbumName = nullptr;
    mRating = 1;
    mRank = 0.0f;
    mCharacter = emelia;
    mGender = 1;
    mBpm = 0;
    mLength = 0;
    mAlternatePath = false;
    isMedley = false;
    unk72 = false;
    unk73 = false;
}

BEGIN_HANDLERS(HamSongMetadata)
END_HANDLERS

int GetNoteNum(DataArray *d, Symbol s) {
    DataArray *array = d->Node(0).Array(d);
    if (0 < array->Size()) {
        for (int i = 0; i < array->Size(); i++) {
            DataArray *d1 = d->Node(0).Array(d);
            String str = d1->Node(i).Str(d1);
            str.ToLower();
            if (s == str.c_str()) {
                return i;
            }
            DataArray *d2 = d->Node(0).Array(d);
        }
    }
    return -1;
}
