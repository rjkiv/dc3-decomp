#include "meta_ham/Overshell.h"
#include "Overshell.h"
#include "flow/PropertyEventProvider.h"
#include "gesture/GestureMgr.h"
#include "gesture/Skeleton.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamPlayerData.h"
#include "meta_ham/HamUI.h"
#include "obj/Dir.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "utl/Symbol.h"

OvershellSlot::OvershellSlot(HamPlayerData &data)
    : mPlayerData(data), mState((OvershellSlotState)0), unk38(0), unk3c(-1) {}

void OvershellSlot::SetPlaying(bool playing) {
    static Message quitMsg("player_quit", 0);
    static Message joinMsg("player_join", 0);
    if (playing) {
        joinMsg[0] = mPlayerNum;
        TheHamProvider->Export(joinMsg, true);
    } else {
        quitMsg[0] = mPlayerNum;
        TheHamProvider->Export(quitMsg, true);
    }
}

void OvershellSlot::SetState(OvershellSlotState state) {
    bool state3 = mState == 3;
    if (!state3 || TheGestureMgr->Unk425C() != 1) {
        PropertyEventProvider *prov = mPlayerData.Provider();
        if (prov) {
            Hmx::Object *provObj = prov;
            if (provObj) {
                static Symbol join_state("join_state");
                provObj->SetProperty(join_state, state);
            }
        }
        if (state3 != mPlayerData.IsPlaying()) {
            SetPlaying(mPlayerData.IsPlaying());
        }
        mState = state;
    }
}

void OvershellSlot::Poll(const Skeleton *const (&skeletons)[6]) {
    int trackingID = mPlayerData.GetSkeletonTrackingID();
    Skeleton *skel = TheGestureMgr->GetSkeletonByTrackingID(trackingID);
    if (mState == 3 && trackingID > 0 && !skel) {
        return;
    } else if (mState == 0 || (skel && skel->IsValid())) {
        if (mState == 0) {
            if (mPlayerData.Autoplay().Null()) {
                SkeletonChooser *chooser = TheHamUI.GetShellInput()->mSkelChooser;
                MILO_ASSERT(chooser, 0x99);
                HamPlayerData *playerData = TheGameData->Player(mPlayerNum);
                if (playerData->GetSkeletonTrackingID() <= 0)
                    return;
            }
            SetState((OvershellSlotState)3);
        }
    } else if (mPlayerData.Autoplay().Null()) {
        SetState((OvershellSlotState)0);
    }
}

Overshell::Overshell() {
    for (int i = 0; i < 2; i++) {
        mSlots[i] = new OvershellSlot(*TheGameData->Player(i));
    }
}

Overshell::~Overshell() {
    for (int i = 0; i < 2; i++) {
        delete mSlots[i];
    }
}

BEGIN_HANDLERS(Overshell)
    HANDLE_ACTION(
        set_state, mSlots[_msg->Int(2)]->SetState((OvershellSlotState)_msg->Int(3))
    )
    HANDLE_ACTION(resolve_skeletons, ResolveSkeletons())
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

void Overshell::Init() {
    SetName("overshell", ObjectDir::Main());
    for (int i = 0; i < 2; i++) {
        mSlots[i]->SetPlayerNum(i);
    }
}

void Overshell::Poll(const Skeleton *const (&skeletons)[6]) {
    for (int i = 0; i < 2; i++) {
        mSlots[i]->Poll(skeletons);
    }
}

void Overshell::ResolveSkeletons() {
    if (TheGestureMgr) {
        for (int i = 0; i < 2; i++) {
            HamPlayerData *playerData = TheGameData->Player(i);
            if (!playerData->IsPlaying()) {
                mSlots[i]->SetState((OvershellSlotState)0);
                continue;
            }
            playerData = TheGameData->Player(i);
            Skeleton *skel = TheGestureMgr->GetSkeletonByTrackingID(
                playerData->GetSkeletonTrackingID()
            );

            playerData = TheGameData->Player(i);
            Symbol autoplay = playerData->Autoplay();
            if (skel || !autoplay.Null() || TheGestureMgr->Unk425C() == 1) {
                mSlots[i]->SetState((OvershellSlotState)3);
            } else {
                playerData = TheGameData->Player(i);
                if (playerData->GetSkeletonTrackingID() <= 0) {
                    mSlots[i]->SetState((OvershellSlotState)0);
                }
            }
        }
    }
}
