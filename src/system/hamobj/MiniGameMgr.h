#pragma once
#include "hamobj/MoveGraph.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include <set>

class MiniGameMgr : public Hmx::Object {
protected:
    MiniGameMgr();
    // Hmx::Object
    virtual ~MiniGameMgr() {}

public:
    virtual DataNode Handle(DataArray *, bool);

    void GetMoveOptions(DataArray *, DataArray *);
    void LoadMoveOptions(std::set<const MoveVariant *> &, bool);
    void InitNTD(int);

    static void Init();

protected:
    int mNumMovesNeeded; // 0x2c
    int mBlockingFactor; // 0x30
    Symbol unk34; // 0x34
    std::vector<const MoveVariant *> mValidMoves; // 0x38
    std::vector<const MoveVariant *> mInvalidMoves; // 0x44
    std::vector<const MoveVariant *> mMovePool; // 0x50
};

extern MiniGameMgr *TheMiniGameMgr;
