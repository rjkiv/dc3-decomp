#include "MoveGraph.h"
#include "hamobj/Difficulty.h"
#include "hamobj/MoveGraph.h"
#include "obj/Data.h"
#include "os/Debug.h"
#include "utl/BinStream.h"

MoveCandidate::MoveCandidate(Symbol s1, Symbol, Symbol s3, bool b4) : mAdjacencyFlag(0) {
    if (b4)
        mAdjacencyFlag = 2;
    mAdjacencyFlag |= Adjacency(s3);
    mValue.mVariantName = s1.Str();
}

MoveCandidate::MoveCandidate(const MoveCandidate &c) {
    if (c.mAdjacencyFlag & 1) {
        mValue.mVariantName = c.mValue.mVariant->mVariantName.Str();
    } else
        mValue.mVariant = c.mValue.mVariant;
    mAdjacencyFlag = c.mAdjacencyFlag & ~1;
}

void MoveCandidate::CacheLinks(MoveGraph *graph) {
    const char *variantName =
        mAdjacencyFlag & 1 ? mValue.mVariant->mVariantName.Str() : mValue.mVariantName;
    mValue.mVariant = graph->FindNonConstMoveByVariantName(variantName);
    if (!mValue.mVariant) {
        MILO_FAIL("Could not find link to %s", variantName);
    }
    mAdjacencyFlag |= 1;
}

void MoveCandidate::Load(BinStream &bs) {
    int rev;
    bs >> rev;
    bs >> mAdjacencyFlag;
    Symbol s1, s2, s3;
    bs >> s1;
    bs >> s2;
    bs >> s3;
    mValue.mVariantName = s2.Str();
    if (rev < 1) {
        mAdjacencyFlag |= Adjacency(s3);
    }
}

unsigned int MoveCandidate::Adjacency(Symbol s) {
    static Symbol original_adjacent("original_adjacent");
    static Symbol easy_med_adjacent("easy_med_adjacent");
    static Symbol auto_score("auto_score");
    static Symbol anim_hot_or_not("anim_hot_or_not");
    static Symbol fake("fake");

    if (s == original_adjacent)
        return 4;
    else if (s == easy_med_adjacent)
        return 8;
    else if (s == auto_score)
        return 0x10;
    else if (s == anim_hot_or_not)
        return 0x20;
    else if (s == fake)
        return 0;
    else {
        MILO_NOTIFY("MoveCandidate has unknown source '%s'", s.Str());
        return 0;
    }
}

MoveVariant::MoveVariant(MoveGraph *graph, const MoveVariant *other, MoveParent *parent) {
    mVariantName = other->mVariantName;
    mMoveParent = parent;
    mPrevCandidates = other->mPrevCandidates;
    mNextCandidates = other->mNextCandidates;
    mSongName = other->mSongName;
    mAvgBeatsPerSec = other->mAvgBeatsPerSec;
    mHamMoveName = other->mHamMoveName;
    mHamMoveMiloName = other->mHamMoveMiloName;
    mGenre = other->mGenre;
    mEra = other->mEra;
    mFlags = other->mFlags & ~1;
    if (other->mLinkedTo.mVariant) {
        mLinkedTo.mVariantName = other->mLinkedTo.mVariant->mSongName.Str();
    } else {
        mLinkedTo.mVariant = nullptr;
    }
    if (other->mLinkedFrom.mVariant) {
        mLinkedFrom.mVariantName = other->mLinkedFrom.mVariant->mSongName.Str();
    } else {
        mLinkedFrom.mVariant = nullptr;
    }
    mPositionOffset = other->mPositionOffset;
    graph->mMoveVariants[mSongName] = this;
}

