#pragma once

#include "meta/SongMetadata.h"
#include "obj/Data.h"
#include "stl/_vector.h"
#include "utl/BinStream.h"
#include "utl/Symbol.h"

struct PronunciationsLoc {
    int unk0;
    u64 unk4;
};

class HamSongMetadata : public SongMetadata {
public:
    // SongMetadata
    virtual ~HamSongMetadata();
    virtual DataNode Handle(DataArray *, bool);
    virtual void Save(BinStream &);
    virtual void Load(BinStream &);
    virtual bool IsVersionOK() const;
    virtual bool HasAlternatePath() const;

    HamSongMetadata();
    HamSongMetadata(DataArray *, DataArray *, bool);
    bool IsCover() const;
    bool IsMedley() const;
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

    String mName; // 0x50
    String mArtist; // 0x58
    String unk60;
    String mAlbumName; // 0x68
    bool isCover; // 0x70
    bool isMedley; // 0x71
    bool unk72;
    bool unk73;
    float mRank; // 0x74
    short mRating; // 0x78
    Symbol mCharacter; // 0x7c
    int mGender; // 0x80
    int mLength; // 0x84
    int mBpm; // 0x88
    bool mAlternatePath; // 0x8c
    std::vector<String> unk90;
    std::vector<PronunciationsLoc> unk9c;
    std::map<int, Symbol> unka8;
    Symbol unkc0;
    Symbol unkc4;
    Symbol unkc8;
    Symbol unkcc;
    Symbol unkd0;
    Symbol unkd4;

private:
    void InitHamSongMetadata();
};

int GetNoteNum(DataArray *, Symbol);
