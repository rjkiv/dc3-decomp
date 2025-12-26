#pragma once
#include "hamobj/HamCharacter.h"
#include "meta/SongMetadata.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "stl/_vector.h"
#include "utl/BinStream.h"
#include "utl/Symbol.h"

struct PronunciationsLoc {
    Symbol unk0; // 0x0 - language
    std::vector<String> unk4; // 0x4 - different possible pronunciations
};

class HamSongMetadata : public SongMetadata {
public:
    HamSongMetadata();
    HamSongMetadata(DataArray *main_arr, DataArray *backup_arr, bool onDisc);
    // SongMetadata
    virtual ~HamSongMetadata();
    virtual DataNode Handle(DataArray *, bool);
    virtual void Save(BinStream &);
    virtual void Load(BinStream &);
    virtual bool IsVersionOK() const;
    virtual bool HasAlternatePath() const;

    bool IsCover() const;
    bool IsMedley() const;
    bool IsFake() const { return mIsFake; }
    float Rank() const;
    Symbol DefaultCharacter() const;
    int Bpm() const;
    bool IsRanked() const;
    bool IsDownload() const;
    Symbol Outfit() const;
    Symbol Venue() const;
    Symbol Character() const;
    Symbol DrumEvent(int);
    void Integrate(DataArray *, DataArray *, bool);
    char const *Artist() const;
    int LengthMs() const;
    bool IsPrivate() const;
    const std::vector<String> &Pronunciations() const;
    const std::vector<PronunciationsLoc> &PronunciationsLocalized() const;
    const char *Album() const;
    const char *Title() const;

private:
    void InitHamSongMetadata();
    static int sHamSaveVer;

    /** The song's name. */
    String mName; // 0x50
    /** The song's artist(s). */
    String mArtist; // 0x58
    String unk60; // 0x60
    /** The song's album. */
    String mAlbumName; // 0x68
    /** Is this song a Wavegroup cover? */
    bool mIsCover; // 0x70
    /** Is this song a medley/mashup? Leftover from DC1. */
    bool mIsMedley; // 0x71
    /** Is authoring complete for this song?
        Marked FALSE for unreleased DLC and TRUE for everything else. */
    bool mIsComplete; // 0x72
    /** Is this song fake? (i.e. hide it from the song select menu) */
    bool mIsFake; // 0x73
    /** The song's rank. The higher the number, the more difficult this song is. */
    float mRank; // 0x74
    /** The song's rating (i.e. family friendly, supervision required).
        In DC, everything is marked 2 (supervision required). */
    short mRating; // 0x78
    /** The assigned character for this song. Leftover from DC1.
        Unused in DC3 in favor of default/backup characters below. */
    Symbol mCharacter; // 0x7c
    /** The assigned gender for this song. Should match the character. */
    HamGender mGender; // 0x80
    /** The length of this song, in ms. */
    int mLength; // 0x84
    /** The overall BPM of this song. */
    int mBpm; // 0x88
    /** Does this song have extra authoring files via an alternate path? */
    bool mAlternatePath; // 0x8c
    /** This song's English pronunciations. */
    std::vector<String> mPronunciations; // 0x90
    /** This song's pronunciations in other languages. */
    std::vector<PronunciationsLoc> mPronunciationsLocalized; // 0x9c
    /** This song's authored midi events. */
    std::map<int, Symbol> mMidiEvents; // 0xa8
    /** The assigned character/outfit combo for this song. */
    Symbol mDefaultChar; // 0xc0
    /** The fallback outfit for the assigned character,
        in case the original outfit is locked. */
    Symbol mDefaultCharAlt; // 0xc4
    /** The fallback character/outfit combo for this song,
        in case the original character is locked. */
    Symbol mBackupChar; // 0xc8
    /** The fallback outfit for the fallback character,
        in case the fallback's original outfit is locked. */
    Symbol mBackupCharAlt; // 0xcc
    /** The assigned venue for this song. */
    Symbol mVenue; // 0xd0
    /** The fallback venue for this song, in case the original venue is locked. */
    Symbol mBackupVenue; // 0xd4
    /** The year this song came out. */
    int mYearReleased; // 0xd8
    int mDJIntensityRank; // 0xdc
};
