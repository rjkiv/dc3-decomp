#include "hamobj/MiniGameMgr.h"
#include "hamobj/MoveMgr.h"
#include "obj/Dir.h"
#include "os/Debug.h"

MiniGameMgr *TheMiniGameMgr;

MiniGameMgr::MiniGameMgr() { mNumMovesNeeded = 2; }

void MiniGameMgr::Init() {
    TheMiniGameMgr = new MiniGameMgr();
    if (ObjectDir::Main()) {
        TheMiniGameMgr->SetName("mini_game_mgr", ObjectDir::Main());
    }
}

void MiniGameMgr::LoadMoveOptions(std::set<const MoveVariant *> &options, bool b2) {
    unk34 = TheMoveMgr->PickRandomCategory();
    TheMoveMgr->GenerateMoveChoice(unk34, mValidMoves, mInvalidMoves);
    MILO_ASSERT(mValidMoves.size() > 0, 0x4E);
    MILO_ASSERT(mInvalidMoves.size() > mNumMovesNeeded - 1, 0x4F);
    options.insert(mValidMoves[0]);
    for (int i = 0; i < mNumMovesNeeded - 1; i++) {
        options.insert(mInvalidMoves[i]);
    }
}

void MiniGameMgr::InitNTD(int numMovesNeeded) {
    MILO_ASSERT(numMovesNeeded > 1, 0x23);
    mInvalidMoves.clear();
    mMovePool.clear();
    mValidMoves.clear();
    mNumMovesNeeded = numMovesNeeded;
    std::set<const MoveVariant *> options;
    options.clear();
    LoadMoveOptions(options, false);
}
