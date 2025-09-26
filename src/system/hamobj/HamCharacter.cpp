#include "hamobj/HamCharacter.h"
#include "HamCharacter.h"
#include "char/CharClip.h"
#include "char/CharEyes.h"
#include "char/CharFaceServo.h"
#include "char/CharLipSync.h"
#include "char/CharLipSyncDriver.h"
#include "char/CharServoBone.h"
#include "char/CharWeightable.h"
#include "char/Character.h"
#include "char/FileMerger.h"
#include "char/Waypoint.h"
#include "obj/Data.h"
#include "obj/DataUtl.h"
#include "obj/DirLoader.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "obj/Utl.h"
#include "os/Debug.h"
#include "os/File.h"
#include "os/System.h"
#include "rndobj/Anim.h"
#include "rndobj/Draw.h"
#include "rndobj/TexBlender.h"
#include "synth/Synth.h"
#include "utl/FilePath.h"
#include "utl/Loader.h"
#include "utl/Symbol.h"

namespace {
    const char *kCrewCardMeshName = "crew_card.mesh";
}

HamCharacter::HamCharacter()
    : mCampaignVOBank(0), mCampaignVODir(0), mFileMerger(0), unk2f8(0), mShowBox(0),
      unk2fa(1), mEyes(this), mGender(kHamFemale), unk314(0), mPollWhenHidden(0),
      mTexBlendersActive(1), mIKEffectors(this), unk330(0), mNeutralSkelDir(0),
      mSkeletonBones(0), mCrewCardMesh(nullptr), mUseCameraSkeleton(0) {
    mWaypoint = Hmx::Object::New<Waypoint>();
    mWaypoint->SetAngRadius(0);
    mWaypoint->SetRadius(36);
    mWaypoint->SetYRadius(36);
    mWaypoint->SetStrictRadiusDelta(0.01);
    const char *path = "";
    DataArray *cfg = SystemConfig("objects", "HamCharacter");
    if (cfg->FindData("skeleton_path", path, false) && *path != '\0') {
        FilePathTracker tracker(path);
        mNeutralSkelDir =
            DirLoader::LoadObjects("neutral_skeleton.milo", nullptr, nullptr);
        MILO_ASSERT(mNeutralSkelDir, 0x7D);
        mSkeletonBones =
            mNeutralSkelDir->Find<CharServoBone>("skeleton_bones.servo", true);
        MILO_ASSERT(mSkeletonBones, 0x7F);
    }
}

HamCharacter::~HamCharacter() {
    if (TheSynth) {
        TheSynth->RemovePlayHandler(this);
    }
    delete mWaypoint;
}

BEGIN_HANDLERS(HamCharacter)
    HANDLE(configure_file_merger, OnConfigureFileMerger)
    HANDLE_ACTION(start_load, StartLoad(_msg->Int(2)))
    HANDLE(cam_teleport, OnCamTeleport)
    HANDLE(post_delete, OnPostDelete)
    HANDLE_ACTION(set_lipsync_offset, SetLipsyncOffset(_msg->Float(2)))
    HANDLE(sound_play, OnSoundPlay)
    HANDLE_ACTION(
        enable_facial_animation,
        EnableFacialAnimation(_msg->Obj<CharLipSync>(2), _msg->Float(3))
    )
    HANDLE_ACTION(set_blinking, SetBlinking(_msg->Int(2)))
    HANDLE_EXPR(crew_card_found, Find<RndMesh>(kCrewCardMeshName, false))
    HANDLE_ACTION(set_campaign_vo, SetCampaignVo(_msg->Str(2)))
    HANDLE_EXPR(get_campaign_vo_bank, mCampaignVOBank)
    HANDLE(toggle_interests_overlay, OnToggleInterestDebugOverlay)
    HANDLE_SUPERCLASS(Character)
END_HANDLERS

