#include "meta_ham/SkeletonIdentifier.h"
#include "SkeletonIdentifier.h"
#include "flow/PropertyEventProvider.h"
#include "gesture/GestureMgr.h"
#include "gesture/Skeleton.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamPlayerData.h"
#include "meta_ham/PassiveMessenger.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/PlatformMgr.h"
#include "utl/Locale.h"
#include "utl/MakeString.h"
#include "utl/Symbol.h"

SkeletonIdentifier::EnrolledPlayer::EnrolledPlayer() : unk0(-1) {}

SkeletonIdentifier::SkeletonIdentifier()
    : mIdentityStatus(kIdentityStatus_0), unk30(-1), unk34(-1), unk38(-1), unk3c(-1),
      unk40(-1), unk44(-1), unkc8(false) {
    TheSkeletonIdentifier = this;
}

SkeletonIdentifier::~SkeletonIdentifier() { TheSkeletonIdentifier = nullptr; }

void SkeletonIdentifier::Init() {
    SetName("skeleton_identifier", ObjectDir::Main());
    TheGestureMgr->AddSink(this, "skeleton_enrollment_changed");
    ThePlatformMgr.AddSink(this, "signin_changed");
    UpdateEnrolledPlayers();
}

void SkeletonIdentifier::SetEnrolling() {
    TheGestureMgr->AddSink(this, "skeleton_identified");
}

void SkeletonIdentifier::CorrectIdentity(int i) {
    Skeleton *skel = TheGestureMgr->GetActiveSkeleton();
    if (skel) {
        unk3c = i;
        unk40 = skel->TrackingID();
    }
}

void SkeletonIdentifier::RequestIdentity() {
    if (mIdentityStatus != 0) {
        MILO_NOTIFY("Already searching for identity");
    } else {
        MILO_ASSERT(!GestureMgr::sIdentityOpInProgress, 0x77);
        mIdentityStatus = kIdentityStatus_Identifying;
    }
}

void SkeletonIdentifier::SearchForIdentity() {
    if (TheGestureMgr->IDEnabled()) {
        MILO_ASSERT(mIdentityStatus == kIdentityStatus_Identifying, 0x81);
        if (!GestureMgr::sIdentityOpInProgress) {
            Skeleton &skel = TheGestureMgr->GetSkeleton(unk30);
            if (skel.IsTracked()) {
                TheGestureMgr->AddSink(this, "skeleton_identified");
                skel.RequestIdentity();

            } else {
                mIdentityStatus = kIdentityStatus_0;
            }
        }
    }
}

IdentityStatus SkeletonIdentifier::GetIdentityStatus(int i) {
    if (mIdentityStatus != kIdentityStatus_0) {
        Skeleton &skel = TheGestureMgr->GetSkeleton(unk30);
        if (!skel.IsTracked()) {
            mIdentityStatus = kIdentityStatus_0;
        }
    }
    if (unk34 == i) {
        return mIdentityStatus;
    } else
        return kIdentityStatus_0;
}

void SkeletonIdentifier::NotifyOfRecognition(int i) const {
    bool check = true;
    Skeleton *skel = TheGestureMgr->GetSkeletonByEnrollmentIndex(i);
    if (skel) {
        if (!IsAssociatedWithProfile(i)) {
            check = !skel->ProfileMatched() == 0;
        }
        if (check) {
            int playerID = TheGameData->GetPlayerFromSkeleton(*skel);
            static Symbol p1("p1");
            static Symbol p2("p2");
            static Symbol identity_recognized("identity_recognized");
            static Symbol identification("identification");
            if (0 <= playerID && IsAssociatedWithProfile(i)) {
                String playerName = TheGameData->GetPlayerName(playerID);
                Symbol playerSym = p2;
                if (playerID == 0)
                    playerSym = p1;
                Localize(identity_recognized, 0, TheLocale);
                ThePassiveMessenger->TriggerStringMsg(
                    playerName, playerSym, (PassiveMessageType)0, identification, -1
                );
            }
        }
    }
}

void SkeletonIdentifier::UpdateIdentityStatus() {
    static Symbol identifying("identifying");
    for (int i = 0; i < 2; i++) {
        HamPlayerData *pPlayer = TheGameData->Player(i);
        MILO_ASSERT(pPlayer, 0x146);
        PropertyEventProvider *pProvider = pPlayer->Provider();
        MILO_ASSERT(pProvider, 0x149);
        pProvider->SetProperty(identifying, GetIdentityStatus(i));
    }
}

BEGIN_HANDLERS(SkeletonIdentifier)
    HANDLE_EXPR(toggle_draw_debug, unkc8 = !unkc8)
    HANDLE_ACTION(correct_identity, CorrectIdentity(_msg->Int(2)))
    HANDLE_ACTION(set_up_initial_profiles, SetUpInitialProfiles())
    HANDLE_MESSAGE(SkeletonIdentifiedMsg)
    HANDLE_MESSAGE(SkeletonEnrollmentChangedMsg)
    HANDLE_MESSAGE(SigninChangedMsg)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

String EnrollmentIndexString(int idx) {
    String str = MakeString("enrollment index %d", idx);
    if (idx == -1) {
        str = MakeString("%s %s", str, " (NUI_IDENTITY_ENROLLMENT_INDEX_CALL_IDENTIFY)");
    }
    if (idx == -2) {
        str = MakeString("%s %s", str, " (NUI_IDENTITY_ENROLLMENT_INDEX_UNKNOWN)");
    }
    if (idx == -4) {
        str = MakeString("%s %s", str, " (NUI_IDENTITY_ENROLLMENT_INDEX_BUSY)");
    }
    if (idx == -5) {
        str = MakeString("%s %s", str, " (NUI_IDENTITY_ENROLLMENT_INDEX_FAILURE)");
    }
    return str;
}
