#include "gesture/SkeletonUpdate.h"
#include "BaseSkeleton.h"
#include "SkeletonUpdate.h"
#include "StubCameraInput.h"
#include "gesture/CameraInput.h"
#include "gesture/GestureMgr.h"
#include "gesture/LiveCameraInput.h"
#include "gesture/Skeleton.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamPlayerData.h"
#include "math/Vec.h"
#include "obj/DataFunc.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "os/CritSec.h"
#include "os/Debug.h"
#include "os/Joypad.h"
#include "os/OSFuncs.h"
#include "os/Timer.h"
#include "utl/MemMgr.h"
#include "utl/Std.h"
#include "xdk/NUI.h"
#include "xdk/XAPILIB.h"
#include "xdk/nui/nuiskeleton.h"
#include "xdk/win_types.h"
#include "xdk/xapilibi/synchapi.h"

CriticalSection SkeletonUpdateHandle::sCritSec;
static const float sFloat = 2.0f;

#pragma region SkeletonUpdateHandle

SkeletonUpdateHandle::SkeletonUpdateHandle(SkeletonUpdate *update) : mInst(update) {
    MILO_ASSERT(mInst, 0x45);
    sCritSec.Enter();
}

SkeletonUpdateHandle::~SkeletonUpdateHandle() { sCritSec.Exit(); }

std::vector<SkeletonCallback *> &SkeletonUpdateHandle::Callbacks() {
    return mInst->mCallbacks;
}
CameraInput *SkeletonUpdateHandle::GetCameraInput() const { return mInst->mCameraInput; }
void SkeletonUpdateHandle::SetCameraInput(CameraInput *input) {
    mInst->SetCameraInput(input);
}

bool SkeletonUpdateHandle::HasCallback(SkeletonCallback *cb) {
    return VectorFind(mInst->mCallbacks, cb);
}

void SkeletonUpdateHandle::AddCallback(SkeletonCallback *cb) {
    MILO_ASSERT(!HasCallback(cb), 0xA2);
    mInst->mCallbacks.push_back(cb);
}

void SkeletonUpdateHandle::RemoveCallback(SkeletonCallback *cb) {
    MILO_ASSERT(HasCallback(cb), 0xA8);
    mInst->mCallbacks.erase(
        std::find(mInst->mCallbacks.begin(), mInst->mCallbacks.end(), cb)
    );
}

void SkeletonUpdateHandle::PostUpdate() { mInst->PostUpdate(); }

#pragma endregion
#pragma region SkeletonUpdate

static bool sBool878;

DWORD SkeletonUpdateThread(LPVOID) {
    HANDLE new_skeleton_event = SkeletonUpdate::NewSkeletonEvent();
    MILO_ASSERT(new_skeleton_event, 0x21);
    HANDLE skeleton_updated_event = SkeletonUpdate::SkeletonUpdatedEvent();
    MILO_ASSERT(skeleton_updated_event, 0x23);
    WaitForSingleObject(new_skeleton_event, -1);
    while (!sBool878) {
        SkeletonUpdateHandle handle = SkeletonUpdate::InstanceHandle();
        //     SkeletonUpdate::InstanceHandle(local_40);
        //     if (local_40[0][0x539c] != (SkeletonUpdate)0x0) {
        //       SkeletonUpdate::Update(local_40[0]);
        //       SetEvent(hEvent);
        //     }
        //     CriticalSection::Exit(&SkeletonUpdateHandle::sCritSec);
        WaitForSingleObject(new_skeleton_event, -1);
    }
    return 0;
}

SkeletonUpdate::SkeletonUpdate()
    : unk78(0), mCameraInput(this), unk90(0), unk91(0), unk5388(0), unk538c(0),
      unk5390(0), unk5394(0), unk5398(0), unk539c(true) {
    MILO_ASSERT(sInstance == NULL, 0x119);
    SetCameraInput(LiveCameraInput::sInstance);
    for (int i = 0; i < 2; i++) {
        unk5360[i] = nullptr;
        unk5368[i] = nullptr;
    }
    mNUISkeletonFrame = (NUI_SKELETON_FRAME *)MemAlloc(
        sizeof(NUI_SKELETON_FRAME), __FILE__, 0x126, "NUI_SKELETON_FRAME", 0x10
    );
    memset(mNUISkeletonFrame, 0, sizeof(NUI_SKELETON_FRAME));
    memset(&mSkeletonFrame, 0, sizeof(SkeletonFrame));
    unk53a0 = CreateThread(nullptr, 0, SkeletonUpdateThread, nullptr, 4, nullptr);
    XSetThreadProcessor(unk53a0, 5);
    ResumeThread(unk53a0);
}

