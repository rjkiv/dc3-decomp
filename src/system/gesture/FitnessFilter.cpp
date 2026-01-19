#include "gesture/FitnessFilter.h"
#include "FitnessFilter.h"
#include "gesture/GestureMgr.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamPlayerData.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rndobj/Overlay.h"
#include "utl/Symbol.h"
#include "xdk/nui/nuifitnesslib.h"
#include "xdk/win_types.h"

#pragma region FitnessFilter

FitnessFilter::FitnessFilter()
    : unk4(true), unk5(false), unk6(false), unkc(false),
      mFitnessMeterOverlay(RndOverlay::Find("fitness_meter")), mPlayerIndex(0) {
    Clear();
}

void FitnessFilter::Clear() {
    mTrackingID = -1;
    unk4 = unk5 = unk6 = false;
}

void FitnessFilter::Poll() {
    if (!unk4 && unk6) {
        HamPlayerData *player = TheGameData->Player(mPlayerIndex);
        MILO_ASSERT(player, 0x3e);
        if (player->IsPlaying()) {
            int id = player->GetSkeletonTrackingID();
            bool b4;
            if (id <= 0) {
                b4 = false;
            } else {
                b4 = TheGestureMgr->GetSkeletonByTrackingID(id);
            }
            if (b4) {
                if (!unk5) {
                    HRESULT hr = NuiFitnessStartTracking(
                        NUI_FITNESS_TRACKING_AUTO, id, player->PadNum()
                    );
                    if (!SUCCEEDED(hr)) {
                        MILO_NOTIFY(
                            "NuiFitnessStartTracking failed with error 0x%08x", hr
                        );
                        unkc = false;
                    } else {
                        unkc = true;
                    }
                    mTrackingID = id;
                    unk5 = true;
                }
                if (mTrackingID != id && mTrackingID > 0) {
                    HRESULT pauseHr = NuiFitnessPauseTracking(mTrackingID);
                    if (!SUCCEEDED(pauseHr)) {
                        MILO_NOTIFY(
                            "NuiFitnessPauseTracking failed with error 0x%08x", pauseHr
                        );
                    }
                    HRESULT resumeHr = NuiFitnessResumeTracking(mTrackingID, id);
                    if (!SUCCEEDED(resumeHr)) {
                        MILO_NOTIFY(
                            "NuiFitnessResumeTracking failed with error 0x%08x", resumeHr
                        );
                    }
                    mTrackingID = id;
                }
            }
        }
    }
}

void FitnessFilter::Draw(const BaseSkeleton &, SkeletonViz &) {
    UpdateOverlay(nullptr, 0.05f);
}

void FitnessFilter::SetPlayerIndex(int index) {
    mPlayerIndex = index;
    if (mPlayerIndex == 0) {
        mFitnessMeterOverlay->SetCallback(this);
    }
}

bool FitnessFilter::GetFitnessData(float &f1, float &f2) const {
    f1 = 0;
    f2 = 0;
    if (mTrackingID > 0 && unk6 && unkc) {
        NUI_FITNESS_DATA data;
        HRESULT hr = NuiFitnessGetCurrentFitnessData(mTrackingID, &data);
        if (SUCCEEDED(hr)) {
            f1 = data.Joules * 0.00023900573f;
            f2 = data.DurationInMS / 1000.0f;
        } else {
            MILO_NOTIFY("NuiFitnessGetCurrentFitnessData failed with error 0x%08x", hr);
        }
        return true;
    } else {
        return false;
    }
}

bool FitnessFilter::GetFitnessDataAndReset(float &f1, float &f2) {
    if (GetFitnessData(f1, f2)) {
        HRESULT hr = NuiFitnessStopTracking(mTrackingID);
        if (!SUCCEEDED(hr)) {
            MILO_NOTIFY("NuiFitnessStopTracking failed with error 0x%08x", hr);
        }
        Clear();
        return true;
    } else {
        f1 = 0;
        f2 = 0;
        Clear();
        return false;
    }
}

void FitnessFilter::SetPaused(bool b1) {
    HamPlayerData *player = TheGameData->Player(mPlayerIndex);
    MILO_ASSERT(player, 0xB2);
    int playerId = -1;
    if (player->IsPlaying()) {
        playerId = player->GetSkeletonTrackingID();
    }
    if (unk5) {
        if (b1 && !unk4) {
            HRESULT hr = NuiFitnessPauseTracking(mTrackingID);
            if (!SUCCEEDED(hr)) {
                MILO_NOTIFY("NuiFitnessPauseTracking failed with error 0x%08x", hr);
            }
        } else {
            bool b4;
            if (playerId <= 0) {
                b4 = false;
            } else {
                b4 = TheGestureMgr->GetSkeletonByTrackingID(playerId);
            }
            if (b4 && unk4 && !b1) {
                HRESULT hr = NuiFitnessResumeTracking(mTrackingID, playerId);
                if (!SUCCEEDED(hr)) {
                    MILO_NOTIFY("NuiFitnessResumeTracking failed with error 0x%08x", hr);
                }
                mTrackingID = playerId;
            }
        }
    }
    unk4 = b1;
}

void FitnessFilter::StopTracking() {
    float x;
    GetFitnessDataAndReset(x, x);
}

#pragma endregion
#pragma region FitnessFilterObj

FitnessFilterObj::FitnessFilterObj() {}

BEGIN_HANDLERS(FitnessFilterObj)
    HANDLE_ACTION(set_player_index, mFilter.SetPlayerIndex(_msg->Int(2)))
    HANDLE_ACTION(start_tracking, mFilter.StartTracking())
    HANDLE_ACTION(stop_tracking, mFilter.StopTracking())
    HANDLE_ACTION(poll, mFilter.Poll())
    HANDLE_ACTION(set_paused, mFilter.SetPaused(_msg->Int(2)))
    HANDLE_EXPR(get_fitness_data, OnGetFitnessData(_msg))
    HANDLE_EXPR(get_fitness_data_and_reset, OnGetFitnessDataAndReset(_msg))
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_SAVES(FitnessFilterObj)
    MILO_ASSERT(0, 0xd7);
END_SAVES

BEGIN_COPYS(FitnessFilterObj)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(FitnessFilterObj)
    COPY_MEMBER(mFilter)
END_COPYS

BEGIN_LOADS(FitnessFilterObj)
    MILO_ASSERT(0, 0xd8);
END_LOADS

bool FitnessFilterObj::OnGetFitnessData(DataArray *a) const {
    float f40, f3c;
    bool ret = mFilter.GetFitnessData(f40, f3c);
    *a->Node(2).Var() = f40;
    if (a->Size() >= 4) {
        *a->Node(3).Var() = f3c;
    }
    return ret;
}

bool FitnessFilterObj::OnGetFitnessDataAndReset(DataArray *a) {
    float f40, f3c;
    bool ret = mFilter.GetFitnessDataAndReset(f40, f3c);
    *a->Node(2).Var() = f40;
    if (a->Size() >= 4) {
        *a->Node(3).Var() = f3c;
    }
    return ret;
}

#pragma endregion