BEGIN_PROPSYNCS(HamCharacter)
    SYNC_PROP(outfit, mOutfit)
    SYNC_PROP(outfit_dir, mOutfitDir)
    SYNC_PROP(show_box, mShowBox)
    SYNC_PROP(gender, (int &)mGender)
    SYNC_PROP_SET(force_blink, true, if (_val.Int()) ForceBlink())
    SYNC_PROP_SET(enable_auto_blinks, true, EnableBlinks(_val.Int(), false))
    SYNC_PROP_SET(
        force_lookat,
        mEyes->GetCurrentInterest() ? Symbol(mEyes->GetCurrentInterest()->Name())
                                    : Symbol(),
        SetFocusInterest(_val.Sym(), 0)
    )
    SYNC_PROP(poll_when_hidden, mPollWhenHidden)
    SYNC_PROP_MODIFY(
        tex_blenders_active, mTexBlendersActive, SetTexBlendersActive(mTexBlendersActive)
    )
    SYNC_PROP_SET(
        crew_card_showing,
        mCrewCardMesh ? mCrewCardMesh->Showing() : false,
        if (mCrewCardMesh) mCrewCardMesh->SetShowing(_val.Int())
    )
    SYNC_PROP_SET(prop_0_showing, GetPropShowing(0), SetPropShowing(0, _val.Int()))
    SYNC_PROP_SET(prop_1_showing, GetPropShowing(1), SetPropShowing(1, _val.Int()))
    SYNC_PROP_SET(prop_2_showing, GetPropShowing(2), SetPropShowing(2, _val.Int()))
    SYNC_PROP_SET(prop_3_showing, GetPropShowing(3), SetPropShowing(3, _val.Int()))
    SYNC_SUPERCLASS(Character)
END_PROPSYNCS

BEGIN_SAVES(HamCharacter)
    SAVE_REVS(3, 0)
    SAVE_SUPERCLASS(Character)
    bs << mOutfit;
    bs << mOutfitDir;
    bs << mShowBox;
    bs << mPollWhenHidden;
    bs << mTexBlendersActive;
END_SAVES

BEGIN_COPYS(HamCharacter)
    COPY_SUPERCLASS(Character)
    CREATE_COPY(HamCharacter)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mOutfit)
        COPY_MEMBER(mShowBox)
        COPY_MEMBER(mOutfitDir)
        COPY_MEMBER(mGender)
        COPY_MEMBER(mPollWhenHidden)
        COPY_MEMBER(mTexBlendersActive)
    END_COPYING_MEMBERS
END_COPYS

void HamCharacter::AddedObject(Hmx::Object *obj) {
    Character::AddedObject(obj);
    static Symbol HamIKEffector("HamIKEffector");
    Symbol className = obj->ClassName();
    if (streq(obj->Name(), "char.fm")) {
        mFileMerger = dynamic_cast<FileMerger *>(obj);
    } else if (streq(obj->Name(), "CharEyes.eyes")) {
        mEyes = dynamic_cast<CharEyes *>(obj);
    } else if (className == HamIKEffector) {
        mIKEffectors.push_back(dynamic_cast<CharWeightable *>(obj));
    }
}

void HamCharacter::RemovingObject(Hmx::Object *obj) {
    Character::RemovingObject(obj);
    if (obj == mFileMerger) {
        mFileMerger = nullptr;
    }
}

void HamCharacter::Exit() {
    if (TheSynth) {
        TheSynth->RemovePlayHandler(this);
    }
    Character::Exit();
}

void HamCharacter::Draw() {
    if (!mShowing && !mTexBlendersActive) {
        for (ObjDirItr<RndTexBlender> it(this, true); it != nullptr; ++it) {
            it->DrawShowing();
        }
    }
    RndDrawable::Draw();
}

