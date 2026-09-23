#include "hamobj/MiniGameMgr.h"
#include "hamobj/HamDirector.h"
#include "hamobj/HamGameData.h"
#include "hamobj/MoveGraph.h"
#include "hamobj/MoveMgr.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "os/Debug.h"

MiniGameMgr *TheMiniGameMgr;

class CompareMoveVariantName {
public:
    CompareMoveVariantName(const DataNode &n) : mName(n) {}

    bool operator()(const MoveVariant *mv) {
        DataNode otherMV = mv->Name();
        if (otherMV.Equal(mName, nullptr, true)) {
            return true;
        } else {
            return false;
        }
    }

private:
    DataNode mName; // 0x0
};

MiniGameMgr::MiniGameMgr() { mNumMovesNeeded = 2; }

BEGIN_HANDLERS(MiniGameMgr)
    HANDLE_ACTION(init_ntd, InitNTD(_msg->Int(2)))
    HANDLE_ACTION(get_move_options, GetMoveOptions(_msg->Array(2), _msg->Array(3)))
    HANDLE_EXPR(get_category, mCategory)
    HANDLE_ACTION(init_cascade, InitCascade(_msg->Int(2), _msg->Int(3)))
    HANDLE_ACTION(refresh_move_list, GetCascadeMoveList(_msg->Array(2), _msg->Array(3)))
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

void MiniGameMgr::Init() {
    TheMiniGameMgr = new MiniGameMgr();
    if (ObjectDir::Main()) {
        TheMiniGameMgr->SetName("mini_game_mgr", ObjectDir::Main());
    }
}

void MiniGameMgr::LoadMoveOptions(std::set<const MoveVariant *> &options, bool b2) {
    mCategory = TheMoveMgr->PickRandomCategory();
    TheMoveMgr->GenerateMoveChoice(mCategory, mValidMoves, mInvalidMoves);
    MILO_ASSERT(mValidMoves.size() > 0, 0x4E);
    MILO_ASSERT(mInvalidMoves.size() > mNumMovesNeeded - 1, 0x4F);
    options.insert(mValidMoves[0]);
    for (int i = 0; i < mNumMovesNeeded - 1; i++) {
        options.insert(mInvalidMoves[i]);
    }
    TheHamDirector->LoadRoutineBuilderData(options, b2);
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
    mValidMoves.clear();
    mInvalidMoves.clear();
    UpdateCascadeMovePool(TheMoveMgr->Graph(), mMovePool, mValidMoves);
    LoadValidMoves(false);
}

void MiniGameMgr::UpdateCascadeMovePool(
    MoveGraph &graph,
    std::vector<const MoveVariant *> &allMoves,
    std::vector<const MoveVariant *> &validMoves
) {
    allMoves.clear();
    FOREACH (it, graph.MoveParents()) {
        const MoveVariant *mv = it->second->PickRandomVariant();
        if (mv->IsValidForMinigame()
            && std::find(validMoves.begin(), validMoves.end(), mv) == validMoves.end()) {
            allMoves.push_back(mv);
        }
    }
    std::random_shuffle(allMoves.begin(), allMoves.end());
    for (int i = 0; i < 1; i++) {
        for (auto it = allMoves.begin(); it != allMoves.end()
             && allMoves.size() > mBlockingFactor + mNumMovesNeeded;) {
            const MoveVariant *mv = *it;
            bool b8 = true;
            if (i != 0) {
                MILO_ASSERT(0, 0xB3);
            } else {
                b8 = mv->Song() == TheGameData->GetSong();
            }
            if (b8) {
                ++it;
            } else {
                it = allMoves.erase(it);
            }
        }
    }
}

void MiniGameMgr::GetMoveOptions(DataArray *a1, DataArray *a2) {
    MILO_ASSERT(TheHamDirector->IsWorldLoaded(), 0x32);
    std::set<const MoveVariant *> options;
    options.clear();
    a1->Resize(1);
    a2->Resize(mNumMovesNeeded - 1);
    a1->Node(0) = mValidMoves.front()->Name();
    options.insert(mValidMoves.front());
    MILO_LOG("ADDING %s...\n", mValidMoves.front()->Name());
    for (int i = 0; i < mNumMovesNeeded - 1; i++) {
        a2->Node(i) = mInvalidMoves[i]->Name();
        options.insert(mInvalidMoves[i]);
        MILO_LOG("ADDING %s...\n", mInvalidMoves[i]->Name());
    }
    LoadMoveOptions(options, false);
}

void MiniGameMgr::LoadValidMoves(bool b1) {
    for (int i = 0; mValidMoves.size() < mNumMovesNeeded + mBlockingFactor;) {
        const MoveVariant *cur = mMovePool[i++];
        mValidMoves.push_back(cur);
    }
    std::set<const MoveVariant *> moveVariants;
    moveVariants.clear();
    FOREACH (it, mValidMoves) {
        moveVariants.insert(*it);
    }
    TheHamDirector->LoadRoutineBuilderData(moveVariants, b1);
}

void MiniGameMgr::GetCascadeMoveList(DataArray *a1, DataArray *a2) {
    int new_moves_count = a1->Size();
    if (new_moves_count != 0) {
        UpdateCascadeMovePool(TheMoveMgr->Graph(), mMovePool, mValidMoves);
        MILO_ASSERT(mMovePool.size() >= new_moves_count, 0xDA);
    }
    for (int i = 0; i < a1->Size(); i++) {
        DataNode n;
        n = a1->Node(i);
        mValidMoves.erase(
            std::remove_if(
                mValidMoves.begin(), mValidMoves.end(), CompareMoveVariantName(n)
            ),
            mValidMoves.end()
        );
    }
    if (mValidMoves.size() < mBlockingFactor + mNumMovesNeeded) {
        LoadValidMoves(false);
    }
    a2->Resize(mNumMovesNeeded);
    for (int i = 0; i < mNumMovesNeeded; i++) {
        a2->Node(i) = mValidMoves[i]->Name();
    }
}
