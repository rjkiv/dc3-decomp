#include "hamobj/SongLayout.h"
#include "hamobj/MoveMgr.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "utl/BinStream.h"
#include "utl/Std.h"
#include <cstring>

SongLayout::SongLayout() {
    if (TheMoveMgr) {
        TheMoveMgr->RegisterSongLayout(this);
    }
}

SongLayout::~SongLayout() {
    if (TheMoveMgr) {
        TheMoveMgr->UnRegisterSongLayout(this);
    }
}

BEGIN_HANDLERS(SongLayout)
    HANDLE(add_pattern, AddPattern)
    HANDLE(add_section, AddSection)
    HANDLE_EXPR(pattern_name, GetPatternName(_msg->Int(2)))
    HANDLE_EXPR(pattern_count, (int)mSongPatterns.size())
    HANDLE_EXPR(pattern_size, (int)mSongPatterns[_msg->Int(2)].mElements.size())
    HANDLE_EXPR(first_measure, mSongPatterns[_msg->Int(2)].mInitialMeasureRange.start)
    HANDLE_EXPR(move_count, mSongPatterns[_msg->Int(2)].mNumMoves)
    HANDLE_ACTION(add_move, AddPatternMove(_msg->Int(2), _msg->ForceSym(3)))
    HANDLE_ACTION(replace_move, SetReplacerMove(_msg->Int(2), _msg->ForceSym(3)))
    HANDLE_ACTION(clear_moves, ClearMoves(_msg->Int(2)))
    HANDLE_ACTION(clear_all_moves, ClearAllPatternMoves())
    HANDLE_EXPR(measure_in_pattern, MeasureInPattern(_msg->Int(2), _msg->Int(3)))
    HANDLE_EXPR(first_unfilled_pattern, FirstUnfilledPattern())
    HANDLE_EXPR(first_unfilled_replacer, FirstUnfilledReplacer())
    HANDLE_EXPR(replacer_first_measure, ReplacerFirstMeasure(_msg->Int(2)))
    HANDLE_ACTION(dump_patterns, DumpPatterns())
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

void SongLayout::ClearMoves(int pattern) {
    mSongPatterns[pattern].mNumMoves = 0;
    auto itEnd = mSongPatterns[pattern].mMoveParents.end();
    auto it = mSongPatterns[pattern].mMoveParents.begin();
    for (; it != itEnd; ++it) {
        (*it) = nullptr;
    }
}

bool SongLayout::MeasureInPattern(int measure, int pattern) const {
    Range r = mSongPatterns[pattern].mInitialMeasureRange;
    return measure >= r.start && measure <= r.end;
}

BEGIN_CUSTOM_PROPSYNC(SongSection)
    SYNC_PROP(measure_range, o.mMeasureRange)
    SYNC_PROP(pattern_range, o.mPatternRange)
    SYNC_PROP(pattern, o.mPattern)
END_CUSTOM_PROPSYNC

BEGIN_CUSTOM_PROPSYNC(SongPattern)
    SYNC_PROP(name, o.mName)
    SYNC_PROP(initial_measure_range, o.mInitialMeasureRange)
    SYNC_PROP(elements, o.mElements)
END_CUSTOM_PROPSYNC

BEGIN_PROPSYNCS(SongLayout)
    SYNC_PROP(patterns, mSongPatterns)
    SYNC_PROP(sections, mSongSections)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BinStream &operator<<(BinStream &bs, const SongPattern &s) {
    bs << s.mName;
    bs << s.mInitialMeasureRange.start;
    bs << s.mInitialMeasureRange.end;
    bs << s.mElements;
    return bs;
}

BinStream &operator<<(BinStream &bs, const SongSection &s) {
    bs << s.mMeasureRange.start;
    bs << s.mMeasureRange.end;
    bs << s.mPatternRange.start;
    bs << s.mPatternRange.end;
    bs << s.mPattern;
    return bs;
}