void HamCharacter::Init() {
    REGISTER_OBJ_FACTORY(HamCharacter);
    TheDebug.AddExitCallback(HamCharacter::Terminate);
    const char *path = "";
    DataArray *cfg = SystemConfig("objects", "HamCharacter");
    static Symbol CHARCLIP_SKELETONS("CHARCLIP_SKELETONS");
    DataArray *macro = DataGetMacro(CHARCLIP_SKELETONS);
    if (macro) {
        if (cfg->FindData("skeleton_path", path, false) && *path != '\0') {
            FilePathTracker tracker(path);
            int numSkels = macro->Size();
            MILO_ASSERT(numSkels == kNumSkeletons, 0x42);
            ObjectDir *clips =
                DirLoader::LoadObjects("skeleton_clips.milo", nullptr, nullptr);
            MILO_ASSERT(clips, 0x45);
            for (int i = 0; i < numSkels; i++) {
                Symbol s = macro->Sym(i);
                sSkeletonClips[i] =
                    clips->Find<CharClip>(MakeString("%s_skeleton", s), true);
                MILO_ASSERT(sSkeletonClips[i], 0x4C);
            }
        }
    }
}

void HamCharacter::Terminate() {
    for (int i = 0; i < kNumSkeletons; i++) {
        delete sSkeletonClips[i];
    }
}

String HamCharacter::GetCampaignVo() { return mCampaignVO; }

void HamCharacter::StartLoad(bool start) {
    if (!mFileMerger->StartLoad(start)) {
        SyncObjects();
    }
}

void HamCharacter::SetOutfit(Symbol outfit) { mOutfit = outfit; }
void HamCharacter::SetOutfitDir(Symbol outfitDir) { mOutfitDir = outfitDir; }

void HamCharacter::UnloadAll() {
    if (mFileMerger)
        mFileMerger->Clear();
}

String HamCharacter::GetCampaignVoMilo() {
    return MakeString("sfx/loc/eng/campaign/%s.milo", mCampaignVO);
}

void HamCharacter::SetTexBlendersActive(bool active) {
    mTexBlendersActive = active;
    for (ObjDirItr<RndTexBlender> it(this, true); it != nullptr; ++it) {
        it->SetShowing(active);
    }
}

bool HamCharacter::InClipTest() {
    if (TheLoadMgr.EditMode() && streq(Dir()->Name(), "clip_test")) {
        return true;
    } else
        return false;
}

void HamCharacter::SetIKEffectorWeights(float weight) {
    for (ObjPtrList<CharWeightable>::iterator it = mIKEffectors.begin();
         it != mIKEffectors.end();
         ++it) {
        if (*it) {
            (*it)->SetWeight(weight);
        }
    }
}

void HamCharacter::ResyncLipSync(CharLipSync *sync) {
    CharLipSyncDriver *driver = Find<CharLipSyncDriver>("face.lipdrv", false);
    if (driver) {
        if (sync) {
            driver->SetLipSync(sync);
        }
        driver->Sync();
    }
}

void HamCharacter::PlayBaseViseme() {
    ObjectDir *visemeDir = Find<ObjectDir>("viseme", false);
    if (visemeDir) {
        CharFaceServo *servo = Find<CharFaceServo>("face.faceservo", false);
        if (servo) {
            servo->SetClips(visemeDir);
            CharClip *clip = servo->BaseClip();
            if (clip) {
                clip->PoseMeshes(this, clip->StartBeat());
            }
        }
    }
}

void HamCharacter::EnableFacialAnimation(CharLipSync *sync, float f2) {
    MILO_LOG(
        "HamCharacter::EnableFacialAnimation Name:%s lipsync name:%s\n",
        Name(),
        SafeName(sync)
    );
    ObjectDir *visemeDir = Find<ObjectDir>("viseme", false);
    if (visemeDir && !visemeDir->Find<CharClip>("Base", false)) {
        return;
    }
    unk330 = f2;
    CharLipSyncDriver *driver = Find<CharLipSyncDriver>("face.lipdrv", false);
    if (sync && driver) {
        if (!driver->SetLipSync(sync)) {
            driver->Sync();
        }
        driver->SetSongOffset(f2);
        String str(FileGetBase(sync->Name()));
        str += ".anim";
        RndAnimatable *anim = sync->Dir()->Find<RndAnimatable>(str.c_str(), false);
        if (anim) {
            static Symbol animate("animate");
            anim->Handle(Message(animate), true);
        }
    }
}

