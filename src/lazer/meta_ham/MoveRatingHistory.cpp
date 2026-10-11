#include "meta_ham/MoveRatingHistory.h"
#include "meta/FixedSizeSaveable.h"
#include "meta/FixedSizeSaveableStream.h"

void MoveRatingHistory::SaveFixed(FixedSizeSaveableStream &fs) const {
    int size = mMap.size();
    static int sMaxSize = 0x4000;
    if (size > 0x4000) {
        MILO_NOTIFY(
            "The move awards history size is greater than the maximum supplied! size=%i max=%i",
            size,
            sMaxSize
        );
        size = 0x4000;
    }
    fs.Tell();
    fs << size;
    FOREACH (it, mMap) {
        FixedSizeSaveable::SaveSymbolID(fs, it->first.mMove);
        for (int i = 0; i < 4; i++) {
            fs << it->second.mRating[i];
        }
    }
    if (size < 0x4000) {
        FixedSizeSaveable::PadStream(fs, (0x4000 - size) * 20);
    }
    const_cast<MoveRatingHistory *>(this)->mDirty = false;
}

void MoveRatingHistory::LoadFixed(FixedSizeSaveableStream &fs, int i2) {
    if (mMap.size() > 0) {
        MILO_NOTIFY("Move award history map is not empty on load!");
        mMap.clear();
    }
    int size;
    fs >> size;
    for (int i = 0; i < size; i++) {
        Key key;
        FixedSizeSaveable::LoadSymbolFromID(fs, key.mMove);
        for (int j = 0; j < 4; j++) {
            fs >> (int &)mMap[key].mRating[j];
        }
    }
    if (size < 0x4000) {
        FixedSizeSaveable::DepadStream(fs, (0x4000 - size) * 20);
    }
}

void MoveRatingHistory::Clear() {
    mMap.clear();
    mDirty = false;
}

int MoveRatingHistory::GetRating(Symbol s1, int i2) {
    Key key;
    key.mMove = s1;
    if (HasRatingHistory(key)) {
        return mMap[key].mRating[i2];
    } else {
        return -1;
    }
}

int MoveRatingHistory::SaveSize(int) { return 0x50004; }

void MoveRatingHistory::AddHistory(Symbol s1, int i2) {
    Key key;
    key.mMove = s1;
    RatingHistory &history = mMap[key];
    for (int i = 1; i < 4; i++) {
        history.mRating[i] = history.mRating[i - 1];
    }
    history.mRating[0] = (MoveRating)i2;
    mDirty = true;
}
