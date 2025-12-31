#pragma once
#include "hamobj/MiniLeaderboardDisplay.h"
#include "net/DingoSvr.h"
#include "net_ham/LeaderboardJobs.h"
#include "net_ham/RCJobDingo.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "ui/UIListProvider.h"
#include "ui/UIListWidget.h"
#include "utl/Symbol.h"

class AppMiniLeaderboardDisplay : public MiniLeaderboardDisplay, public UIListProvider {
public:
    AppMiniLeaderboardDisplay();
    // Hmx::Object
    virtual ~AppMiniLeaderboardDisplay();
    // not an oversight
    OBJ_CLASSNAME(MiniLeaderboardDisplay);
    OBJ_SET_TYPE(MiniLeaderboardDisplay);
    virtual DataNode Handle(DataArray *, bool);
    // RndPollable
    virtual void Poll();
    virtual void Enter();
    virtual void Exit();
    // RndDrawable
    virtual void DrawShowing();
    // UIListProvider
    virtual void Text(int, int, UIListLabel *, UILabel *) const;
    virtual int NumData() const;
    virtual UIListWidgetState ElementStateOverride(int, int, UIListWidgetState s) const;

    NEW_OBJ(AppMiniLeaderboardDisplay)

    bool UpdateLeaderboard(Symbol);

protected:
    virtual void Update();

    void UpdateData(GetMiniLeaderboardJob *);
    void UpdateLeaderboardOnline(int);
    void ClearData();
    DataNode OnMsg(ServerStatusChangedMsg const &);
    DataNode OnMsg(RCJobCompleteMsg const &);

    int unk60;
    UIList *mLeaderboardList; // 0x64
    int unk68;
    float unk6c;
    std::vector<LeaderboardRow> unk70;

private:
    void UpdateSelfInRows();
};
