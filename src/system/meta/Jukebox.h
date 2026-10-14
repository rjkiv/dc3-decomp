#pragma once
#include <vector>

struct JukeboxItem {
    JukeboxItem(int id, int lastplayed) : mSongID(id), mLastPlayed(lastplayed) {}
    ~JukeboxItem() {}
    bool operator==(int x) const { return mSongID == x; }
    int mSongID; // 0x0
    int mLastPlayed; // 0x4
};

class Jukebox {
private:
    void AddItem(int, int);

    std::vector<JukeboxItem> mItems; // 0x0
    int mNumPlayed; // 0xc

public:
    Jukebox(int);
    void Play(int);
    int Pick(const std::vector<int> &);
};
