#pragma once
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/FileCache.h"
#include "rndobj/Poll.h"
#include "stl/_vector.h"
#include "utl/Symbol.h"

class SongSequence : public RndPollable {
public:
    // size 0x3c
    struct Entry {
        Symbol unk0;
        Symbol unk4;
        Symbol unk8;
        float unkc;
        float unk10;
        Symbol unk14;
        float unk18;
        float unk1c;
        bool unk20;
        bool unk21;
        Symbol mIntroCamShot; // 0x24
        Symbol mOutroCamShot; // 0x28
        Symbol unk2c; // 0x2c - crew1?
        Symbol unk30; // 0x30 - crew2?
        int unk34;
        int unk38;
    };

    SongSequence();
    virtual ~SongSequence();
    virtual DataNode Handle(DataArray *, bool);

    bool Done() const;
    void LoadNextSongAudio();
    Symbol GetIntroCamShot() const;
    Symbol GetOutroCamShot() const;
    void OnSongLoaded();
    void Clear();
    bool DoNext(bool, bool);
    void Init();
    void Add(const DataArray *);
    int CurrentIndex() const { return mCurrentIndex; }

protected:
    std::vector<Entry> mEntries; // 0x8
    int mCurrentIndex; // 0x14
    float unk18;
    float unk1c;
    u32 unk20;
    float unk24;
    bool unk28;
    FileCache *mFileCache; // 0x2c
};

extern SongSequence TheSongSequence;
