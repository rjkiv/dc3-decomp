#include "char/CharLipSyncDriver.h"
#include "char/Char.h"
#include "char/CharFaceServo.h"
#include "char/CharLipSync.h"
#include "char/CharWeightable.h"
#include "math/Utl.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "obj/Utl.h"
#include "os/Debug.h"
#include "os/Timer.h"
#include "rndobj/Poll.h"
#include "rndobj/Rnd.h"
#include "utl/Loader.h"
#include "world/CameraManager.h"
#include "world/CameraShot.h"
#include "world/Dir.h"

CharLipSyncDriver::CharLipSyncDriver()
    : mLipSync(this), mClips(this), mBlinkClip(this), mSongOwner(this), mSongOffset(0),
      mLoop(0), unk88(0), unk8c(0), unk90(1), unk94(0), mBones(this), mTestClip(this),
      mTestWeight(1), mOverallOverrideWeight(0), unkc8(0), unkc9(0), unkcc(0), unkd4(0),
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

INIT_REVS(7, 0)

BEGIN_LOADS(CharLipSyncDriver) // register error
    LOAD_REVS(bs)
    ASSERT_REVS(7, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    LOAD_SUPERCLASS(CharWeightable)
    d >> mBones;
    d >> mClips;
    if (d.rev < 1) {
        FilePath fp;
        d >> fp;
        MILO_NOTIFY("%s old version, won't load %s", PathName(this), fp);
        String str;
        d >> str;
    } else
        d >> mLipSync;
    if (d.rev > 1) {
        mTestClip.Load(d.stream, true, mClips);
        d >> mTestWeight;
    }
    if (d.rev > 2) {
        mOverrideClip.Load(d.stream, true, mClips);
        if (d.rev < 5) {
            int x;
            d >> x;
        }
        d >> mOverrideOptions;
    }
    if (d.rev > 3)
        d >> mApplyOverrideAdditively;
    if (d.rev > 5)
        d >> mOverrideWeight;
    if (d.rev > 6)
        d >> mAlternateDriver;
    Sync();
END_LOADS

void CharLipSyncDriver::Poll() {
    START_AUTO_TIMER("lipsyncdriver");
    if (mClips && mBones) {
        if (mTestClip && TheLoadMgr.EditMode()) {
            if (mTestClip->Relative() && mTestWeight >= 0) {
                mBones->ScaleAdd(
                    mTestClip->Relative(), mTestWeight, mTestClip->StartBeat(), 0
                );
            }
            return;
        } else {
            float f15 = TheTaskMgr.Seconds(TaskMgr::kTaskTRVideo) * 1000;
            if (unkd0) {
                unkd4 = f15;
                unkd0 = false;
            }
            if (unkc8) {
                if (f15 > unkcc + unkd4) {
                    mOverallOverrideWeight = 1;
                    unkc8 = false;
                } else {
                    float f11 = Clamp(0.0f, 1.0f, (f15 - unkd4) / unkcc);
                    if (f11 > mOverallOverrideWeight) {
                        mOverallOverrideWeight = f11;
                    }
                }
            } else if (unkc9) {
                if (f15 > unkcc + unkd4) {
                    mOverallOverrideWeight = 1;
                    unkc9 = false;
                } else {
                    float f11 = 1.0f - Clamp(0.0f, 1.0f, (f15 - unkd4) / unkcc);
                    if (f11 < mOverallOverrideWeight) {
                        mOverallOverrideWeight = f11;
                    }
                }
            }
            if (unk11c > 0) {
                if (unk128) {
                    unk124 = f15;
                    unk128 = false;
                }
                if (f15 > unk120 + unk124) {
                    mOverrideClip = unk108;
                    mOverrideWeight = unk11c;
                    unk11c = 0;
                } else if (unk88) {
                    float pct = Clamp(0.0f, 1.0f, (f15 - unk124) / unk120);
                    if (mOverrideClip) {
                        if (mOverrideWeight > 0) {
                            float f11 =
                                (1.0f - pct) * mOverrideWeight * mOverallOverrideWeight;
                            if (f11 < 0) {
                                f11 = 0;
                                MILO_ASSERT_FMT(
                                    mOverallOverrideWeight >= 0,
                                    "mOverallOverrideWeight = %f",
                                    mOverallOverrideWeight
                                );
                                MILO_ASSERT_FMT(
                                    mOverrideWeight >= f11,
                                    "mOverrideWeight = %f",
                                    mOverrideWeight
                                );
                                MILO_ASSERT_FMT(pct <= 1, "pct = %f", pct);
                            }
                            ScaleAddViseme(mOverrideClip, f11);
                        }
                    }
                    if (unk108 && mOverrideWeight > 0) {
                        float f11 = mOverrideWeight * mOverallOverrideWeight;
                        MILO_ASSERT_FMT(
                            mOverallOverrideWeight >= 0,
                            "mOverallOverrideWeight = %f",
                            mOverallOverrideWeight
                        );
                        MILO_ASSERT_FMT(
                            mOverrideWeight >= f11,
                            "mOverrideWeight = %f",
                            mOverrideWeight
                        );
                        MILO_ASSERT_FMT(pct >= f11, "pct = %f", pct);
                        ScaleAddViseme(unk108, f11);
                    }
                    if (mOverallOverrideWeight >= 1) {
                        ApplyBlinks();
                        return;
                    }
                }
            }
            if (mOverallOverrideWeight <= 0) {
                if (mOverrideClip && mOverrideWeight * mOverallOverrideWeight > 0) {
                    ScaleAddViseme(
                        mOverrideClip, mOverrideWeight * mOverallOverrideWeight
                    );
                    if (mOverallOverrideWeight >= 1) {
                        ApplyBlinks();
                        return;
                    }
                }
            }

            float f11 = 1.0f - mOverallOverrideWeight;
            if (f11 <= 0) {
                return;
            } else {
                UpdatePlayback(unk88, unk90 * f11, mSongOffset);
                if (!unk8c && unk88) {
                    if (unk88->mLipSync
                        && TheTaskMgr.Seconds(TaskMgr::kTaskTRVideo) + mSongOffset
                            >= unk88->mLipSync->Duration()) {
                        MILO_LOG(
                            "CharLipSyncDriver::Poll() - Triggering VO Lip Sync FadeOut - Name:%s\n",
                            unk88->mLipSync ? unk88->mLipSync->Name() : ""
                        );
                        unk8c = true;
                    }
                }
                if (unk8c) {
                    if (unk90 < 0.001f) {
                        MILO_LOG(
                            "CharLipSyncDriver::Poll() - Deleting VO Lip Sync track because it finished and faded out - Name:%s\n",
                            unk88->mLipSync ? unk88->mLipSync->Name() : ""
                        );
                        RELEASE(unk88);
                        mLipSync = nullptr;
                        unk90 = 1;
                        unk8c = false;
                    } else {
                        unk90 *= 0.99f;
                    }
                }
                CamShot *shot = nullptr;
                if (TheWorld) {
                    CameraManager *mgr = TheWorld->GetCameraManager();
                    if (mgr) {
                        shot = mgr->CurrentShot();
                        if (!mgr->CurrentShot()) {
                            shot = mgr->MiloCamera();
                        }
                    }
                }
                bool updated = unk88 && unk88->mLipSync && shot && shot->Name()
                    && strneq(shot->Name(), "battle_", 7);
                if (!updated) {
                    UpdatePlayback(unk94, f11, 0);
                }
                if (mSongOwner && mSongOwner->unk88) {
                    float f15 = TheTaskMgr.Seconds(TaskMgr::kTaskTRVideo)
                        + mSongOwner->mSongOffset;
                    if (unk8c) {
                        f15 = Mod(f15, mSongOwner->unk88->mLipSync->Duration() - 0.001f);
                    }
                    mSongOwner->unk88->Poll(f15);
                    auto &weights = mSongOwner->unk88->mWeights;
                    for (int i = 0; i < weights.size(); i++) {
                        CharLipSync::PlayBack::Weight &wt = weights[i];
                        float cur = wt.current;
                        CharClip *clip = wt.clip;
                        if (cur != 0 && clip && clip != mSongOwner->mBlinkClip) {
                            ScaleAddViseme(mClips->Find<CharClip>(clip->Name()), cur);
                        }
                    }
                }
                ApplyBlinks();
            }
        }
    }
}

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
            mLipSync ? mLipSync->Name() : "NULL"
        );
        RELEASE(unk88);
        mLipSync = nullptr;
        unk8c = false;
        unk90 = 1;
    }
    if (sync
        && (streq(sync->Name(), "player1_cam.lipsync")
            || streq(sync->Name(), "player2_cam.lipsync")
            || streq(sync->Name(), "dancer_face.lipsync"))) {
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
    } else {
        return false;
    }
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
                float f14 = curWeight.last;
                CharClip *clip = curWeight.clip;
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
    if (clip->LengthSeconds() != 0) {
        dVar2 = fmodf(TheTaskMgr.Seconds(TaskMgr::kTaskTRVideo), clip->LengthSeconds());
    } else {
        dVar2 = 0;
    }
    length = clip->FrameToBeat(clip->FramesPerSec() * dVar2);
    mBones->ScaleAdd(clip, f1, length, 0);
}

void CharLipSyncDriver::UpdatePlayback(CharLipSync::PlayBack *pb, float f2, float f3) {
    if (pb) {
        float f10 = TheTaskMgr.Seconds(TaskMgr::kTaskTRVideo) + f3;
        if (mLoop) {
            f10 = Mod(f10, pb->mLipSync->Duration() - 0.001f);
        }
        if (mAlternateDriver) {
            f10 = mAlternateDriver->TopClipFrame();
        }
        pb->Poll(f10);
        for (int i = 0; i < pb->mWeights.size(); i++) {
            CharLipSync::PlayBack::Weight &wt = pb->mWeights[i];
            float weight = wt.current;
            if (weight != 0) {
                CharClip *clip = wt.clip;
                if (clip != mBlinkClip) {
                    if (mSongOwner) {
                        weight = 0;
                    } else {
                        weight *= f2;
                    }
                }
                if (clip && weight != 0) {
                    MILO_ASSERT_FMT(weight >= 0, "weight = %f", weight);
                    if (weight < 0) {
                        weight = 0;
                    }
                    ScaleAddViseme(clip, weight);
                }
            }
        }
    }
}
