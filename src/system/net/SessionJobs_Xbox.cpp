#include "net/SessionJobs_Xbox.h"
#include "net/DingoSvr.h"
#include "obj/Object.h"
#include "xdk/win_types.h"
#include "xdk/XAPILIB.h"
#include "xdk/xapilibi/xbox.h"

XboxSessionJob::XboxSessionJob(void *v) : mSession(v), mSuccess(true) {
    memset(&mXOverlapped, 0, sizeof(XOVERLAPPED));
}

XboxSessionJob::~XboxSessionJob() {
    if (mXOverlapped.InternalLow == ERROR_IO_PENDING) {
        XCancelOverlapped(&mXOverlapped);
    }
}

bool XboxSessionJob::IsFinished() {
    DWORD dw;
    DWORD res = XGetOverlappedResult(&mXOverlapped, &dw, false);
    if (res == ERROR_IO_PENDING) {
        CheckError(res, &mXOverlapped);
    }
    return res;
}

void XboxSessionJob::Cancel(Hmx::Object *) { XCancelOverlapped(&mXOverlapped); }

void XboxSessionJob::CheckError(DWORD err, XOVERLAPPED *overlapped) {
    if (err && err != ERROR_IO_PENDING) {
        MILO_NOTIFY(
            "Error %i in Xbox Session: %x ", err, XGetOverlappedExtendedError(overlapped)
        );
        mSuccess = false;
    }
}

StartSessionJob::StartSessionJob(void *v) : XboxSessionJob(v) {}

void StartSessionJob::Start() {
    DWORD res = XSessionStart(mSession, 0, &mXOverlapped);
    CheckError(res, &mXOverlapped);
}

void StartSessionJob::OnCompletion(Hmx::Object *obj) {
    TheServer.StartSessionComplete(mSuccess);
}

EndSessionJob::EndSessionJob(void *v) : XboxSessionJob(v) {}

void EndSessionJob::Start() {
    DWORD res = XSessionEnd(mSession, &mXOverlapped);
    CheckError(res, &mXOverlapped);
}

void EndSessionJob::OnCompletion(Hmx::Object *) {
    TheServer.EndSessionComplete(mSuccess);
}

WriteCareerLeaderboardJob::WriteCareerLeaderboardJob(
    void *v, int i1, int i2, XUID u3, u64 u4
)
    : XboxSessionJob(v), mXUID(u3) {
    mUserProp.dwPropertyId = i2;
    mUserProp.value.i64Data = u4;
    mSessionViewProp.dwViewId = i1;
    mUserProp.value.type = 2;
    mSessionViewProp.dwNumProperties = 1;
    mSessionViewProp.pProperties = &mUserProp;
}

void WriteCareerLeaderboardJob::Start() {
    DWORD res = XSessionWriteStats(mSession, mXUID, 1, &mSessionViewProp, &mXOverlapped);
    CheckError(res, &mXOverlapped);
}

void WriteCareerLeaderboardJob::OnCompletion(Hmx::Object *) {
    TheServer.WriteCareerLeaderboardComplete(mSuccess);
}

MakeSessionJob::MakeSessionJob(HANDLE *v, DWORD dw, int idx)
    : mSession(v), mSessionFlags(dw), mUserIndex(idx), mSuccess(true) {
    memset(&mSessionInfo, 0, sizeof(XSESSION_INFO));
    memset(&mXOverlapped, 0, sizeof(XOVERLAPPED));
}

void MakeSessionJob::Start() {
    if (mSession) {
        ULONGLONG nonce;
        DWORD res = XSessionCreate(
            mSessionFlags, mUserIndex, 1, 0, &nonce, &mSessionInfo, &mXOverlapped, mSession
        );
        CheckError(res, &mXOverlapped);
    }
}

bool MakeSessionJob::IsFinished() {
    DWORD dw;
    DWORD res = XGetOverlappedResult(&mXOverlapped, &dw, false);
    if (res == 0x3E4) {
        return false;
    } else {
        if (res != 0) {
            *mSession = INVALID_HANDLE_VALUE;
            mSession = nullptr;
            mSuccess = false;
        }
        return true;
    }
}

void MakeSessionJob::Cancel(Hmx::Object *) {
    XCancelOverlapped(&mXOverlapped);
    if (mSession) {
        *mSession = INVALID_HANDLE_VALUE;
    }
}

void MakeSessionJob::OnCompletion(Hmx::Object *) {
    TheServer.MakeSessionJobComplete(mSuccess);
}

void MakeSessionJob::CheckError(DWORD err, XOVERLAPPED *overlapped) {
    if (err != 0 && err != 0x3E5) {
        MILO_NOTIFY(
            "Error %i in Xbox Session: %x ", err, XGetOverlappedExtendedError(overlapped)
        );
        mSuccess = false;
    }
}

DeleteSessionJob::DeleteSessionJob(void *v) : XboxSessionJob(v) {}

void DeleteSessionJob::Start() {
    DWORD res = XSessionDelete(mSession, &mXOverlapped);
    CheckError(res, &mXOverlapped);
}

void DeleteSessionJob::OnCompletion(Hmx::Object *) {
    CloseHandle(mSession);
    TheServer.DeleteSessionComplete(mSuccess);
}

AddLocalPlayerJob::AddLocalPlayerJob(void *v, int i, bool b)
    : XboxSessionJob(v), mUserIndex(i), mPrivateSlot(b) {}

void AddLocalPlayerJob::Start() {
    DWORD res = XSessionJoinLocal(mSession, 1, &mUserIndex, &mPrivateSlot, &mXOverlapped);
    CheckError(res, &mXOverlapped);
}

void AddLocalPlayerJob::OnCompletion(Hmx::Object *) {
    TheServer.JoinSessionComplete(mSuccess);
}

RemoveLocalPlayerJob::RemoveLocalPlayerJob(void *v, int i)
    : XboxSessionJob(v), mUserIndex(i) {}

void RemoveLocalPlayerJob::Start() {
    DWORD res = XSessionLeaveLocal(mSession, 1, &mUserIndex, &mXOverlapped);
    CheckError(res, &mXOverlapped);
}

void RemoveLocalPlayerJob::OnCompletion(Hmx::Object *) {
    TheServer.LeaveSessionComplete(mSuccess);
}
