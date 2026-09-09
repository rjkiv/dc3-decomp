#include "hamobj/MocapSkeletonIterator.h"
#include "ClipPlayer.h"
#include "HamRegulate.h"
#include "hamobj/HamCharacter.h"
#include "hamobj/HamDirector.h"
#include "hamobj/HamGameData.h"
#include "math/Utl.h"
#include "obj/Task.h"
#include "os/Debug.h"

MocapSkeletonIterator::MocapSkeletonIterator(int x, int y)
    : mDancer(TheHamDirector->GetCharacter(0)), mInput(mDancer), unk24b0(x), unk24b4(y) {
    MILO_ASSERT(TheGameData, 0x16);
    unk24c4.Init();
    unk24bc = -kHugeFloat;
    unk24b8 = unk24b0;
    unk2f98 = TheTaskMgr.Seconds(TaskMgr::kRealTime);
    HamCharacter *c = mDancer;
    if (mDancer) {
        unk2f9c = mDancer->LocalXfm();
        c->Enter();
        mDancer->DirtyLocalXfm().Reset();
        mDancer->Teleport(nullptr);
        Update();
        mInput.ResetSkeletonCharOrigin();
    }
}

MocapSkeletonIterator::~MocapSkeletonIterator() {
    if (mDancer) {
        mDancer->DirtyLocalXfm().Reset();
        mDancer->Teleport(nullptr);
        mDancer->SetLocalXfm(unk2f9c);
    }
    TheTaskMgr.SetSeconds(unk2f98, true);
}

MocapSkeletonIterator::operator bool() { return mDancer && unk24b8 < unk24b4; }

void MocapSkeletonIterator::operator++() {
    unk24b8++;
    Update();
}

bool MocapSkeletonIterator::PrevSkeleton(
    const Skeleton &s, int i2, ArchiveSkeleton &as, int &i3
) const {
    return PrevFromArchive(*this, s, i2, as, i3);
}

void MocapSkeletonIterator::Update() {
    MILO_ASSERT(mDancer, 0x55);
    TheTaskMgr.SetSeconds(unk24b8 * 0.033333335f, (unk24b0 - unk24b8) == 0);
    ClipPlayer player(0);
    if (player.Init(0)) {
        player.PlayAnims(mDancer, unk24b8, unk24bc, 0);
        unk24bc = unk24b8;
    } else {
        MILO_NOTIFY(
            "Failed to init ClipPlayer for %s!", PathName(TheHamDirector->ClipDir())
        );
    }
    HamRegulate *reg = mDancer->Regulator();
    reg->SetWaypoint(nullptr);
    mDancer->Poll();
    if (unk24c4.IsTracked()) {
        AddToHistory(0, unk24c4);
    }
    mInput.PollTracking();
    const SkeletonFrame *frame_data = mInput.NewFrame();
    MILO_ASSERT(frame_data, 0x6F);
    MILO_ASSERT(frame_data->mElapsedMs == 33, 0x70);
    MILO_ASSERT(frame_data->mSkeletonDatas[0].mTracking == kSkeletonTracked, 0x71);
    unk24c4.Poll(0, *frame_data);
}
