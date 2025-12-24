#pragma once
#include "hamobj/Difficulty.h"
#include "obj/Object.h"
#include "rndobj/Dir.h"
#include "stl/_vector.h"
#include "ui/UILabel.h"
#include "ui/UIListLabel.h"
#include "ui/UIListProvider.h"
#include "utl/Symbol.h"

class DifficultyProvider : public UIListProvider, public Hmx::Object {
public:
    virtual Symbol DataSymbol(int) const;
    virtual void Text(int, int, UIListLabel *, UILabel *) const;
    virtual void InitData(RndDir *);
    virtual int NumData() const;

    DifficultyProvider();
    bool IsDifficultyUnlocked(Symbol) const;

    int unk30;
    std::vector<Difficulty> mDifficulties; // 0x34
};
