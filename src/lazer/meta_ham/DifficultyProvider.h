#pragma once
#include "hamobj/Difficulty.h"
#include "obj/Object.h"
#include "rndobj/Dir.h"
#include "ui/UILabel.h"
#include "ui/UIListLabel.h"
#include "ui/UIListProvider.h"
#include "utl/Symbol.h"

class DifficultyProvider : public UIListProvider, public Hmx::Object {
public:
    DifficultyProvider();
    virtual void Text(int, int, UIListLabel *, UILabel *) const;
    virtual Symbol DataSymbol(int) const;
    virtual int NumData() const { return mDifficulties.size(); }
    virtual void InitData(RndDir *);

    bool IsDifficultyUnlocked(Symbol) const;
    void SetPlayerIndex(int player) { mPlayerIndex = player; }

private:
    int mPlayerIndex; // 0x30
    std::vector<Difficulty> mDifficulties; // 0x34
};