MoveVariant::MoveVariant(MoveGraph *graph, DataArray *cfg, MoveParent *parent) {
    static Symbol song_name("song_name");
    static Symbol average_beats_per_second("average_beats_per_second");
    static Symbol genre("genre");
    static Symbol era("era");
    static Symbol scored("scored");
    static Symbol paradiddle("paradiddle");
    static Symbol final_pose("final_pose");
    static Symbol suppress_guide_gesture("suppress_guide_gesture");
    static Symbol suppress_practice_options("suppress_practice_options");
    static Symbol omit_minigame("omit_minigame");
    static Symbol useful("useful");
    static Symbol difficulty("difficulty");
    static Symbol linked_to("linked_to");
    static Symbol linked_from("linked_from");
    static Symbol position_offset("position_offset");
    static Symbol ham_move_name("ham_move_name");
    static Symbol ham_move_milo_name("ham_move_milo_name");
    static Symbol prev_candidates("prev_candidates");
    static Symbol next_candidates("next_candidates");
    static Symbol original_adjacent("original_adjacent");
    mMoveParent = parent;
    mVariantName = cfg->Sym(0);
    mSongName = cfg->FindSym(song_name);
    mAvgBeatsPerSec = cfg->FindFloat(average_beats_per_second);
    mGenre = cfg->FindSym(genre);
    mFlags = 0;
    mEra = cfg->FindSym(era);
    mFlags |= cfg->FindInt(scored, 0) << 1;
    mFlags |= cfg->FindInt(final_pose, 0) << 3;
    mFlags |= cfg->FindInt(suppress_guide_gesture, 0) << 4;
    mFlags |= cfg->FindInt(suppress_practice_options, 0) << 7;
    mFlags |= cfg->FindInt(omit_minigame, 0) << 5;
    bool isUseful = cfg->FindInt(useful, 0);
    if (isUseful)
        mFlags |= 0x40;
    else
        mFlags &= ~0x40;
    if (cfg->FindArray(linked_to, false))
        mLinkedTo.mVariantName = cfg->FindSym(linked_to).Str();
    else
        mLinkedTo.mVariant = nullptr;
    if (cfg->FindArray(linked_from, false))
        mLinkedFrom.mVariantName = cfg->FindSym(linked_from).Str();
    else
        mLinkedFrom.mVariant = 0;

    if (cfg->FindArray(position_offset, false)) {
        mPositionOffset.x = cfg->FindArray(position_offset)->Float(1);
        mPositionOffset.y = cfg->FindArray(position_offset)->Float(2);
        mPositionOffset.z = cfg->FindArray(position_offset)->Float(3);
    } else {
        mPositionOffset.Zero();
    }
    mHamMoveName = cfg->FindSym(ham_move_name);
    mHamMoveMiloName = cfg->FindSym(ham_move_milo_name);
    mPrevCandidates.clear();
    mNextCandidates.clear();

    DataArray *prevCandsArr = cfg->FindArray(prev_candidates, true);
    for (int i = 1; i < prevCandsArr->Size(); i++) {
        Symbol s1 = prevCandsArr->Array(i)->Type(0) == kDataInt
            ? MakeString("%i", prevCandsArr->Array(i)->Int(0))
            : prevCandsArr->Array(i)->Sym(0);
        Symbol s2 = prevCandsArr->Array(i)->Type(1) == kDataInt
            ? MakeString("%i", prevCandsArr->Array(i)->Int(1))
            : prevCandsArr->Array(i)->Sym(1);
        bool b3 = prevCandsArr->Array(i)->Int(2);
        Symbol s3 = prevCandsArr->Array(i)->Size() > 3 ? prevCandsArr->Array(i)->Sym(3)
                                                       : original_adjacent;
        mPrevCandidates.push_back(MoveCandidate(s1, s2, s3, b3));
    }
    DataArray *nextCandsArr = cfg->FindArray(next_candidates, true);
    for (int i = 1; i < nextCandsArr->Size(); i++) {
        Symbol s1 = nextCandsArr->Array(i)->Type(0) == kDataInt
            ? MakeString("%i", nextCandsArr->Array(i)->Int(0))
            : nextCandsArr->Array(i)->Sym(0);
        Symbol s2 = nextCandsArr->Array(i)->Type(1) == kDataInt
            ? MakeString("%i", nextCandsArr->Array(i)->Int(1))
            : nextCandsArr->Array(i)->Sym(1);
        bool b3 = nextCandsArr->Array(i)->Int(2);
        Symbol s3 = nextCandsArr->Array(i)->Size() > 3 ? nextCandsArr->Array(i)->Sym(3)
                                                       : original_adjacent;
        mNextCandidates.push_back(MoveCandidate(s1, s2, s3, b3));
    }
    graph->mMoveVariants[mVariantName] = this;
}

MoveVariant::~MoveVariant() {
    mPrevCandidates.clear();
    mNextCandidates.clear();
}

Difficulty MoveVariant::GetDifficulty() const { return mMoveParent->GetDifficulty(); }

bool MoveVariant::IsRest() const {
    static Symbol Rest("Rest.move");
    static Symbol rest("rest.move");
    static Symbol groove("groove");
    if (mHamMoveName == Rest || mHamMoveName == rest || mHamMoveName == groove)
        return true;
    String moveStr(mHamMoveName);
    if (moveStr.contains("finish"))
        return true;
    else
        return false;
}

void MoveVariant::CacheLinks(MoveGraph *graph) {
    mLinkedTo.mVariant = graph->FindMoveByVariantName(mLinkedTo.mVariantName);
    mLinkedFrom.mVariant = graph->FindMoveByVariantName(mLinkedFrom.mVariantName);
    for (std::vector<MoveCandidate>::iterator it = mPrevCandidates.begin();
         it != mPrevCandidates.end();
         ++it) {
        it->CacheLinks(graph);
    }
    for (std::vector<MoveCandidate>::iterator it = mNextCandidates.begin();
         it != mNextCandidates.end();
         ++it) {
        it->CacheLinks(graph);
    }
}

void MoveVariant::Load(BinStream &bs, MoveGraph *graph, MoveParent *parent) {
    int rev;
    bs >> rev;
    mMoveParent = parent;
    bs >> mPositionOffset;
    bs >> mVariantName;
    bs >> mHamMoveName;
    bs >> mHamMoveMiloName;
    bs >> mGenre;
    bs >> mEra;
    bs >> mSongName;
    bs >> mAvgBeatsPerSec;
    bs >> mFlags;

    bool isSym;
    bs >> isSym;
    if (isSym) {
        Symbol s;
        bs >> s;
        mLinkedTo.mVariantName = s.Str();
    } else {
        mLinkedTo.mVariant = nullptr;
    }
    if (rev >= 1) {
        bs >> isSym;
        if (isSym) {
            Symbol s;
            bs >> s;
            mLinkedFrom.mVariantName = s.Str();
        } else {
            goto nullset;
        }
    } else {
    nullset:
        mLinkedFrom.mVariant = nullptr;
    }

    int numCandidates;
    bs >> numCandidates;
    mPrevCandidates.resize(numCandidates);
    for (int i = 0; i < numCandidates; i++) {
        mPrevCandidates[i].Load(bs);
    }
    bs >> numCandidates;
    mNextCandidates.resize(numCandidates);
    for (int i = 0; i < numCandidates; i++) {
        mNextCandidates[i].Load(bs);
    }
    graph->mMoveVariants[mSongName] = this;
}
