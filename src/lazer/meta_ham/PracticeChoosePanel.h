#pragma once
#include "HamPanel.h"
#include "hamobj/HamMove.h"
#include "hamobj/PracticeSection.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "rndobj/Dir.h"
#include "rndobj/Mat.h"
#include "ui/UIListProvider.h"
#include "utl/Symbol.h"
#include <vector>

class StepMoves : public PracticeStep {
    friend bool PropSync(StepMoves &, DataNode &, DataArray *, int, PropOp);

public:
    StepMoves() : mSelected(false), unk28(0), unk2c(0) {}
    ~StepMoves() {}
    bool operator<(const StepMoves &) const;

    bool IsRecap() const;
    int GetRatingHistory(int) const;
    float GetOverallRating() const;
    String GetDisplayName(bool) const;

    // rename these once you have a better idea of what they do
    bool MoveRecapCheck() const { return mMoves.empty() && !IsRecap(); }
    bool HistoryCheck() const {
        int history = GetRatingHistory(0);
        return history >= 0 && history < 2;
    }

    std::vector<HamMove *> mMoves; // 0x18
    bool mSelected; // 0x24
    int unk28;
    int unk2c;
};

class PracticeChoosePanel : public HamPanel, public UIListProvider {
public:
    PracticeChoosePanel();
    // Hmx::Object
    OBJ_CLASSNAME(PracticeChoosePanel)
    OBJ_SET_TYPE(PracticeChoosePanel)
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);

    // UIPanel
    virtual void Enter() { HamPanel::Enter(); }
    virtual void Exit();

    // UIListProvider
    virtual void Text(int, int, UIListLabel *, UILabel *) const;
    virtual RndMat *Mat(int, int, UIListMesh *) const;
    virtual Symbol DataSymbol(int) const;
    virtual int DataIndex(Symbol) const;
    virtual int NumData() const;
    virtual void InitData(RndDir *);

    NEW_OBJ(PracticeChoosePanel)

    float StepStartBeat(int) const;
    float StepEndBeat(int) const;
    void SetUpCustomSection();
    int NumSelected() const;
    void ToggleSort();

private:
    bool WantToAutoSelectRecommended() const;
    bool CheckIfAutoSelected(const std::vector<HamMove *> &) const;
    int GetStepNumber(const StepMoves &) const;
    std::vector<HamMove *> GetMovesInStep(PracticeStep);

    std::vector<StepMoves> mStepMoves; // 0x40
    Symbol unk4c; // 0x4c - song
    Symbol unk50; // 0x50 - difficulty sym
    std::vector<int> unk54; // 0x54 - memory?
    RndMat *unk60; // 0x60
};
