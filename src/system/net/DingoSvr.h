#pragma once

#include "DingoJob.h"
#include "meta/ConnectionStatusPanel.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/OnlineID.h"
#include "os/PlatformMgr.h"
#include "stl/_vector.h"
#include "utl/DataPointMgr.h"
#include "utl/Str.h"
class DingoServer : public Hmx::Object {
public:
    virtual DataNode Handle(DataArray *, bool);
    virtual void Logout();
    virtual void Init();
    virtual void ManageJob(DingoJob *);

    DingoServer();
    void DelayJob(DingoJob *);
    void CancelDelayedCalls();
    void AddDelayedCalls();

    int mAuthState; // 0x2c
    String unk30;
    u32 unk38;
    int unk3c;
    String unk40;
    String mAuthUrl; // 0x48
    String unk50;
    String unk58;
    String unk60;
    String unk68;
    int unk70;
    int unk74;
    bool unk78[4];
    OnlineID unk80;
    std::vector<DingoJob *> unk98;
    std::vector<DingoJob *> unka4;

protected:
    virtual void FillAuthParams(DataPoint &);
    virtual void DoAdditionalLogin();

    DataNode OnMsg(SigninChangedMsg const &);
    DataNode OnMsg(ConnectionStatusChangedMsg const &);
    DataNode OnMsg(DingoJobCompleteMsg const &);
    bool InitAndAddJob(DingoJob *, bool, bool);
    bool Authenticate(int, char const *);

private:
    bool SendAuthenticateMsg(char const *, DataPoint &, Hmx::Object *);
};

extern const DingoServer &TheDingoServer;
extern DingoServer &TheServer;
