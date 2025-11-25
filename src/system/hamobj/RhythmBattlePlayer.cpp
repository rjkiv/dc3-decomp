#include "hamobj/RhythmBattlePlayer.h"
#include "char/Waypoint.h"
#include "gesture/GestureMgr.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamLabel.h"
#include "hamobj/HamPlayerData.h"
#include "math/Easing.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "os/Debug.h"
#include "rndobj/Anim.h"
#include "rndobj/PartLauncher.h"
#include "rndobj/Poll.h"
#include "utl/Symbol.h"

RhythmBattlePlayer::RhythmBattlePlayer()
    : unk8(this), unk1c(this), unk30(this), unk44(this), unk58(this), unk6c(this),
      unk80(this), unk94(this), unka8(this), unkbc(this), unkd0(this), unke4(this),
      unkf8(this), unk10c(this), unk120(this), unk134(this), unk148(this), unk15c(this),
      unk170(this), unk184(this), unk198(this), unk1ac(this), unk1c0(this), unk1d4(this),
      unk1e8(this), unk1fc(this), unk210(this), unk224(this), unk238(0), unk23c(0),
      unk244(0), unk248(0), unk250(0), unk258(0), unk25c(0), unk260(0), mInTheZone(-2),
      unk270(0), unk274(0), unk280(0), unk284(0), unk288(false), unk294(-1),
      unk298("none"), unk29c(0), unk2a4(false), unk2a5(false), unk2a8(0) {}

RhythmBattlePlayer::~RhythmBattlePlayer() {}

BEGIN_SAVES(RhythmBattlePlayer)
    SAVE_REVS(1, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    bs << unk8;
    bs << unk1c;
    bs << unk30;
    bs << unk44;
    bs << unk58;
    bs << unk6c;
END_SAVES

BEGIN_LOADS(RhythmBattlePlayer)
    LOAD_REVS(bs)
    ASSERT_REVS(1, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    bs >> unk8;
    bs >> unk1c;
    bs >> unk30;
    bs >> unk44;
    bs >> unk58;
    bs >> unk6c;
    bs >> unk80;
    bs >> unk10c;
    bs >> unk120;
    bs >> unk238;
    if (d.rev >= 1)
        bs >> unkf8;
END_LOADS

BEGIN_COPYS(RhythmBattlePlayer)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY_AS(RhythmBattlePlayer, c)
    BEGIN_COPYING_MEMBERS_FROM(c)
        COPY_MEMBER(unk8)
        COPY_MEMBER(unk1c)
        COPY_MEMBER(unk30)
        COPY_MEMBER(unk44)
        COPY_MEMBER(unk58)
        COPY_MEMBER(unk6c)
        COPY_MEMBER(unk238)
        COPY_MEMBER(unk120)
        COPY_MEMBER(unkbc)
        COPY_MEMBER(unkd0)
        COPY_MEMBER(unke4)
        COPY_MEMBER(unk184)
    END_COPYING_MEMBERS
END_COPYS

BEGIN_PROPSYNCS(RhythmBattlePlayer)

END_PROPSYNCS

int RhythmBattlePlayer::SwagJacked(Hmx::Object *, RhythmBattleJackState) {
    MILO_ASSERT(mInTheZone == 1, 0x44a);
    static Symbol rhythmbattle_swagjackeddd1("rhythmbattle_swagjackeddd1");
    unk298 = rhythmbattle_swagjackeddd1;
    unk260 = 0;
    if (unk170)
        unk170->Activate();
    return 1;
}

void RhythmBattlePlayer::HackPlayerQuit() {
    HamPlayerData *player = TheGameData->Player(unk238);
    int trackingID =
        TheGestureMgr->GetSkeletonIndexByTrackingID(player->GetSkeletonTrackingID());
}

int RhythmBattlePlayer::InTheZone() const { return mInTheZone == 1; }

float RhythmBattlePlayer::InAnimBeatLength() const { return 4.0f; }

void RhythmBattlePlayer::SetWindow(float f1, float f2) {
    unk28c = f1;
    unk290 = f2;
}

void RhythmBattlePlayer::SetInTheZone(int i, bool b1, bool b2) {
    if (!unk240)
        i = -1;

    if (mInTheZone == i)
        return;

    AnimateBoxyState(i, b1, b2);
}

void RhythmBattlePlayer::AnimateIn() { AnimateBoxyState(0, true, false); }

void RhythmBattlePlayer::ResetCombo() {
    int i = 0;
    if (!unk240)
        i = -1;

    if (mInTheZone != i)
        AnimateBoxyState(i, false, false);

    unk264 = 0;
    unk2a8 = 0;
    unk26c = mInTheZone;
    unk284 = 0.0f;
}

void RhythmBattlePlayer::SwagJackedBonus(
    Hmx::Object *o, RhythmBattleJackState state, int i
) {
    if (unk224) {
        unk224->Animate(
            unk224->StartFrame(),
            unk224->EndFrame(),
            unk224->Units(),
            0,
            0,
            o,
            kEaseLinear,
            0,
            0
        );
    }

    static Symbol swag_jacked("swag_jacked");
    HamPlayerData *player = TheGameData->Player(unk238);

    static Symbol rhythmbattle_swagjackeddd2("rhythmbattle_swagjackeddd2");
    unk298 = rhythmbattle_swagjackeddd2;
    if (i != 0) {
        unk260 = 1;
        unk284 = 16.0f;
    }
}

void RhythmBattlePlayer::SwapObjs(RhythmBattlePlayer *player) {
    RndAnimatable *temp = player->unk8;
    player->unk8 = unk8;
    unk8 = temp;

    temp = player->unk1c;
    player->unk1c = unk1c;
    unk1c = temp;

    temp = player->unk30;
    player->unk30 = unk30;
    unk30 = temp;

    temp = player->unk44;
    player->unk44 = unk44;
    unk44 = temp;

    temp = player->unk58;
    player->unk58 = unk58;
    unk58 = temp;

    temp = player->unk6c;
    player->unk6c = unk6c;
    unk6c = temp;

    temp = player->unka8;
    player->unka8 = unka8;
    unka8 = temp;

    temp = player->unkbc;
    player->unkbc = unkbc;
    unkbc = temp;

    temp = player->unkd0;
    player->unkd0 = unkd0;
    unkd0 = temp;

    temp = player->unke4;
    player->unke4 = unke4;
    unke4 = temp;

    HamLabel *tempLabel = player->unk10c;
    player->unk10c = unk10c;
    unk10c = tempLabel;

    tempLabel = player->unk120;
    player->unk120 = unk120;
    unk120 = tempLabel;

    temp = player->unk94;
    player->unk94 = unk94;
    unk94 = temp;

    unk288 = !unk288;
    player->unk288 = !player->unk288;
}

BEGIN_HANDLERS(RhythmBattlePlayer)
    HANDLE_SUPERCLASS(RndPollable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS
