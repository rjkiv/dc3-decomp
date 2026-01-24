#include "meta_ham/MoveRatingHistory.h"
#include "meta/FixedSizeSaveable.h"
#include "meta/FixedSizeSaveableStream.h"

void MoveRatingHistory::SaveFixed(FixedSizeSaveableStream &fs) const {
    int size = unk8.size();
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
    FOREACH (it, unk8) {
        FixedSizeSaveable::SaveSymbolID(fs, it->first.unk0);
        for (int i = 0; i < 4; i++) {
            fs << it->second.unk0[i];
        }
    }
    if (size < 0x4000) {
        FixedSizeSaveable::PadStream(fs, (0x4000 - size) * 20);
    }
    const_cast<MoveRatingHistory *>(this)->unk20 = false;
}

void MoveRatingHistory::LoadFixed(FixedSizeSaveableStream &fs, int i2) {
    if (unk8.size() > 0) {
        MILO_NOTIFY("Move award history map is not empty on load!");
        unk8.clear();
    }
    int size;
    fs >> size;
    for (int i = 0; i < size; i++) {
        Key key;
        FixedSizeSaveable::LoadSymbolFromID(fs, key.unk0);
        for (int j = 0; j < 4; j++) {
            fs >> (int &)unk8[key].unk0[j];
        }
    }
    if (size < 0x4000) {
        FixedSizeSaveable::DepadStream(fs, (0x4000 - size) * 20);
    }
}

void MoveRatingHistory::Clear() {
    unk8.clear();
    unk20 = false;
}

int MoveRatingHistory::GetRating(Symbol s1, int i2) {
    Key key;
    key.unk0 = s1;
    if (HasRatingHistory(key)) {
        return unk8[key].unk0[i2];
    } else {
        return -1;
    }
}

int MoveRatingHistory::SaveSize(int) { return 0x50004; }

void MoveRatingHistory::AddHistory(Symbol s1, int i2) {
    Key key;
    key.unk0 = s1;
    RatingHistory &history = unk8[key];
    MoveRating old = history.unk0[0];
    history.unk0[1] = old;
    history.unk0[2] = old;
    history.unk0[3] = old;
    history.unk0[0] = (MoveRating)i2;
    unk20 = true;
}
