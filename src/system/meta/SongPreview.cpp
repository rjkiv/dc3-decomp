#include "meta/SongPreview.h"
#include "SongMetadata.h"
#include "SongMgr.h"
#include "macros.h"
#include "movie/TexMovie.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "os/ContentMgr.h"
#include "os/Debug.h"
#include "os/System.h"
#include "synth/Faders.h"
#include "utl/SongInfoCopy.h"
#include "utl/Symbol.h"

SongPreview::SongPreview(const SongMgr &mgr)
    : mSongMgr(mgr), unk34(0), unk38(this), unk4c(0), mFader(0), mMusicFader(0),
      mCrowdSingFader(0), unk5c(0), unk60(0.0f), unk68(0.0f), unk70(0), unk7c(0.0f),
      unk80(0.0f), unk84(0.0f), unk88(0.0f), unk8c(0), unk8d(0), unk8e(0) {}

void SongPreview::ContentMounted(char const *contentName, char const *cc2) {
    MILO_ASSERT(contentName, 0xbf);

    Symbol s = Symbol(contentName);
    if (s == unk78) {
        unk78 = 0;
    }
}

void SongPreview::ContentFailed(char const *contentName) {
    MILO_ASSERT(contentName, 0xcb);

    Symbol sym = contentName;
    if (sym == unk78) {
        unk74 = 0;
        unk70 = 0;
        unk78 = 0;
    }
}

SongPreview::~SongPreview() { Terminate(); }

bool SongPreview::IsWaitingToDelete() const { return unk70 == 3; }

bool SongPreview::IsFadingOut() const { return unk70 == 5; }

void SongPreview::SetMusicVol(float f) {
    if (unk4c == 0) {
        return;
    }
    if (f < mMusicFader->GetMLevelTarget()) {
        mMusicFader->DoFade(f, 250.0f);
    } else {
        mMusicFader->DoFade(f, 1000.0f);
    }
}

void SongPreview::Init() {
    if (unk4c) {
        unk4c = true;
        unk74 = 0;
        unk78 = 0;
        if (unk34) {
            // do something
        }
        unk34 = 0;
        RELEASE(unk38);
        unk70 = 0;
        unk6c = true;
        DataArray *cfg = SystemConfig("sound", "song_select");
        cfg->FindData("loop_forever", unk6d, true);
        cfg->FindData("fade_time", unk64, true);
        cfg->FindData("attenuation", unk60, true);
        cfg->FindData("preview_db", unk68, true);
        unk64 *= 1000.0f;
        mFader = Hmx::Object::New<Fader>();
        mMusicFader = Hmx::Object::New<Fader>();
        mCrowdSingFader = Hmx::Object::New<Fader>();
        mCrowdSingFader->SetVolume(-96.0f);
    }
}

void SongPreview::Terminate() {
    if (unk4c) {
        unk4c = 0;
        DetachFader(mMusicFader);
        DetachFader(mCrowdSingFader);
        unk74 = 0;
        unk78 = 0;
        RELEASE(unk34);
        RELEASE(mFader);
        RELEASE(mMusicFader);
        RELEASE(mCrowdSingFader);

        if (unk8c) {
            TheContentMgr->UnregisterCallback(this, true);
            unk8c = 0;
        }
    }
}

void SongPreview::Start(Symbol s, TexMovie *t) {
    if (unk4c || s) {
        MILO_ASSERT(mFader && mMusicFader && mCrowdSingFader,0x6c);
        unk38.SetObjConcrete(t);
        if (s == unk74) {
            unk8d = true;
        } else {
            if (s) {
                if (!mSongMgr.HasSong(s, false)) {
                    return;
                }
                const SongMetadata *data =
                    mSongMgr.Data(mSongMgr.GetSongIDFromShortName(s, true));
                if (data && !data->IsVersionOK()) {
                    s = gNullStr;
                }
                if (unk8c) {
                    TheContentMgr->RegisterCallback(this, false);
                    unk8c = true;
                }
            }
            unk6c = true;
            mMusicFader->SetVolume(unk68);
            mCrowdSingFader->SetVolume(-96.0f);
            int x;
            if (unk70 < 2) {
                if (unk34) {
                    // do something
                }
                x = 0;
                unk34 = 0;
            } else if (unk70 == 2) {
                x = 3;
            } else {
                if (unk70 != 4)
                    return;
                else {
                    mFader->DoFade(-48.0f);
                }
            }
            unk70 = x;
        }
    }
}

void SongPreview::PreparePreview() {
    float previewstart = 0.0f;
    float previewend = 15000.0f;
    if (unk84 != 0.0f || unk88 != 0.0f) {
        previewend = unk84;
        previewstart = unk88;
    } else {
        int songid = mSongMgr.GetSongIDFromShortName(unk74, true);
        mSongMgr.Data(songid)->PreviewTimes(previewstart, previewend);
    }
    unk7c = previewstart;
    unk80 = previewend;
    PrepareSong(unk74);
}

void SongPreview::Poll() {}

DataNode SongPreview::OnStart(DataArray *arr) { return NULL_OBJ; }

void SongPreview::DetachFader(Fader *f) {
    if (unk34 && f) {
        for (int i = 0; i < unk5c; i++) {
            unk34->ChannelFaders(i)->Remove(f);
        }
    }
}

void SongPreview::PrepareFaders(SongInfo const *info) {
    for (int i = 0; i < unk5c; i++) {
        FaderGroup *f = unk34->ChannelFaders(i);
        f->Add(mMusicFader);
    }
}

void SongPreview::PrepareSong(Symbol s) {}

BEGIN_HANDLERS(SongPreview)
//     HANDLE(start, OnStart)
//     HANDLE_ACTION(start_video, action)
//     HANDLE_ACTION(set_music_vol, SetMusicVol(_msg->Float(2)))
//     HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS
