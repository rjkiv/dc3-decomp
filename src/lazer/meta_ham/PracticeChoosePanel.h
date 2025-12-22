#pragma once
#include "HamPanel.h"
#include "hamobj/HamMove.h"
#include "hamobj/PracticeSection.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "rndobj/Dir.h"
#include "stl/_vector.h"
#include "ui/UIListProvider.h"
#include "utl/Symbol.h"

class StepMoves : public PracticeStep {
public:
    ~StepMoves();
    StepMoves();
    StepMoves(StepMoves const &);
    bool IsRecap() const;
    int GetRatingHistory(int) const;
    float GetOverallRating() const;
    String GetDisplayName(bool) const;

    std::vector<HamMove *> unk18;
    bool unk24;
    int unk28;
    int unk2c;
};

class PracticeChoosePanel : public HamPanel, public UIListProvider {
public:
    // Hmx::Object
    OBJ_CLASSNAME(PracticeChoosePanel)
    OBJ_SET_TYPE(PracticeChoosePanel)
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);

    // UIPanel
    virtual void Enter();
    virtual void Exit();

    // UIListProvider
    virtual void Text(int, int, UIListLabel *, UILabel *) const;
    virtual RndMat *Mat(int, int, UIListMesh *) const;
    virtual Symbol DataSymbol(int) const;
    virtual int DataIndex(Symbol) const;
    virtual int NumData() const;
    virtual void InitData(RndDir *);

    PracticeChoosePanel();
    float StepStartBeat(int) const;
    float StepEndBeat(int) const;
    void SetUpCustomSection();

protected:
    std::vector<StepMoves> unk40;
    Symbol unk4c;
    Symbol unk50;
    std::vector<int> unk54;
    u32 unk60;

private:
    bool WantToAutoSelectRecommended() const;
    bool CheckIfAutoSelected(std::vector<HamMove *> const &) const;
    int GetStepNumber(StepMoves const &) const;
};
