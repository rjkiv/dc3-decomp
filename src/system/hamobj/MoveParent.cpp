#include "hamobj/MoveGraph.h"
#include "hamobj/Difficulty.h"
#include "math/Rand.h"
#include "obj/Data.h"
#include "os/Debug.h"
#include "utl/BinStream.h"
#include <set>

MoveParent::MoveParent() {}
MoveParent::MoveParent(const MoveParent *other) {
    mName = other->mName;
    mDifficulty = other->mDifficulty;
    mSuperEasy = false;
}

MoveParent::MoveParent(MoveGraph *graph, DataArray *arr) {
    static Symbol name("name");
    static Symbol genre_flags("genre_flags");
    static Symbol era_flags("era_flags");
    static Symbol variant("variant");
    static Symbol difficulty("difficulty");
    if (arr->Type(0) == kDataInt) {
        mName = MakeString("%i", arr->Int(0));
    } else {
        mName = arr->Sym(0);
    }
    DataArray *diffArr = arr->FindArray(difficulty, false);
    if (diffArr) {
        mDifficulty = SymToDifficulty(diffArr->Sym(1));
    } else {
        mDifficulty = kDifficultyExpert;
    }
    mGenreFlags.clear();
    DataArray *genreArr = arr->FindArray(genre_flags, false);
    if (genreArr) {
        for (int i = 1; i < genreArr->Size(); i++) {
            mGenreFlags.push_back(genreArr->Sym(i));
        }
    }
    mEraFlags.clear();
    DataArray *eraArr = arr->FindArray(era_flags, false);
    if (eraArr) {
        for (int i = 1; i < eraArr->Size(); i++) {
            mEraFlags.push_back(eraArr->Sym(i));
        }
    }
    mMoveVariants.clear();
    DataArray *variantArr = arr->FindArray(variant, true);
    for (int i = 1; i < variantArr->Size(); i++) {
        DataArray *curVarArr = variantArr->Array(i);
        MoveVariant *pVariant = new MoveVariant(graph, curVarArr, this);
        MILO_ASSERT(pVariant, 0x44);
        mMoveVariants.push_back(pVariant);
    }
    mSuperEasy = false;
}

MoveParent::~MoveParent() {
    for (int i = 0; i < mMoveVariants.size(); i++) {
        RELEASE(mMoveVariants[i]);
    }
    mMoveVariants.clear();
}

bool MoveParent::IsValidForMiniGame() const {
    FOREACH (it, mMoveVariants) {
        if ((*it)->IsValidForMinigame())
            return true;
    }
    return false;
}

const MoveVariant *MoveParent::PickRandomVariant() const {
    return mMoveVariants[RandomInt(0, mMoveVariants.size())];
}

bool MoveParent::HasPrevAdjacent(const MoveParent *parent) const {
    FOREACH (it, mPrev) {
        if (parent == *it)
            return true;
    }
    return false;
}

bool MoveParent::HasGenre(Symbol genre) const {
    for (int i = 0; i < mGenreFlags.size(); i++) {
        if (mGenreFlags[i] == genre)
            return true;
    }
    return false;
}

bool MoveParent::HasEra(Symbol era) const {
    for (int i = 0; i < mEraFlags.size(); i++) {
        if (mEraFlags[i] == era)
            return true;
    }
    return false;
}

bool MoveParent::HasCategory(Symbol cat) const { return HasGenre(cat) || HasEra(cat); }

bool MoveParent::HasFinalMoveVariant() const {
    FOREACH (it, mMoveVariants) {
        if ((*it)->IsFinalPose())
            return true;
    }
    return false;
}

bool MoveParent::HasRestMoveVariant() const {
    FOREACH (it, mMoveVariants) {
        if ((*it)->IsRest())
            return true;
    }
    return false;
}

void MoveParent::PopulateAdjacentParents() {
    std::set<const MoveParent *> nextParents;
    std::set<const MoveParent *> prevParents;
    FOREACH (it, mMoveVariants) {
        FOREACH (cand, (*it)->mNextCandidates) {
            nextParents.insert(cand->mVariant->GetParent());
        }
        FOREACH (cand, (*it)->mPrevCandidates) {
            prevParents.insert(cand->mVariant->GetParent());
        }
    }
    mNext.resize(nextParents.size());
    std::copy(nextParents.begin(), nextParents.end(), mNext.begin());
    mPrev.resize(prevParents.size());
    std::copy(prevParents.begin(), prevParents.end(), mPrev.begin());
}

void MoveParent::CacheLinks(MoveGraph *graph) {
    FOREACH (it, mMoveVariants) {
        (*it)->CacheLinks(graph);
    }
    PopulateAdjacentParents();
}

void MoveParent::Load(BinStream &bs, MoveGraph *graph) {
    int rev;
    bs >> rev;
    bs >> mName;
    int diff;
    bs >> diff;
    mDifficulty = (Difficulty)diff;
    int numFlags;
    bs >> numFlags;
    mGenreFlags.reserve(numFlags);
    for (int i = 0; i < numFlags; i++) {
        Symbol genre;
        bs >> genre;
        mGenreFlags.push_back(genre);
    }
    bs >> numFlags;
    mEraFlags.reserve(numFlags);
    for (int i = 0; i < numFlags; i++) {
        Symbol era;
        bs >> era;
        mEraFlags.push_back(era);
    }
    bs >> mSuperEasy;
    String str;
    bs >> str;
    bs >> numFlags;
    mMoveVariants.reserve(numFlags);
    for (int i = 0; i < numFlags; i++) {
        MoveVariant *var = new MoveVariant();
        var->Load(bs, graph, this);
        mMoveVariants.push_back(var);
    }
}
