#include "lazer/meta_ham/HamSongMetadata.h"
#include "HamSongMetadata.h"
#include "hamobj/HamCharacter.h"
#include "hamobj/HamGameData.h"
#include "meta/DataArraySongInfo.h"
#include "meta/SongMetadata.h"
#include "meta_ham/ProfileMgr.h"
#include "obj/Data.h"
#include "obj/DataUtl.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/System.h"
#include "utl/BinStream.h"
#include "utl/Symbol.h"

#define MIN_SONG_VERSION 0
#define MAX_SONG_VERSION 99

int HamSongMetadata::sHamSaveVer = 14;

int GetNoteNum(DataArray *a, Symbol s) {
    for (int i = 0; i < a->Array(0)->Size(); i++) {
        String str(a->Array(0)->Str(i));
        str.ToLower();
        if (Symbol(str.c_str()) == s) {
            return i;
        }
    }
    return -1;
}

HamSongMetadata::HamSongMetadata() : SongMetadata() { InitHamSongMetadata(); }

HamSongMetadata::HamSongMetadata(DataArray *main, DataArray *backup, bool onDisc)
    : SongMetadata(main, backup, onDisc) {
    InitHamSongMetadata();
    Integrate(main, backup, true);
}

HamSongMetadata::~HamSongMetadata() {}

BEGIN_HANDLERS(HamSongMetadata)
    HANDLE_EXPR(character, Character())
    HANDLE_EXPR(gender, mGender)
    HANDLE_EXPR(bpm, mBpm)
    HANDLE_EXPR(title, mTitle.c_str())
    HANDLE_EXPR(artist, mArtist.c_str())
    HANDLE_EXPR(is_cover, mIsCover)
    HANDLE_EXPR(is_complete, mIsComplete)
    HANDLE_EXPR(is_fake, mIsFake)
    HANDLE_EXPR(length_ms, mLength)
    HANDLE_EXPR(game_origin, GameOrigin())
    HANDLE_EXPR(drum_event, DrumEvent(_msg->Int(2)))
    HANDLE_SUPERCLASS(SongMetadata)
END_HANDLERS

BEGIN_SAVES(HamSongMetadata)
    bs << sHamSaveVer;
    SAVE_SUPERCLASS(SongMetadata)
    bs << mTitle;
    bs << mArtist;
    bs << mIsCover;
    bs << mAlbum;
    bs << mRank;
    bs << mRating;
    bs << mCharacter;
    bs << mGender;
    bs << mLength;
    bs << mBpm;
    bs << mHasAlternatePath;
    bs << mPronunciations;
    bs << mPronunciationsLoc;
    bs << mDrumMap;
    bs << mDefaultCharacter;
    bs << mDefaultCharacterAlt;
    bs << mBackupCharacter;
    bs << mBackupCharacterAlt;
    bs << mDefaultVenue;
    bs << mBackupVenue;
    bs << mYearReleased;
    bs << mDJIntensityRank;
END_SAVES

