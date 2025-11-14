#include "char/CharDriverMidi.h"
#include "char/CharDriver.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "utl/Symbol.h"

CharDriverMidi::CharDriverMidi() : mClipFlags(0), mBlendOverridePct(1.0f) {}

CharDriverMidi::~CharDriverMidi() {}

BEGIN_PROPSYNCS(CharDriverMidi)
    SYNC_PROP(parser, mParser)
    SYNC_PROP(flag_parser, mFlagParser)
    SYNC_PROP(blend_override_pct, mBlendOverridePct)
    SYNC_SUPERCLASS(CharDriver)
END_PROPSYNCS

BEGIN_SAVES(CharDriverMidi)
    SAVE_REVS(7, 0)
    SAVE_SUPERCLASS(CharDriver)
    bs << mParser;
    bs << mFlagParser;
    bs << mBlendOverridePct;
END_SAVES

BEGIN_COPYS(CharDriverMidi)
    COPY_SUPERCLASS(CharDriver)
    CREATE_COPY_AS(CharDriverMidi, c)
    BEGIN_COPYING_MEMBERS_FROM(c)
        COPY_MEMBER(unke0)
        COPY_MEMBER(mParser)
        COPY_MEMBER(mFlagParser)
        COPY_MEMBER(mBlendOverridePct)
    END_COPYING_MEMBERS
END_COPYS

BEGIN_LOADS(CharDriverMidi)
    LOAD_REVS(bs)
    ASSERT_REVS(7, 0)
    LOAD_SUPERCLASS(CharDriverMidi)
    if (d.rev < 7) {
        unk70.Load(bs, false, mClips);
    }
    if (d.rev == 2) {
        String str;
        bs >> str;
    } else if (d.rev > 3)
        bs >> mParser;
    if (d.rev > 4)
        bs >> mFlagParser;
    if (d.rev > 5)
        bs >> mBlendOverridePct;
END_LOADS

void CharDriverMidi::Poll() { CharDriver::Poll(); }

void CharDriverMidi::PollDeps(
    std::list<Hmx::Object *> &changedBy, std::list<Hmx::Object *> &change
) {
    CharDriver::PollDeps(changedBy, change);
}

void CharDriverMidi::Enter() {}

void CharDriverMidi::Exit() { CharDriver::Exit(); }

DataNode CharDriverMidi::OnMidiParser(DataArray *da) { return 0; }

DataNode CharDriverMidi::OnMidiParserFlags(DataArray *da) { return 0; }

DataNode CharDriverMidi::OnMidiParserGroup(DataArray *da) { return NULL_OBJ; }

BEGIN_HANDLERS(CharDriverMidi)
    HANDLE(midi_parser, OnMidiParser)
    HANDLE(midi_parser_group, OnMidiParserGroup)
    HANDLE(midi_parser_flags, OnMidiParserFlags)
    HANDLE_SUPERCLASS(CharDriver)
END_HANDLERS
