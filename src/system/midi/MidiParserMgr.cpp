#include "midi/MidiParserMgr.h"
#include "beatmatch/GemListInterface.h"
#include "math/Utl.h"
#include "midi/Midi.h"
#include "midi/MidiConstants.h"
#include "obj/DataFile.h"
#include "obj/Dir.h"
#include "os/Debug.h"
#include "os/System.h"
#include "utl/TimeConversion.h"
#include "utl/Std.h"

MidiParserMgr *TheMidiParserMgr;

#pragma region MidiReceiver

MidiParserMgr::MidiParserMgr(GemListInterface *gListInt, Symbol sym)
    : mGems(gListInt), mLoaded(0), mFilename(0), mTrackName(), mSongName(), mTrackNames(),
      unk6c(true), unk6d(true) {
    MILO_ASSERT(!TheMidiParserMgr, 0x27);
    TheMidiParserMgr = this;
    SetName("midiparsermgr", ObjectDir::Main());
    mSongName = sym;
    MidiParser::Init();
    DataArray *arr = SystemConfig("beatmatcher")->FindArray("midi_parsers", false);
    if (arr) {
        DataArray *initArr = arr->FindArray("init", false);
        if (initArr)
            initArr->ExecuteScript(1, this, 0, 1);
        else
            MILO_NOTIFY(
                "Could not find init block in midi parser array, no parsers will be constructed"
            );
    }
}

MidiParserMgr::~MidiParserMgr() {
    MidiParser::ClearManagedParsers();
    TheMidiParserMgr = nullptr;
}

void MidiParserMgr::OnNewTrack(int) {
    MemDoTempAllocations tmp;
    MILO_ASSERT(!mSongName.Null(), 0x7C);
    FreeAllData();
    mNoteOns.resize(128, -1);
    mText.reserve(2000);
    unk6c = true;
}

void MidiParserMgr::OnEndOfTrack() {
    if (!mTrackName.Null()) {
        if (mText.size() > 2000) {
            MILO_NOTIFY(
                "%s track %s has %d text events which is over the limit of %d, if that is correct contact James to increase kMaxTextSize",
                mFilename,
                mTrackName,
                mText.size(),
                2000
            );
        }
        if (mGems)
            mGems->SetTrack(mTrackName);
        std::list<MidiParser *> &parsers = MidiParser::GetParsers();
        for (std::list<MidiParser *>::iterator it = parsers.begin(); it != parsers.end();
             ++it) {
            MidiParser *cur = *it;
            if (cur->TrackName() == mTrackName) {
                int numnotes = cur->ParseAll(mGems, mText);
                if (numnotes > 20000) {
                    MILO_NOTIFY(
                        "%s track %s has %d notes which is over the limit of %d, if that is correct contact James to increase kMaxNoteSize",
                        mFilename,
                        mTrackName,
                        numnotes,
                        20000
                    );
                }
            }
        }
        FreeAllData();
        mTrackName = "";
    }
}

void MidiParserMgr::OnMidiMessage(
    int tick, unsigned char status, unsigned char data1, unsigned char data2
) {
    int i28;
    bool created = CreateNote(tick, status, data1, i28);
    if (created) {
        std::list<MidiParser *> &parsers = MidiParser::GetParsers();
        for (std::list<MidiParser *>::iterator it = parsers.begin(); it != parsers.end();
             ++it) {
            MidiParser *cur = *it;
            if (cur->TrackName() == mTrackName) {
                cur->ParseNote(i28, tick, data1);
            }
        }
    }
}

void MidiParserMgr::OnText(int tick, const char *text, unsigned char type) {
    if (type == kTrackname)
        OnTrackName(text);
    else if (type == kLyricEvent || type == kTextEvent) {
        MemDoTempAllocations tmp;
        MidiParser::VocalEvent vocEv;
        vocEv.startTick = tick;
        if (*text == '[') {
            DataArray *parsed = ParseText(text, tick);
            if (!parsed)
                return;
            vocEv.data = parsed;
            parsed->Release();
        } else
            vocEv.data = text;
        mText.push_back(vocEv);
    }
}

void MidiParserMgr::SetMidiReader(MidiReader *mr) {
    MidiReceiver::SetMidiReader(mr);
    mFilename = mr->GetFilename();
}

#pragma endregion
#pragma region Hmx::Object

BEGIN_PROPSYNCS(MidiParserMgr)
    SYNC_PROP(song_name, mSongName)
END_PROPSYNCS

#pragma endregion
#pragma region MidiParserMgr

