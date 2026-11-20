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
        Symbol song_moves; // 0x0
        Symbol song_audio; // 0x4
        Symbol mode; // 0x8
        float jump_start; // 0xc
        float jump_end; // 0x10
        Symbol config; // 0x14
        float loop_start; // 0x18
        float loop_end; // 0x1c
        bool decoding; // 0x20
        bool timey_wimey; // 0x21
        Symbol intro_cam_shot; // 0x24
        Symbol outro_cam_shot; // 0x28
        Symbol crew_0_override; // 0x2c
        Symbol crew_1_override; // 0x30
        int total_score; // 0x34
        int stars; // 0x38
    };

    SongSequence();
    virtual ~SongSequence();
    virtual DataNode Handle(DataArray *, bool);

    void Init();
    void Clear();
    bool Done() const;
    // bool Empty() const;
    void Add(const DataArray *);
    // void Add(const Entry &);
    bool DoNext(bool, bool);
    void OnSongLoaded();
    // void OnRhythmBattleComboFull();
    void LoadNextSongAudio();
    Symbol GetIntroCamShot() const;
    Symbol GetOutroCamShot() const;

    // oops! all public
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
