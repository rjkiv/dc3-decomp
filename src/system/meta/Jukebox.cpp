#include "meta/Jukebox.h"
#include "math/Rand.h"
#include "os/Debug.h"
#include "utl/Std.h"
#include <algorithm>

Jukebox::Jukebox(int numItems) : mNumPlayed(0) { mItems.reserve(numItems); }

void Jukebox::AddItem(int i1, int i2) {
    JukeboxItem item(i1, i2);
    if (mItems.size() == mItems.capacity()) {
        MILO_NOTIFY("Growing jukebox capacity");
    }
    mItems.push_back(item);
}

int Jukebox::Pick(const std::vector<int> &valid_names) {
    MILO_ASSERT(!valid_names.empty(), 0x26);
    std::vector<JukeboxItem> items;
    FOREACH (it, valid_names) {
        auto jit = std::find(mItems.begin(), mItems.end(), *it);
        if (jit == mItems.end()) {
            AddItem(*it, -1);
            items.push_back(mItems.back());
        } else {
            items.push_back(*jit);
        }
    }
    std::random_shuffle(items.begin(), items.end());
    return items[RandomInt(0, items.size())].mSongID;
}

void Jukebox::Play(int x) {
    auto jit = std::find(mItems.begin(), mItems.end(), x);
    if (jit == mItems.end()) {
        AddItem(x, mNumPlayed);
    } else {
        jit->mLastPlayed = mNumPlayed;
    }
    mNumPlayed++;
}