void HamCharacter::DisableFacialAnimation() {
    CharLipSyncDriver *driver = Find<CharLipSyncDriver>("face.lipdrv", false);
    if (driver) {
        driver->SetLipSync(nullptr);
        driver->Sync();
    }
}

void HamCharacter::ResetFacialAnimation() {
    MILO_LOG("HamCharacter::ResetFacialAnimation() Name:%s\n", Name());
    CharLipSyncDriver *driver = Find<CharLipSyncDriver>("face.lipdrv", false);
    if (driver) {
        driver->ClearLipSync();
    }
}

void HamCharacter::SetBlinking(bool blinking) {
    CharLipSyncDriver *driver = Find<CharLipSyncDriver>("face.lipdrv", false);
    ObjectDir *visemeDir = Find<ObjectDir>("viseme", false);
    if (visemeDir) {
        CharFaceServo *servo = Find<CharFaceServo>("face.faceservo", false);
        if (servo) {
            servo->SetBlinkClipLeft(blinking ? "Blink" : "");
            servo->SetBlinkClipRight(blinking ? "Blink" : "");
            servo->SetClips(visemeDir);
        }
        if (driver && visemeDir->Find<CharClip>("Base", false)) {
            driver->SetClips(visemeDir);
        }
    }
}

void HamCharacter::BlendInFaceOverrides(float f1) {
    CharLipSyncDriver *driver = Find<CharLipSyncDriver>("face.lipdrv", false);
    if (driver) {
        driver->BlendInOverrides(f1);
    }
}

void HamCharacter::BlendOutFaceOverrides(float f1) {
    CharLipSyncDriver *driver = Find<CharLipSyncDriver>("face.lipdrv", false);
    if (driver) {
        driver->BlendOutOverrides(f1);
    }
}

void HamCharacter::SetLipsyncOffset(float offset) {
    CharLipSyncDriver *driver = Find<CharLipSyncDriver>("face.lipdrv", false);
    if (driver) {
        driver->SetSongOffset(unk330 + offset);
    }
}

void HamCharacter::SetFaceOverrideWeight(float weight) {
    CharLipSyncDriver *driver = Find<CharLipSyncDriver>("face.lipdrv", false);
    if (driver) {
        driver->SetOverrideWeight(weight);
    }
}

float HamCharacter::GetFaceOverrideWeight() {
    CharLipSyncDriver *driver = Find<CharLipSyncDriver>("face.lipdrv", false);
    return driver ? driver->GetOverrideWeight() : 0;
}

void HamCharacter::SetUseCameraSkeleton(bool use) {
    mUseCameraSkeleton = use;
    if (mUseCameraSkeleton) {
        SetIKEffectorWeights(0);
    } else {
        SetIKEffectorWeights(1);
    }
}

Symbol HamCharacter::GetFaceOverrideClip() {
    CharLipSyncDriver *driver = Find<CharLipSyncDriver>("face.lipdrv", false);
    if (driver && driver->OverrideClip()) {
        return driver->OverrideClip()->Name();
    } else
        return Symbol();
}

void HamCharacter::ResetFaceOverrideBlending() {
    CharLipSyncDriver *driver = Find<CharLipSyncDriver>("face.lipdrv", false);
    if (driver) {
        driver->ResetOverrideBlend();
    }
}

void HamCharacter::SetCampaignVo(const char *cc) {
    mCampaignVO = cc;
    RELEASE(mCampaignVOBank);
    if (!mCampaignVO.empty()) {
        String milo = GetCampaignVoMilo();
        mCampaignVODir = DirLoader::LoadObjects(milo.c_str(), nullptr, nullptr);
        for (ObjDirItr<Hmx::Object> it(mCampaignVODir, false); it != nullptr; ++it) {
            if (it->Type() == "character_vo") {
                mCampaignVOBank = it;
                return;
            }
        }
    }
}
