#include "gesture/FitnessFilter.h"
#include "FitnessFilter.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamPlayerData.h"
#include "obj/Data.h"
#include "os/Debug.h"
#include "rndobj/Overlay.h"
#include "utl/Symbol.h"

#pragma region FitnessFilter

FitnessFilter::FitnessFilter() : unk4(true), unk5(false), unk6(false), unkc(false) {
    Symbol fitness_meter("fitness_meter");
    unk10 = RndOverlay::Find(fitness_meter, true);

    unk14 = 0;
    unk8 = -1;

    unk6 = false;
    unk5 = false;
    unk4 = false;
}

void FitnessFilter::Clear() {
    unk6 = false;
    unk8 = -1;
    unk5 = false;
    unk4 = false;
}

void FitnessFilter::Draw(BaseSkeleton const &, SkeletonViz &) {}

void FitnessFilter::Poll() {
    if (!unk4 && unk6) {
        HamPlayerData *player = TheGameData->Player(unk14);
        MILO_ASSERT(player, 0x3e);

        if (player->IsPlaying()) {
        }
    }
}

void FitnessFilter::SetPlayerIndex(int index) {
    unk14 = index;
    if (index != 0)
        return;
    unk10->SetCallback(0);
}

bool FitnessFilter::GetFitnessData(float &, float &) const { return false; }

bool FitnessFilter::GetFitnessDataAndReset(float &, float &) { return true; }

void FitnessFilter::SetPaused(bool) {}

void FitnessFilter::StopTracking() {}

#pragma endregion FitnessFilter
#pragma region FitnessFilterObj

FitnessFilterObj::FitnessFilterObj() : unk2c(0) {}

BEGIN_SAVES(FitnessFilterObj)
    MILO_ASSERT(0, 0xd7);
END_SAVES

BEGIN_LOADS(FitnessFilterObj)
    MILO_ASSERT(0, 0xd8);
END_LOADS

void FitnessFilterObj::Copy(Hmx::Object const *, Hmx::Object::CopyType) {}

bool FitnessFilterObj::OnGetFitnessData(DataArray *) const { return false; }

bool FitnessFilterObj::OnGetFitnessDataAndReset(DataArray *) { return false; }

BEGIN_HANDLERS(FitnessFilterObj)
END_HANDLERS

#pragma endregion FitnessFilterObj