const char *MidiParserMgr::StripEndBracket(char *c1, const char *cc2) {
    char *ret = c1;
    const char *ptr;
    for (ptr = cc2; *ptr != '\0' && *ptr != ']'; ptr++) {
        *ret++ = *ptr;
    }

    if (*ptr == '\0') {
        MILO_NOTIFY(
            "MidiParser: %s, track %s event \"%s\" is missing right bracket",
            mFilename,
            mTrackName,
            cc2
        );
    }

    *ret = '\0';
    return c1;
}

DataArray *MidiParserMgr::ParseText(const char *str, int tick) {
    MILO_ASSERT(strlen(str) < 256, 0xF3);
    char buf[256];
    StripEndBracket(buf, str + 1);
    DataArray *parsed = nullptr;
    MILO_TRY { parsed = DataReadString(buf); }
    MILO_CATCH(errMsg) {
        parsed = nullptr;
        MILO_NOTIFY(MakeString(
            "MidiParser: %s, track %s, tick %d, event \"%s\" has bad format: %s",
            TheMidiParserMgr->mFilename,
            mTrackName,
            tick,
            buf,
            errMsg
        ));
    }
    return parsed;
}

void MidiParserMgr::FinishLoad() {
    DataArray *arr = SystemConfig("beatmatcher")->FindArray("midi_parsers", false);
    if (arr) {
        DataArray *finishArr = arr->FindArray("finish_loading", false);
        if (finishArr) {
            finishArr->ExecuteScript(1, this, 0, 1);
        }
    }
    mLoaded = true;
}

bool MidiParserMgr::CreateNote(
    int tick, unsigned char status, unsigned char data1, int &start_tick
) {
    if (mNoteOns.empty()) {
        if (unk6c) {
            MILO_NOTIFY("%s has a track that was not named.", mFilename);
            unk6c = false;
        }
        return true;
    } else {
        switch (MidiGetType(status)) {
        case kNoteOn:
            if (mNoteOns[data1] == -1) {
                mNoteOns[data1] = tick;
            } else
                Error(MakeString("Double note-on (%d)", data1), tick);
            break;
        case kNoteOff: {
            int onTick = mNoteOns[data1];
            if (onTick == -1) {
                Error(MakeString("Double note-off (%d)", data1), tick);
            } else {
                mNoteOns[data1] = -1;
                start_tick = onTick;
                return true;
            }
            break;
        }
        default:
            break;
        }
        return false;
    }
}

void MidiParserMgr::Reset(int i) {
    if (mLoaded && unk6d) {
        float beat = TickToBeat(i);
        std::list<MidiParser *> &parsers = MidiParser::GetParsers();
        for (std::list<MidiParser *>::iterator it = parsers.begin(); it != parsers.end();
             ++it) {
            (*it)->Reset(beat);
        }
    }
}

void MidiParserMgr::Reset() {
    if (mLoaded) {
        std::list<MidiParser *> &parsers = MidiParser::GetParsers();
        for (std::list<MidiParser *>::iterator it = parsers.begin(); it != parsers.end();
             ++it) {
            (*it)->Reset(-2 * kHugeFloat);
        }
    }
}

void MidiParserMgr::Poll() {
    if (unk6d) {
        std::list<MidiParser *> &parsers = MidiParser::GetParsers();
        for (std::list<MidiParser *>::iterator it = parsers.begin(); it != parsers.end();
             ++it) {
            (*it)->Poll();
        }
    }
}

MidiParser *MidiParserMgr::GetParser(Symbol s) {
    std::list<MidiParser *> &parsers = MidiParser::GetParsers();
    for (std::list<MidiParser *>::iterator it = parsers.begin(); it != parsers.end();
         ++it) {
        if (s == (*it)->Name())
            return *it;
    }
    return nullptr;
}

DataEventList *MidiParserMgr::GetEventsList() {
    MidiParser *evParser = GetParser("events_parser");
    MILO_ASSERT(evParser, 0x17E);
    return evParser->Events();
}

void MidiParserMgr::FreeAllData() {
    ClearAndShrink(mText);
    ClearAndShrink(mNoteOns);
}

void MidiParserMgr::OnTrackName(Symbol s) {
    if (std::find(mTrackNames.begin(), mTrackNames.end(), s) != mTrackNames.end()) {
        std::list<MidiParser *> &parsers = MidiParser::GetParsers();
        for (std::list<MidiParser *>::iterator it = parsers.begin(); it != parsers.end();
             ++it) {
            MidiParser *cur = *it;
            if (cur->TrackName() == s) {
                cur->Clear();
            }
        }
    } else
        mTrackNames.push_back(s);
    mTrackName = s;
}
