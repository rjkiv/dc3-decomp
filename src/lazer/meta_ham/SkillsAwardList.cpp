#include "meta_ham/SkillsAwardList.h"
#include "hamobj/HamGameData.h"
#include "meta/FixedSizeSaveableStream.h"
#include "meta_ham/HamSongMgr.h"
#include "obj/Data.h"
#include "obj/Object.h"

void SkillsAwardList::SaveFixed(FixedSizeSaveableStream &stream) const {
    int size = mAwardList.size();
    if (size > 0x4000) {
        static int sMaxSize = 0x4000;
        MILO_NOTIFY(
            "The Skills awards size is greater than the maximum supplied! size=%i max=%i",
            size,
            sMaxSize
        );
        size = 0x4000;
    }
    stream.Tell();
    stream << size;
    FOREACH (it, mAwardList) {
        stream << it->first.mSongID;
        for (int i = 0; i < 4; i++) {
            SaveSymbolID(stream, it->first.unk4[i]);
        }
        stream << it->second;
    }
    if (size < 0x4000) {
        PadStream(stream, (0x4000 - size) * 0x18);
    }
    const_cast<SkillsAwardList *>(this)->mDirty = false;
}

void SkillsAwardList::LoadFixed(FixedSizeSaveableStream &stream, int i2) {
    if (mAwardList.size() > 0) {
        MILO_NOTIFY("Skills awardlist map is not empty on load!");
        mAwardList.clear();
    }
    int size;
    stream >> size;
    for (int i = 0; i < size; i++) {
        Key key;
        stream >> key.mSongID;
        for (int j = 0; j < 4; j++) {
            LoadSymbolFromID(stream, key.unk4[j]);
        }
        SkillsAward award = (SkillsAward)0;
        stream >> (int &)award;
        mAwardList[key] = award;
    }
    if (size < 0x4000) {
        DepadStream(stream, (0x4000 - size) * 0x18);
    }
}

bool SkillsAwardList::HasFailure() const {
    FOREACH (it, mAwardList) {
        if (it->second == 1) {
            return true;
        }
    }
    return false;
}

int SkillsAwardList::SaveSize(int i1) { return 0x60004; }

SkillsAward SkillsAwardList::GetAward(DataArray *da) {
    return GetAward(TheGameData->GetSong(), da);
}

SkillsAward SkillsAwardList::GetAward(Symbol song, DataArray *da) {
    Key key;
    key.mSongID = TheHamSongMgr.GetSongIDFromShortName(song);
    for (int i = 0; i < da->Size() && i < 4; i++) {
        key.unk4[i] = da->Obj<Hmx::Object>(i)->Name();
    }
    if (mAwardList.find(key) != mAwardList.end()) {
        return mAwardList[key];
    } else {
        return (SkillsAward)0;
    }
}

int SkillsAwardList::AwardCount(SkillsAward sa) const {
    int count = 0;
    FOREACH (it, mAwardList) {
        if (it->second == sa) {
            count++;
        }
    }
    return count;
}

void SkillsAwardList::Clear() {
    mAwardList.clear();
    mDirty = false;
}

void SkillsAwardList::SetAward(DataArray *a, SkillsAward award) {
    SetAward(TheGameData->GetSong(), a, award);
}

void SkillsAwardList::SetAward(Symbol song, DataArray *a, SkillsAward award) {
    Key key;
    key.mSongID = TheHamSongMgr.GetSongIDFromShortName(song);
    for (int i = 0; i < a->Size() && i < 4; i++) {
        key.unk4[i] = a->Obj<Hmx::Object>(i)->Name();
    }
    if (mAwardList.find(key) == mAwardList.end() || mAwardList[key] < award) {
        mAwardList[key] = award;
        mDirty = true;
    }
}
