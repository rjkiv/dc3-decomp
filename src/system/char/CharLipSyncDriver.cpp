#include "char/CharLipSyncDriver.h"
#include "char/Char.h"
#include "char/CharFaceServo.h"
#include "char/CharLipSync.h"
#include "char/CharWeightable.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "obj/Utl.h"
#include "rndobj/Poll.h"
#include "rndobj/Rnd.h"

CharLipSyncDriver::CharLipSyncDriver()
    : mLipSync(this), mClips(this), mBlinkClip(this), mSongOwner(this), mSongOffset(0),
      mLoop(0), unk88(0), unk8c(0), unk90(1), unk94(0), mBones(this), mTestClip(this),
      mTestWeight(1), unkc4(0), unkc8(0), unkc9(0), unkcc(0), unkd4(0),
      mOverrideClip(this), mOverrideWeight(0), mOverrideOptions(this),
      mApplyOverrideAdditively(0), unk108(this), unk11c(0), unk120(0), unk124(0),
      unk128(0), mAlternateDriver(this) {}

CharLipSyncDriver::~CharLipSyncDriver() {
    RELEASE(unk88);
    RELEASE(unk94);
}

BEGIN_HANDLERS(CharLipSyncDriver)
    HANDLE_ACTION(resync, Sync())
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(CharLipSyncDriver)
    SYNC_PROP(bones, mBones)
    SYNC_PROP_SET(clips, mClips.Ptr(), SetClips(_val.Obj<ObjectDir>()))
    SYNC_PROP_SET(lipsync, mLipSync.Ptr(), SetLipSync(_val.Obj<CharLipSync>()))
    SYNC_PROP(song_owner, mSongOwner)
    SYNC_PROP(loop, mLoop)
    SYNC_PROP(song_offset, mSongOffset)
    SYNC_PROP(test_clip, mTestClip)
    SYNC_PROP(test_weight, mTestWeight)
    SYNC_PROP(override_clip, mOverrideClip)
    SYNC_PROP(override_weight, mOverrideWeight)
    SYNC_PROP(override_options, mOverrideOptions)
    SYNC_PROP(apply_override_additively, mApplyOverrideAdditively)
    SYNC_PROP(alternate_driver, mAlternateDriver)
    SYNC_SUPERCLASS(CharWeightable)
    SYNC_SUPERCLASS(CharPollable)
END_PROPSYNCS

BEGIN_SAVES(CharLipSyncDriver)
    SAVE_REVS(7, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(CharWeightable)
    bs << mBones;
    bs << mClips;
    bs << mLipSync;
    bs << mTestClip;
    bs << mTestWeight;
    bs << mOverrideClip;
    bs << mOverrideOptions;
    bs << mApplyOverrideAdditively;
    bs << mOverrideWeight;
    bs << mAlternateDriver;
END_SAVES

BEGIN_COPYS(CharLipSyncDriver)
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(CharWeightable)
    CREATE_COPY(CharLipSyncDriver)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mBones)
        COPY_MEMBER(mClips)
        COPY_MEMBER(mLipSync)
        COPY_MEMBER(mBlinkClip)
        COPY_MEMBER(mSongOffset)
        COPY_MEMBER(mLoop)
        COPY_MEMBER(mSongOwner)
        COPY_MEMBER(mTestClip)
        COPY_MEMBER(mTestWeight)
        COPY_MEMBER(mOverrideWeight)
        COPY_MEMBER(mOverrideClip)
        COPY_MEMBER(mOverrideOptions)
        COPY_MEMBER(mApplyOverrideAdditively)
        COPY_MEMBER(mAlternateDriver)
    END_COPYING_MEMBERS
END_COPYS

void CharLipSyncDriver::Enter() {
    RndPollable::Enter();
    mOverrideWeight = 0;
    if (mLipSync)
        Sync();
}

void CharLipSyncDriver::PollDeps(
    std::list<Hmx::Object *> &changedBy, std::list<Hmx::Object *> &change
) {
    change.push_back(mBones);
}

void CharLipSyncDriver::SetClips(ObjectDir *dir) {
    mClips = dir;
    Sync();
}

