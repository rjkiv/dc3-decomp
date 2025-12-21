#pragma once
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/FileCache.h"
#include "rndobj/Poll.h"
#include "stl/_vector.h"
#include "utl/Symbol.h"

class SongSequence : public RndPollable {
public:
    struct Entry {
        int unk0;
        int unk4;
        int unk8;
        int unkc;
        int unk10;
        int unk14;
        int unk18;
        int unk1c;
        int unk20;
        Symbol unk24;
    };

    virtual ~SongSequence();
    virtual DataNode Handle(DataArray *, bool);

    SongSequence();
    bool Done() const;
    void LoadNextSongAudio();
    Symbol GetIntroCamShot() const;
    Symbol GetOutroCamShot() const;
    void OnSongLoaded();
    void Clear();
    bool DoNext(bool, bool);
    void Init();
    void Add(DataArray const *);

protected:
    std::vector<Entry> unk8;
    int mCurrentIndex; // 0x14
    float unk18;
    float unk1c;
    u32 unk20;
    u32 unk24;
    u32 unk28;
    FileCache *unk2c;
};