SkeletonUpdate::~SkeletonUpdate() {
    sBool878 = true;
    SetEvent(sNewSkeletonEvent);
    WaitForSingleObject(unk53a0, -1);
    CloseHandle(unk53a0);
    unk53a0 = nullptr;
    MemFree(mNUISkeletonFrame);
}

bool SkeletonUpdate::PrevSkeleton(
    const Skeleton &s, int i2, ArchiveSkeleton &as, int &iref
) const {
    return SkeletonHistory::PrevFromArchive(*this, s, i2, as, iref);
}

SkeletonUpdateHandle SkeletonUpdate::InstanceHandle() {
    MILO_ASSERT(sInstance, 0x146);
    return sInstance;
}

bool SkeletonUpdate::Replace(ObjRef *from, Hmx::Object *to) {
    if (from == &mCameraInput) {
        SetCameraInput(LiveCameraInput::sInstance);
    }
    return Hmx::Object::Replace(from, to);
}

void SkeletonUpdate::SetCameraInput(CameraInput *cam_input) {
    MILO_ASSERT(cam_input, 0x165);
    mCameraInput = cam_input;
    FOREACH (it, mCallbacks) {
        (*it)->Clear();
    }
}

void SkeletonUpdate::CreateInstance() {
    MILO_ASSERT(sInstance == NULL, 0x102);
    sInstance = new SkeletonUpdate();
}

void SkeletonUpdate::Terminate() { RELEASE(sInstance); }
bool SkeletonUpdate::HasInstance() { return sInstance; }
void *SkeletonUpdate::NewSkeletonEvent() { return sNewSkeletonEvent; }

void SkeletonUpdateCallbackSlowdownCB(float, void *);

void SkeletonUpdate::PostUpdate() {
    MILO_ASSERT(MainThread(), 0x26F);
    MILO_ASSERT(mCameraInput, 0x273);
    mCameraInput->PollTracking();
    unk90 = mCameraInput->IsConnected();
    unk91 = mCameraInput->IsOverride();
    if (unk91) {
        const SkeletonFrame *newFrame = mCameraInput->NewFrame();
        if (newFrame) {
            mSkeletonFrame = *newFrame;
            unk78 = true;
        }
    }
    if (TheGameData) {
        for (int i = 0; i < 2; i++) {
            HamPlayerData *player_data = TheGameData->Player(i);
            MILO_ASSERT(player_data, 0x28B);
            unk5380[i] = player_data->GetSkeletonTrackingID();
        }
    }
    if (unk539c) {
        WaitForSingleObject(sSkeletonUpdatedEvent, 1);
        ResetEvent(sSkeletonUpdatedEvent);
    } else {
        Update();
    }
    if (unk78) {
        LiveCameraInput::sInstance->SetNewFrame(&mSkeletonFrame);
    }
    SkeletonUpdateData updateData(*unk5360[0], *unk5368[0]);
    updateData.unk8 = &mSkeletonFrame;
    updateData.unkc = this;
    updateData.unk10 = mCameraInput;
    FOREACH (it, mCallbacks) {
        AutoGlitchReport report(4.0f, SkeletonUpdateCallbackSlowdownCB, *it);
        (*it)->PostUpdate(unk78 ? &updateData : nullptr);
    }
    unk78 = false;
    for (int i = 0; i < NUM_SKELETONS; i++) {
        mSkeletons[i].PostUpdate();
    }
}

DataNode OnToggleSkeletalUpdateThread(DataArray *) {
    SkeletonUpdateHandle handle = SkeletonUpdate::InstanceHandle();
    handle.Inst()->SetUnk539C(!handle.Inst()->GetUnk539C());
    ResetEvent(SkeletonUpdate::SkeletonUpdatedEvent());
    return handle.Inst()->GetUnk539C();
}

