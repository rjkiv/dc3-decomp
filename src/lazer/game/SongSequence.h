#pragma once
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/FileCache.h"
#include "rndobj/Poll.h"
#include "utl/Symbol.h"

class SongSequence : public RndPollable {
public:
    // size 0x3c
    struct Entry {
        Symbol unk0; // 0x0 - song shortname
        Symbol unk4; // 0x4 - song shortname
        Symbol unk8; // 0x8 - game mode
        float unkc;
        float unk10;
        Symbol unk14; // 0x14 - hollaback config?
        float unk18;
        float unk1c;
        bool unk20;
        bool unk21;
        Symbol mIntroCamShot; // 0x24
        Symbol mOutroCamShot; // 0x28
        Symbol unk2c; // 0x2c - crew1?
        Symbol unk30; // 0x30 - crew2?
        int unk34; // 0x34 - combined score across p1 and p2?
        int unk38; // 0x38 - stars?
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
    int CurrentIndex() const { return m_Current; }
    bool GetHaveReenteredCharacters() const { return m_HaveReenteredCharacters; }
    void SetHaveReenteredCharacters(bool b) {
        m_HaveReenteredCharacters = b;
    } // 0x28 - venueEntered flag

protected:
    std::vector<Entry> m_Entries; // 0x8
    int m_Current; // 0x14
    float m_PrevChangeTime; // 0x18
    float m_TimeSinceLoad; // 0x1c
    float m_MasterVolume; // 0x20
    float m_BeginLoadTime; // 0x24
    bool m_HaveReenteredCharacters; // 0x28
    FileCache *sCache; // 0x2c
};

extern SongSequence TheSongSequence;
