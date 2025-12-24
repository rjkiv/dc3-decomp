#include "hamobj/MiniGameMgr.h"
#include "hamobj/MoveMgr.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
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

void MiniGameMgr::InitCascade(int numMovesNeeded, int blockingFactor) {
    MILO_ASSERT(numMovesNeeded > 0, 0x6c);
    MILO_ASSERT(blockingFactor > 0, 0x6d);
    mNumMovesNeeded = numMovesNeeded;
    mBlockingFactor = blockingFactor;
    mMovePool.clear();
    mInvalidMoves.clear();
    mValidMoves.clear();
}

void MiniGameMgr::UpdateCascadeMovePool(
    MoveGraph &, std::vector<const MoveVariant *> &, std::vector<const MoveVariant *> &
) {}

void MiniGameMgr::GetMoveOptions(DataArray *, DataArray *) {}

void MiniGameMgr::LoadValidMoves(bool) {}

void MiniGameMgr::GetCascadeMoveList(DataArray *, DataArray *) {}

BEGIN_HANDLERS(MiniGameMgr)
    HANDLE_ACTION(init_ntd, InitNTD(_msg->Int(2)))
    HANDLE_ACTION(get_move_options, GetMoveOptions(_msg->Array(2), _msg->Array(3)))
    HANDLE_EXPR(get_category, 0)
    HANDLE_ACTION(init_cascade, InitCascade(_msg->Int(2), _msg->Int(3)))
    HANDLE_ACTION(refresh_move_list, GetCascadeMoveList(_msg->Array(2), _msg->Array(3)))
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS
