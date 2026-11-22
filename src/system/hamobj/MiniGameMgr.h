#pragma once
#include "hamobj/HamGameData.h"
#include "hamobj/MoveGraph.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "utl/Symbol.h"
#include <vector>
#include <set>

class MiniGameMgr : public Hmx::Object {
protected:
    MiniGameMgr();
    // Hmx::Object
    virtual ~MiniGameMgr() {}

public:
    virtual DataNode Handle(DataArray *, bool);

    void InitNTD(int numMovesNeeded);
    void GetMoveOptions(DataArray *validArray, DataArray *invalidArray);
    void
    LoadMoveOptions(std::set<const MoveVariant *> &movesNeeded, bool load_transitions);
    void InitCascade(int numMovesNeeded, int blockingFactor);
    void GetCascadeMoveList(DataArray *defunct_moves, DataArray *out_moves);
    void UpdateCascadeMovePool(
        MoveGraph &moveGraph,
        std::vector<const MoveVariant *> &movePool,
        std::vector<const MoveVariant *> &excludedMoves
    );
    void LoadValidMoves(bool load_transitions);

    static void Init();

protected:
    int mNumMovesNeeded; // 0x2c
    int mBlockingFactor; // 0x30
    Symbol mCategory; // 0x34
    std::vector<const MoveVariant *> mValidMoves; // 0x38
    std::vector<const MoveVariant *> mInvalidMoves; // 0x44
    std::vector<const MoveVariant *> mMovePool; // 0x50
};

extern MiniGameMgr *TheMiniGameMgr;
