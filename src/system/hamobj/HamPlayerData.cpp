#include "hamobj/HamPlayerData.h"
#include "gesture/BaseSkeleton.h"
#include "hamobj/HamDirector.h"
#include "hamobj/HamGameData.h"
#include "gesture/GestureMgr.h"
#include "gesture/Skeleton.h"
#include "hamobj/Difficulty.h"
#include "obj/Data.h"
#include "obj/DataFile.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "os/Debug.h"
#include "utl/Loader.h"
#include "utl/Symbol.h"

HamPlayerData::HamPlayerData(int i)
    : mPlayerIdx(i), mCharacter(gNullStr), mPreferredCharacter(gNullStr),
      mPreferredOutfit(gNullStr), mOutfit(gNullStr), mDifficulty(DefaultDifficulty()),
      mTrackingIDTimestamp(-1), mSkeletonTrackingID(-1), mProvider(this), mPadNum(-1),
      mSameRatingCount(0), mLastRatingIdx(0x19) {
    DataArray *dancerDta = DataReadFile("devkit:\\dancers.dta", false);
    if (dancerDta) {
        for (int i = 0; i < dancerDta->Size(); i++) {
            mTestDancers.push_back(dancerDta->Str(i));
        }
        dancerDta->Release();
    }
    if (!mTestDancers.empty()) {
        mCurTestDancer = mTestDancers.front();
    }
}

BEGIN_HANDLERS(HamPlayerData)
    HANDLE_ACTION(set_character_outfit, SetCharacterOutfit(_msg->Sym(2)))
    HANDLE_ACTION(set_dancer, SetCharacter(_msg->Sym(2)))
    HANDLE_EXPR(get_dancer, mCharacter)
    HANDLE_EXPR(get_same_rating_count, mSameRatingCount)
    HANDLE_ACTION(increment_same_rating_count, mSameRatingCount++)
    HANDLE_ACTION(clear_same_rating_count, mSameRatingCount = 0)
    HANDLE_EXPR(get_last_rating_idx, mLastRatingIdx)
    HANDLE_ACTION(set_last_rating_idx, mLastRatingIdx = _msg->Int(2))
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(HamPlayerData)
    SYNC_PROP(character, mCharacter)
    SYNC_PROP(outfit, mOutfit)
    SYNC_PROP(pad_num, mPadNum)
    SYNC_PROP_SET(crew, mCrew, SetCrew(_val.Sym()))
    SYNC_PROP_SET(difficulty, mDifficulty, SetDifficulty((Difficulty)_val.Int()))
    SYNC_PROP_SET(playing, IsPlaying(), SetPlaying(_val.Int()))
    SYNC_PROP(autoplay, mAutoplayRating)
    SYNC_PROP(provider, mProvider)
    SYNC_PROP(skeleton_tracking_id, mSkeletonTrackingID)
    if (mProvider && mProvider->SyncProperty(_val, _prop, _i, _op))
        return true;
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

bool HamPlayerData::SetAssociatedPadNum(int padnum, String name) {
    MILO_ASSERT(mProvider, 0xB7);
    mPadNum = padnum;
    static Symbol player_name("player_name");
    String playerStr = mProvider->Property(player_name, true)->Str();
    mProvider->SetProperty(player_name, name);
    bool update = playerStr != name;
    if (update) {
        static Message padnumUpdatedMsg("padnum_updated");
        mProvider->Export(padnumUpdatedMsg, true);
    }
    return update;
}

const Skeleton *HamPlayerData::GetSkeleton() const {
    int idx = TheGestureMgr->GetSkeletonIndexByTrackingID(mSkeletonTrackingID);
    if (idx >= 0 && idx < 6) {
        return &TheGestureMgr->GetSkeleton(idx);
    } else
        return nullptr;
}

