#include "net/DingoSvr.h"
#include "DingoJob.h"
#include "WebSvcReq.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/PlatformMgr.h"
#include "utl/DataPointMgr.h"
#include "utl/Std.h"
#include "utl/Symbol.h"

DingoServer::DingoServer() : mAuthState(0), unk3c(0), unk70(-1), unk74(-1) {
    for (int i = 0; i < sizeof(unk78); i++) {
        unk78[i] = false;
    }
}

void DingoServer::Logout() {
    unk40 = "";
    mAuthState = 0;
    unk74 = -1;
    for (int i = 0; i < sizeof(unk78); i++) {
        unk78[i] = false;
    }
    unk80.Clear();
}

void DingoServer::Init() {}

void DingoServer::ManageJob(DingoJob *job) { MILO_ASSERT(job, 0xd0); }

void DingoServer::DelayJob(DingoJob *job) { unka4.push_back(job); }

void DingoServer::CancelDelayedCalls() {
    FOREACH (it, unka4) {
    }
}

void DingoServer::AddDelayedCalls() {}

void DingoServer::FillAuthParams(DataPoint &point) {
    static Symbol locale("locale");
    point.AddPair(locale, unk58.c_str());
    static Symbol language("language");
    point.AddPair(language, unk60.c_str());
}

void DingoServer::DoAdditionalLogin() {
    MILO_ASSERT(mAuthUrl.length() > 0, 0xa9);
    // MILO_ASSERT(mAuthState == kServerAuthed, line);
}

DataNode DingoServer::OnMsg(SigninChangedMsg const &) { return NULL_OBJ; }

DataNode DingoServer::OnMsg(ConnectionStatusChangedMsg const &) { return NULL_OBJ; }

DataNode DingoServer::OnMsg(DingoJobCompleteMsg const &) { return NULL_OBJ; }

bool DingoServer::InitAndAddJob(DingoJob *, bool, bool) { return false; }

bool DingoServer::Authenticate(int, char const *) { return false; }

bool DingoServer::SendAuthenticateMsg(char const *, DataPoint &, Hmx::Object *) {
    return false;
}

BEGIN_HANDLERS(DingoServer)
END_HANDLERS
