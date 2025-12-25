#pragma once
#include "hamobj/MiniLeaderboardDisplay.h"
#include "obj/Object.h"
#include "ui/UIListProvider.h"

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

protected:
    int unk60;
    int unk64;
    int unk68;
    float unk6c;
    std::vector<AppMiniLeaderboardDisplay> unk70;
};
