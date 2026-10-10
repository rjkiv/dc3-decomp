#include "char/CharDriverMidi.h"
#include "char/CharClip.h"
#include "char/CharDriver.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "os/Debug.h"
#include "utl/Symbol.h"
#include "utl/TimeConversion.h"

CharDriverMidi::CharDriverMidi() : mClipFlags(0), mBlendOverridePct(1.0f) {}

BEGIN_HANDLERS(CharDriverMidi)
    HANDLE(midi_parser, OnMidiParser)
    HANDLE(midi_parser_group, OnMidiParserGroup)
    HANDLE(midi_parser_flags, OnMidiParserFlags)
    HANDLE_SUPERCLASS(CharDriver)
END_HANDLERS

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

INIT_REVS(7, 0)

BEGIN_LOADS(CharDriverMidi)
    LOAD_REVS(bs)
    ASSERT_REVS(7, 0)
    LOAD_SUPERCLASS(CharDriver)
    if (d.rev < 7) {
        mDefaultClip.Load(d.stream, false, mClips);
    }
    if (d.rev == 2) {
        String str;
        d >> str;
    } else if (d.rev > 3)
        d >> mParser;
    if (d.rev > 4)
        d >> mFlagParser;
    if (d.rev > 5)
        d >> mBlendOverridePct;
END_LOADS

void CharDriverMidi::Poll() { CharDriver::Poll(); }

void CharDriverMidi::PollDeps(
    std::list<Hmx::Object *> &changedBy, std::list<Hmx::Object *> &change
) {
    CharDriver::PollDeps(changedBy, change);
}

void CharDriverMidi::Enter() {
    unke0 = true;
    CharDriver::Enter();
    Hmx::Object *parserObj = Dir()->FindObject(mParser.Str(), true, true);
    if (parserObj) {
        parserObj->AddSink(this);
    }
    Hmx::Object *flagParserObj = Dir()->FindObject(mFlagParser.Str(), true, true);
    if (flagParserObj) {
        flagParserObj->AddSink(this);
    }
}

void CharDriverMidi::Exit() {
    CharDriver::Exit();
    Hmx::Object *parserObj = ObjectDir::Main()->Find<Hmx::Object>(mParser.Str(), false);
    if (parserObj) {
        parserObj->RemoveSink(this);
    }
    Hmx::Object *flagParserObj =
        ObjectDir::Main()->Find<Hmx::Object>(mFlagParser.Str(), false);
    if (flagParserObj) {
        flagParserObj->RemoveSink(this);
    }
}

DataNode CharDriverMidi::OnMidiParser(DataArray *da) {
    CharClip *clip;
    if (!unke0 && mDefaultClip) {
        clip = dynamic_cast<CharClip *>(mDefaultClip.Ptr());
    } else {
        clip = FindClip(da->Node(2), false);
    }
    if (clip) {
        float somefloat = da->Float(3);
        if (clip->PlayFlags() & 0x200) {
            somefloat = BeatToSeconds(somefloat + TheTaskMgr.Beat());
            somefloat -= TheTaskMgr.Seconds(TaskMgr::kTaskTRVideo);
            somefloat *= clip->AverageBeatsPerSecond();
        }
        MaxEq(somefloat, 0.0f);
        Play(clip, 0, somefloat * mBlendOverridePct, -somefloat, 0.0f);
        return 0;
    } else {
        return 0;
    }
}

DataNode CharDriverMidi::OnMidiParserFlags(DataArray *da) {
    mClipFlags = da->Int(2);
    return 0;
}

DataNode CharDriverMidi::OnMidiParserGroup(DataArray *da) {
    const char *name = da->Str(2);
    CharClipGroup *grp = mClips->Find<CharClipGroup>(name, false);
    if (!grp) {
        MILO_NOTIFY(
            "%s could not find group %s in %s", PathName(this), name, mClips->Name()
        );
        return 0;
    } else {
        CharClip *clip;
        if (!unke0 && mDefaultClip) {
            clip = dynamic_cast<CharClip *>(mDefaultClip.Ptr());
        } else {
            clip = grp->GetClip(mClipFlags);
        }
        if (!clip) {
            MILO_NOTIFY(
                "%s could not find clip with flags %d in %s",
                PathName(this),
                mClipFlags,
                PathName(grp)
            );
            return 0;
        } else {
            float somefloat = da->Float(3);
            if (clip->PlayFlags() & 0x200) {
                somefloat *= clip->AverageBeatsPerSecond();
            }
            MaxEq(somefloat, 0.0f);
            Play(clip, 0, -somefloat, 1e+30f, 0.0f)->mBlendWidth =
                somefloat * mBlendOverridePct;
            return 0;
        }
    }
}
