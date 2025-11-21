#include "hamobj/SongLayout.h"
#include "hamobj/MoveMgr.h"
#include "obj/Object.h"
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
    // HANDLE_EXPR(pattern_size, mSongPatterns[_msg->Int(2)].mElements.size())
END_HANDLERS

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

BEGIN_COPYS(SongLayout)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(SongLayout)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mSongPatterns)
        COPY_MEMBER(mSongSections)
        COPY_MEMBER(mMoveReplacers)
    END_COPYING_MEMBERS
END_COPYS