DataNode OnCycleNumStubSkeletons(DataArray *) {
    SkeletonUpdateHandle handle = SkeletonUpdate::InstanceHandle();
    int i = (handle.Inst()->GetUnk5388() + 1) % 3;
    if (i < 0) {
        i += 3;
    }
    handle.Inst()->SetUnk5388(i);
    return i;
}

DataNode OnCycleFakeShellSkeletons(DataArray *arr) {
    SkeletonUpdateHandle handle = SkeletonUpdate::InstanceHandle();
    int i = (1 << arr->Int(1)) ^ handle.Inst()->GetUnk538C();
    handle.Inst()->SetUnk538C(i);
    return i;
}

DataNode OnCycleActiveFakeShellSkeleton(DataArray *) {
    SkeletonUpdateHandle handle = SkeletonUpdate::InstanceHandle();
    int i = (handle.Inst()->GetUnk5394() + 1) % 2;
    if (i < 0) {
        i += 2;
    }
    handle.Inst()->SetUnk5394(i);
    return i;
}

DataNode OnSetFakeSkeletonSidesSwapped(DataArray *arr) {
    SkeletonUpdateHandle handle = SkeletonUpdate::InstanceHandle();
    bool b = arr->Int(1) != 0;
    handle.Inst()->SetUnk5390(b);
    return 0;
}

DataNode OnGetFakeSkeletonSidesSwapped(DataArray *) {
    SkeletonUpdateHandle handle = SkeletonUpdate::InstanceHandle();
    return handle.Inst()->GetUnk5390();
}

void SkeletonUpdate::Init() {
    sNewSkeletonEvent = CreateEventA(nullptr, true, false, nullptr);
    sSkeletonUpdatedEvent = CreateEventA(nullptr, true, false, nullptr);
    DataRegisterFunc("toggle_skeletal_update_thread", OnToggleSkeletalUpdateThread);
    DataRegisterFunc("cycle_num_stub_skeletons", OnCycleNumStubSkeletons);
    DataRegisterFunc("cycle_fake_shell_skeletons", OnCycleFakeShellSkeletons);
    DataRegisterFunc("cycle_active_fake_shell_skeleton", OnCycleActiveFakeShellSkeleton);
    DataRegisterFunc("set_fake_skeleton_sides_swapped", OnSetFakeSkeletonSidesSwapped);
    DataRegisterFunc("get_fake_skeleton_sides_swapped", OnGetFakeSkeletonSidesSwapped);
}

void SkeletonUpdate::UpdateFakeArmPos() {
    JoypadData *data = JoypadGetPadData(0);
    float sticks = data->mSticks[1][1];
    float uiSeconds = TheTaskMgr.DeltaUISeconds();
    uiSeconds *= sFloat;
    unk5398 = -(uiSeconds * sticks - unk5398);

    float val = -0.25f;
    val = (-0.25f - unk5398 >= 0) ? -0.25f : unk5398;
    unk5398 = (val - 0.6f >= 0) ? 0.6f : val;
}

void SkeletonUpdate::Update() {
    LONGLONG l = mNUISkeletonFrame->liTimeStamp.QuadPart;
    if (NuiSkeletonGetNextFrame(0, mNUISkeletonFrame) == 0) {
        unk78 = true;
        if (!unk91) {
            mSkeletonFrame.Create(
                *mNUISkeletonFrame, mNUISkeletonFrame->liTimeStamp.QuadPart - l
            );
        }
    } else {
        if (unk90) {
            return;
        }

        if (!unk91) {
            unk78 = true;
            StubCameraInput::StubSkeletonFrame(mSkeletonFrame);
            for (int i = 0; i < NUM_SKELETONS; i++) {
                SkeletonData &data = mSkeletonFrame.mSkeletonDatas[i];
                data.mTracking = kSkeletonNotTracked;
                data.mQualityFlags = 0;
                for (int j = 0; j < kNumJoints; j++) {
                    data.unk144[j].Zero();
                    data.unk284[j] = 0;
                }
                data.mTrackingID = -1;
                data.unk2dc = -1;
                data.unk2e0 = Vector3::GetZero();
            }
        }
    }
    UpdateCallbacks();
}