BEGIN_SAVES(SongLayout)
    SAVE_REVS(2, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    bs << mSongPatterns;
    bs << mSongSections;
END_SAVES

BEGIN_COPYS(SongLayout)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(SongLayout)
    BEGIN_COPYING_MEMBERS
        mSongPatterns.clear();
        mSongPatterns.insert(
            mSongPatterns.begin(), c->mSongPatterns.begin(), c->mSongPatterns.end()
        );
        mSongSections.clear();
        mSongSections.insert(
            mSongSections.begin(), c->mSongSections.begin(), c->mSongSections.end()
        );
    END_COPYING_MEMBERS
END_COPYS

BinStreamRev &operator>>(BinStreamRev &d, SongPattern &s) {
    d >> s.mName;
    d >> s.mInitialMeasureRange.start;
    d >> s.mInitialMeasureRange.end;
    d >> s.mElements;
    s.mMoveParents.resize(s.mElements.size());
    return d;
}

BinStreamRev &operator>>(BinStreamRev &d, SongSection &s) {
    d >> s.mMeasureRange.start;
    d >> s.mMeasureRange.end;
    d >> s.mPatternRange.start;
    d >> s.mPatternRange.end;
    d >> s.mPattern;
    return d;
}

INIT_REVS(2, 0)

BEGIN_LOADS(SongLayout)
    LOAD_REVS(bs)
    ASSERT_REVS(2, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    d >> mSongPatterns;
    d >> mSongSections;
    FOREACH (it, mSongSections) {
        FOREACH (pit, mSongPatterns) {
            if (pit->mName == it->mPattern) {
                it->unk14 = pit;
                break;
            }
        }
    }
END_LOADS

void SongLayout::ClearChosenPatterns() {
    FOREACH (it, mSongPatterns) {
        memset(&it->mMoveParents, 0, it->mMoveParents.size());
        it->mNumMoves = 0;
    }
}

void SongLayout::ClearAllPatternMoves() {
    FOREACH (it, mSongPatterns) {
        it->mNumMoves = 0;
        auto itEnd = it->mMoveParents.end();
        auto pit = it->mMoveParents.begin();
        for (; pit != itEnd; ++pit) {
            (*pit) = nullptr;
        }
    }
}

void SongLayout::DumpPatterns() const {
    int i = 0;
    MILO_LOG("SongLayout patterns:\n");
    FOREACH (it, mSongPatterns) {
        MILO_LOG(
            "pattern %d %s start=%d end=%d",
            i,
            it->mName.Str(),
            it->mInitialMeasureRange.start,
            it->mInitialMeasureRange.end
        );
        MILO_LOG(" size=%d moves=%d\n", (int)it->mElements.size(), it->mNumMoves);
        i++;
        for (int j = 0; j < it->mNumMoves; j++) {
            MILO_LOG("   %s\n", it->mMoveParents[j]->Name());
        }
    }
}

void SongLayout::AddPatternMove(int i1, Symbol s2) {
    if (mSongPatterns[i1].mNumMoves < mSongPatterns[i1].mElements.size()) {
        int numMoves = mSongPatterns[i1].mNumMoves;
        mSongPatterns[i1].mElements[numMoves] = s2;
        mSongPatterns[i1].mMoveParents[numMoves] = TheMoveMgr->Graph().GetMoveParent(s2);
        mSongPatterns[i1].mNumMoves++;
    }
}

void SongLayout::SetReplacerMove(int i, Symbol s2) {
    MILO_ASSERT(mMoveReplacers.size() > i, 0x107);
    MILO_LOG(
        "Replacer %d replacing %s with %s at measures:",
        i,
        mMoveReplacers[i].unk0.Str(),
        s2.Str()
    );
    for (int n = 0; n < mMoveReplacers[i].mMeasures.size(); n++) {
        MILO_LOG(" %d", mMoveReplacers[i].mMeasures[n] + 1);
    }
    MILO_LOG("\n");
    mMoveReplacers[i].unk4 = s2;
    mMoveReplacers[i].unk8 = TheMoveMgr->Graph().GetMoveParent(s2);
}

int SongLayout::ReplacerFirstMeasure(int i) const {
    MILO_ASSERT(i < mMoveReplacers.size(), 0x150);
    MILO_ASSERT(mMoveReplacers[i].mMeasures.size() > 0, 0x151);
    return mMoveReplacers[i].mMeasures[0];
}

int SongLayout::FirstUnfilledPattern() const {
    int count = 0;
    for (auto it = mSongPatterns.begin();
         it != mSongPatterns.end() && it->mNumMoves >= it->mElements.size();
         ++it) {
        count++;
    }
    if (count >= mSongPatterns.size()) {
        count = -1;
    }
    return count;
}

int SongLayout::FirstUnfilledReplacer() const {
    for (int i = 0; i < mMoveReplacers.size(); i++) {
        if (mMoveReplacers[i].unk8 == 0) {
            return i;
        }
    }
    return -1;
}

DataNode SongLayout::AddPattern(DataArray *a) {
    DataArray *arr = a->Array(2);
    SongPattern pattern;
    pattern.mName = arr->Sym(0);
    pattern.mInitialMeasureRange.start = arr->Int(1);
    pattern.mInitialMeasureRange.end = arr->Int(2);
    pattern.mNumMoves = 0;
    for (int i = 3; i < arr->Size(); i++) {
        pattern.mElements.push_back(arr->Sym(i));
        pattern.mMoveParents.push_back(nullptr);
    }
    mSongPatterns.push_back(pattern);
    return 0;
}

DataNode SongLayout::AddSection(DataArray *a) {
    DataArray *arr = a->Array(2);
    SongSection section;
    section.mMeasureRange.start = arr->Int(0);
    section.mMeasureRange.end = arr->Int(1);
    section.mPattern = arr->Sym(2);
    section.mPatternRange.start = arr->Int(3);
    section.mPatternRange.end = arr->Int(4);
    mSongSections.push_back(section);
    return 0;
}

DataNode SongLayout::GetPatternName(int idx) const {
    return mSongPatterns[idx].mName.Str();
}