const Skeleton *HamPlayerData::GetSkeleton(const Skeleton *const (&skeletons)[6]) const {
    for (int i = 0; i < 6; i++) {
        if (skeletons[i]->TrackingID() == mSkeletonTrackingID) {
            return skeletons[i];
        }
    }
    return nullptr;
}

void HamPlayerData::SetCharacterOutfit(Symbol outfit) {
    if (!outfit.Null()) {
        Symbol charOutfit = GetOutfitCharacter(outfit, true);
        mOutfit = outfit;
        mCharacter = charOutfit;
        mCrew = GetCrewForCharacter(charOutfit, true);
    } else {
        mCharacter = gNullStr;
        mOutfit = gNullStr;
        mCrew = gNullStr;
    }
}

void HamPlayerData::SetPreferredOutfit(Symbol outfit) {
    mPreferredOutfit = outfit.Null() ? outfit : GetOutfitRemap(outfit, true);
}

void HamPlayerData::SetOutfit(Symbol outfit) {
    mOutfit = outfit.Null() ? outfit : GetOutfitRemap(outfit, true);
}

Symbol HamPlayerData::CharacterOutfit(Symbol crew) const {
    Symbol outfit = mOutfit;
    if (outfit.Null()) {
        MILO_ASSERT(!crew.Null(), 0x4D);
        Symbol crewChar = GetCrewCharacter(crew, mPlayerIdx);
        outfit = GetCharacterOutfit(crewChar, 0, true);
    }
    return outfit;
}

String HamPlayerData::GetPlayerName() const {
    static Symbol player_name("player_name");
    return mProvider->Property(player_name, true)->Str();
}

SkeletonSide HamPlayerData::Side() const {
    MILO_ASSERT(mProvider, 0xB0);
    static Symbol side("side");
    return (SkeletonSide)mProvider->Property(side, true)->Int();
}

void HamPlayerData::SetPlaying(bool) { MILO_ASSERT(false, 0xE1); }

void HamPlayerData::SetCharacter(Symbol character) {
    mCharacter = character;
    if (mProvider) {
        static Symbol dancer("dancer");
        mProvider->SetProperty(dancer, mCharacter);
    }
}

void HamPlayerData::SetSkeletonTrackingID(int id) {
    int oldID = mSkeletonTrackingID;
    mSkeletonTrackingID = id;
    if (mSkeletonTrackingID <= 0) {
        mTrackingIDTimestamp = -1;
        static Symbol identifying("identifying");
        mProvider->SetProperty(identifying, 0);
    } else if (oldID <= 0) {
        mTrackingIDTimestamp = TheTaskMgr.UISeconds();
    }
}

void HamPlayerData::SetUsingFitness(bool fitness) {
    static Symbol using_fitness("using_fitness");
    mProvider->SetProperty(using_fitness, fitness);
}

void HamPlayerData::SetCrew(Symbol crewSym) {
    mCrew = crewSym;
    static Symbol crew("crew");
    mProvider->SetProperty(crew, mCrew);
}

void HamPlayerData::SetDifficulty(Difficulty d) {
    mDifficulty = d;
    static Symbol difficulty("difficulty");
    Symbol diffSym = DifficultyToSym(d);
    mProvider->SetProperty(difficulty, diffSym);
    if (TheHamDirector) {
        TheHamDirector->HandleDifficultyChange();
    }
}

float HamPlayerData::TrackingAgeSeconds() const {
    if (mTrackingIDTimestamp <= 0)
        return -1;
    else
        return TheTaskMgr.UISeconds() - mTrackingIDTimestamp;
}

bool HamPlayerData::IsPlaying() const {
    if (!TheLoadMgr.EditMode() && mAutoplayRating.Null()
        && TheGestureMgr->PauseOnSkeletonLoss()
            != GestureMgr::kSkelLoss_NoPauseFlashcardsDontDropOut) {
        return mSkeletonTrackingID > 0;
    } else {
        return true;
    }
}

Symbol HamPlayerData::GetPreferredOutfit() const { return mPreferredOutfit; }