BEGIN_LOADS(HamSongMetadata)
    int rev;
    bs >> rev;
    SongMetadata::Load(bs);
    bs >> mTitle;
    bs >> mArtist;
    if (rev >= 13) {
        bs >> mIsCover;
    }
    bs >> mAlbum;
    if (rev < 3) {
        Symbol s;
        int i;
        DateTime dt;
        bs >> i;
        bs >> dt;
        bs >> dt;
        bs >> s;
    }
    if (rev < 8) {
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
    if (rev < 8) {
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
    if (rev >= 10) {
        bs >> mCharacter;
    }
    if (rev > 0 && rev < 12) {
        Symbol s;
        bs >> s;
    }
    if (rev >= 6) {
        int i;
        bs >> i;
        mGender = (HamCharacter::Gender)i;
    }
    if (rev >= 8) {
        bs >> mLength;
    } else if (rev == 7) {
        int i;
        bs >> i;
        mLength = i * 1000;
    }
    if (rev > 3) {
        bs >> mBpm;
    }
    if (rev >= 5 && rev < 8) {
        bool b;
        bs >> b;
    }
    if (rev >= 9) {
        bs >> mHasAlternatePath;
    }
    if (rev >= 11) {
        bs >> mPronunciations;
    }
    if (rev >= 14) {
        bs >> mPronunciationsLoc;
        bs >> mDrumMap;
        bs >> mDefaultCharacter;
        bs >> mDefaultCharacterAlt;
        bs >> mBackupCharacter;
        bs >> mBackupCharacterAlt;
        bs >> mDefaultVenue;
        bs >> mBackupVenue;
        bs >> mYearReleased;
        bs >> mDJIntensityRank;
    }
END_LOADS

bool HamSongMetadata::IsVersionOK() const {
    MILO_ASSERT_RANGE(mVersion, MIN_SONG_VERSION, MAX_SONG_VERSION + 1, 362);
    return mVersion >= 0 && mVersion <= 11;
}

bool HamSongMetadata::HasAlternatePath() const { return mVersion < 11; }

bool HamSongMetadata::IsCover() const { return mIsCover; }
bool HamSongMetadata::IsMedley() const { return mIsMedley; }
float HamSongMetadata::Rank() const { return mRank; }
Symbol HamSongMetadata::DefaultCharacter() const {
    return GetOutfitCharacter(mDefaultCharacter);
}
int HamSongMetadata::Bpm() const { return mBpm; }
bool HamSongMetadata::IsRanked() const { return mRank != 0.0f; }
const std::vector<String> &HamSongMetadata::Pronunciations() const {
    return mPronunciations;
}
const std::vector<PronunciationsLoc> &HamSongMetadata::PronunciationsLocalized() const {
    return mPronunciationsLoc;
}

bool HamSongMetadata::IsDownload() const {
    static Symbol ham3("ham3");
    return GameOrigin() != ham3;
}

Symbol HamSongMetadata::Outfit() const {
    Symbol charSym1 = GetOutfitCharacter(mDefaultCharacter, false);
    Symbol charSym2 = GetOutfitCharacter(mBackupCharacter, false);

    if (TheProfileMgr.IsContentUnlocked(charSym1)) {
        if (TheProfileMgr.IsContentUnlocked(mDefaultCharacter)) {
            return mDefaultCharacter;
        } else if (TheProfileMgr.IsContentUnlocked(mDefaultCharacterAlt)) {
            return mDefaultCharacterAlt;
        }
    } else if (TheProfileMgr.IsContentUnlocked(charSym2)) {
        if (TheProfileMgr.IsContentUnlocked(mBackupCharacter)) {
            return mBackupCharacter;
        } else if (TheProfileMgr.IsContentUnlocked(mBackupCharacterAlt)) {
            return mBackupCharacterAlt;
        }
    }
    MILO_NOTIFY(
        "None of the default/backup characters/outfits for song %s are available.",
        mTitle.c_str()
    );
    return mDefaultCharacter;
}

Symbol HamSongMetadata::Venue() const {
    if (TheProfileMgr.IsContentUnlocked(mDefaultVenue)) {
        return mDefaultVenue;
    } else if (TheProfileMgr.IsContentUnlocked(mBackupVenue)) {
        return mBackupVenue;
    } else {
        MILO_NOTIFY(
            "using a locked backup venue '%s' for song '%s'",
            mBackupVenue.Str(),
            mTitle.c_str()
        );
        static Symbol dci("dci");
        return dci;
    }
}

Symbol HamSongMetadata::Character() const { return GetOutfitCharacter(Outfit()); }

BinStream &operator<<(BinStream &bs, PronunciationsLoc const &pron) {
    bs << pron.mLanguage;
    bs << pron.mPronunciations;
    return bs;
}

BinStream &operator>>(BinStream &bs, PronunciationsLoc &pron) {
    bs >> pron.mLanguage;
    bs >> pron.mPronunciations;
    return bs;
}

Symbol HamSongMetadata::DrumEvent(int index) {
    auto s = mDrumMap.find(index);
    if (s == mDrumMap.end()) {
        return Symbol();
    } else {
        return s->second;
    }
}

char const *HamSongMetadata::Artist() const { return mArtist.c_str(); }

int HamSongMetadata::LengthMs() const { return mLength; }

void HamSongMetadata::InitHamSongMetadata() {
    static Symbol emelia("emelia");
    mTitle = nullptr;
    mArtist = nullptr;
    mIsCover = false;
    mAlbum = nullptr;
    mRating = 1;
    mRank = 0;
    mCharacter = emelia;
    mGender = HamCharacter::kHamMale;
    mBpm = 0;
    mLength = 0;
    mHasAlternatePath = false;
    mIsMedley = false;
    mIsComplete = false;
    mIsFake = false;
}

bool HamSongMetadata::IsPrivate() const { return false; }

void HamSongMetadata::Integrate(DataArray *main_arr, DataArray *backup_arr, bool b) {
    DataArray *member_arr;

    static Symbol name("name");
    if (FIND_WITH_BACKUP(name)) {
        mTitle = member_arr->Str(1);
    }
    static Symbol artist("artist");
    if (FIND_WITH_BACKUP(artist)) {
        mArtist = member_arr->Str(1);
    }
    static Symbol cover("cover");
    if (FIND_WITH_BACKUP(cover)) {
        mIsCover = member_arr->Int(1);
    }
    static Symbol album_name("album_name");
    if (FIND_WITH_BACKUP(album_name)) {
        mAlbum = member_arr->Str(1);
    }
    static Symbol rank("rank");
    if (FIND_WITH_BACKUP(rank)) {
        mRank = member_arr->Float(1);
    }
    static Symbol rating("rating");
    if (FIND_WITH_BACKUP(rating)) {
        mRating = member_arr->Int(1);
    }
    static Symbol character("character");
    if (FIND_WITH_BACKUP(character)) {
        mCharacter = member_arr->Sym(1);
    }
    static Symbol gender("gender");
    if (FIND_WITH_BACKUP(gender)) {
        mGender = (HamCharacter::Gender)member_arr->Int(1);
    }
    static Symbol song_length("song_length");
    if (FIND_WITH_BACKUP(song_length)) {
        mLength = member_arr->Int(1);
    }
    static Symbol bpm("bpm");
    if (FIND_WITH_BACKUP(bpm)) {
        mBpm = member_arr->Int(1);
    }
    static Symbol alternate_path("alternate_path");
    if (FIND_WITH_BACKUP(alternate_path)) {
        mHasAlternatePath = member_arr->Int(1);
    }

    static Symbol pronunciation("pronunciation");
    if (FIND_WITH_BACKUP(pronunciation) && member_arr->Size() == 2) {
        mPronunciations.clear();
        DataArray *curArr = member_arr->Array(1);
        for (int i = 0; i < curArr->Size(); i++) {
            mPronunciations.push_back(curArr->Str(i));
        }
    } else {
        MILO_NOTIFY(
            "song data for song '%s' has no 'pronunciation' block, or an invalid one",
            main_arr->Sym(0).Str()
        );
    }

    static Symbol pronunciation_loc("pronunciation_loc");
    if (FIND_WITH_BACKUP(pronunciation_loc) && member_arr->Size() >= 2) {
        mPronunciationsLoc.clear();
        for (int i = 1; i < member_arr->Size(); i++) {
            DataArray *curArr = member_arr->Array(i);
            if (curArr && curArr->Size() == 2) {
                PronunciationsLoc loc;
                loc.mLanguage = curArr->Sym(0);
                for (int j = 0; j < curArr->Array(1)->Size(); j++) {
                    loc.mPronunciations.push_back(curArr->Array(1)->Str(j));
                }
                mPronunciationsLoc.push_back(loc);
            }
        }
    }

    static Symbol medley("medley");
    if (FIND_WITH_BACKUP(medley)) {
        mIsMedley = member_arr->Int(1);
    }

    static Symbol midi_events("midi_events");
    static Symbol preset("preset");
    if (FIND_WITH_BACKUP(midi_events) != 0) {
        DataArray *list = DataGetMacro("MIDI_NOTE_LIST");
        DataArray *presetsArr = member_arr->FindArray(preset, false);
        if (presetsArr) {
            member_arr = SystemConfig(
                "beatmatcher", "midi_parsers", "presets", presetsArr->Sym(1)
            );
        }
        for (int i = 1; i < member_arr->Size(); i++) {
            DataArray *pairArr = member_arr->Array(i);
            mDrumMap[GetNoteNum(list, pairArr->Sym(0))] = pairArr->Sym(1);
        }
    } else {
        MILO_NOTIFY(
            "song data for song '%s' has no 'midi_events' block, or an invalid one",
            main_arr->Sym(0).Str()
        );
    }

    static Symbol default_character("default_character");
    if (FIND_WITH_BACKUP(default_character)) {
        mDefaultCharacter = member_arr->Sym(1);
    }
    if (mDefaultCharacter.Null()) {
        MILO_NOTIFY(
            "Song %s does not have default character (default outfit) data; Resorting to badness",
            mTitle
        );
        mDefaultCharacter = GetCharacterOutfit(mCharacter, 0);
    }

    static Symbol default_character_alt("default_character_alt");
    if (FIND_WITH_BACKUP(default_character_alt)) {
        mDefaultCharacterAlt = member_arr->Sym(1);
    }
    if (mDefaultCharacterAlt.Null()) {
        MILO_NOTIFY(
            "Song %s does not have default character (backup outfit) data; Resorting to badness",
            mTitle
        );
        mDefaultCharacterAlt = GetCharacterOutfit(mCharacter, 1);
    }
    Symbol defaultCharacter = GetOutfitCharacter(mDefaultCharacter, false);
    Symbol defaultCharacterAlt = GetOutfitCharacter(mDefaultCharacterAlt, false);
    if (defaultCharacter != defaultCharacterAlt) {
        MILO_NOTIFY(
            "defaultCharacter (%s) does not match defaultCharacterAlt (%s) for song %s",
            defaultCharacter,
            defaultCharacterAlt,
            mTitle
        );
    }

    static Symbol backup_character("backup_character");
    if (FIND_WITH_BACKUP(backup_character)) {
        mBackupCharacter = member_arr->Sym(1);
    }
    if (mBackupCharacter.Null()) {
        MILO_NOTIFY(
            "Song %s does not have backup character (default outfit) data; Resorting to badness",
            mTitle
        );
        mBackupCharacter = GetCharacterOutfit(mCharacter, 0);
    }

    static Symbol backup_character_alt("backup_character_alt");
    if (FIND_WITH_BACKUP(backup_character_alt)) {
        mBackupCharacterAlt = member_arr->Sym(1);
    }
    if (mBackupCharacterAlt.Null()) {
        MILO_NOTIFY(
            "Song %s does not have backup character (alternate outfit) data; Resorting to badness",
            mTitle
        );
        mBackupCharacterAlt = GetCharacterOutfit(mCharacter, 1);
    }
    Symbol backupCharacter = GetOutfitCharacter(mBackupCharacter, false);
    Symbol backupCharacterAlt = GetOutfitCharacter(mBackupCharacterAlt, false);
    if (backupCharacter != backupCharacterAlt) {
        MILO_NOTIFY(
            "backupCharacter (%s) does not match backupCharacterAlt (%s) for song %s",
            backupCharacter,
            backupCharacterAlt,
            mTitle
        );
    }

    static Symbol default_venue("default_venue");
    if (FIND_WITH_BACKUP(default_venue)) {
        mDefaultVenue = member_arr->Sym(1);
    }
    static Symbol backup_venue("backup_venue");
    if (FIND_WITH_BACKUP(backup_venue)) {
        mBackupVenue = member_arr->Sym(1);
    }
    static Symbol is_complete("is_complete");
    if (FIND_WITH_BACKUP(is_complete)) {
        mIsComplete = member_arr->Int(1);
    }
    static Symbol is_fake("is_fake");
    if (FIND_WITH_BACKUP(is_fake)) {
        mIsFake = member_arr->Int(1);
    }
    static Symbol year_released("year_released");
    if (FIND_WITH_BACKUP(year_released)) {
        mYearReleased = member_arr->Int(1);
    }
    static Symbol dj_intensity_rank("dj_intensity_rank");
    if (FIND_WITH_BACKUP(dj_intensity_rank)) {
        mDJIntensityRank = member_arr->Int(1);
    }
}