bool CharLipSyncDriver::SetLipSync(CharLipSync *sync) {
    if (unk8c) {
        MILO_LOG(
            "CharLipSyncDriver::SetLipSync() - previous VO Lipsync was fading out.  Deleting now - Name:%s\n",
            SafeName(mLipSync)
        );
        RELEASE(unk88);
        mLipSync = nullptr;
        unk8c = false;
        unk90 = 1;
    }

    if (sync) {
        if (!streq(sync->Name(), "player1_cam.lipsync")
            && !streq(sync->Name(), "player2_cam.lipsync")
            && !streq(sync->Name(), "dancer_face.lipsync")) {
            RELEASE(unk94);
            unk94 = new CharLipSync::PlayBack();
            unk94->Set(sync, mClips);
            unk94->Reset();
            return true;
        } else if (sync != mLipSync) {
            mLipSync = sync;
            mLoop = false;
            mSongOffset = 0;
            Sync();
            return true;
        }
    }
    return false;
}

void CharLipSyncDriver::ApplyBlinks() {
    CharFaceServo *servo = dynamic_cast<CharFaceServo *>(mBones.Ptr());
    if (servo) {
        servo->ApplyProceduralWeights();
    }
}

void CharLipSyncDriver::ResetOverrideBlend() {
    unk108 = nullptr;
    unk11c = 0;
}

void CharLipSyncDriver::BlendInOverrideClip(CharClip *clip, float f1, float f2) {
    unk108 = clip;
    unk11c = f1;
    unk120 = f2;
    unk128 = true;
}

void CharLipSyncDriver::Sync() {
    if (mClips) {
        mBlinkClip = mClips->Find<CharClip>("Blink", false);
    } else {
        mBlinkClip = nullptr;
    }
    RELEASE(unk88);
    if (unk94 && mClips) {
        unk94->SetClips(mClips);
    }
    if (mLipSync && mClips) {
        unk88 = new CharLipSync::PlayBack();
        unk88->Set(mLipSync, mClips);
        unk88->Reset();
        unk8c = false;
        unk90 = 1;
    }
}

void CharLipSyncDriver::ClearLipSync() {
    RELEASE(unk94);
    RELEASE(unk88);
    mLipSync = nullptr;
    unk8c = false;
    unk90 = 1;
}

void CharLipSyncDriver::BlendInOverrides(float f) {
    unkcc = f;
    unkc8 = true;
    unkc9 = false;
    unkd0 = true;
}

void CharLipSyncDriver::BlendOutOverrides(float f) {
    unkcc = f;
    unkc9 = true;
    unkc8 = false;
    unkd0 = true;
}

void CharLipSyncDriver::Highlight() {
    if (gCharHighlightY == -1.0f) {
        CharDeferHighlight(this);
    } else {
        Hmx::Color white(1, 1, 1);
        Vector2 v2(5.0f, gCharHighlightY);
        float y = TheRnd.DrawString(MakeString("%s:", PathName(this)), v2, white, true).y;
        v2.y += y;
        if (unk88) {
            TheRnd.DrawString(MakeString("frame %d", unk88->mFrame), v2, white, true);
            v2.y += y;
            std::vector<CharLipSync::PlayBack::Weight> &weights = unk88->mWeights;
            for (int i = 0; i < weights.size(); i++) {
                CharLipSync::PlayBack::Weight &curWeight = weights[i];
                float f14 = curWeight.unk14;
                CharClip *clip = curWeight.unk0;
                if (f14 != 0 && clip) {
                    TheRnd.DrawString(
                        MakeString("%s %.4f", clip->Name(), f14), v2, white, true
                    );
                    v2.y += y;
                }
            }
        }
        gCharHighlightY = v2.y + y;
    }
}

void CharLipSyncDriver::ScaleAddViseme(CharClip *clip, float f1) {
    float length;
    float dVar2;
    if (clip->LengthSeconds() != 0.0) {
        float temp = clip->LengthSeconds();
        length = TheTaskMgr.Seconds(TaskMgr::kRealTime);
        dVar2 = fmod(length, temp);
    }
    length = clip->FrameToBeat(clip->FramesPerSec() * dVar2);
    mBones.Ptr()->ScaleAdd(clip, 0.0, length, f1);
}