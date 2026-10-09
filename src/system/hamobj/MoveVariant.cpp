#include "MoveGraph.h"
#include "hamobj/Difficulty.h"
#include "hamobj/MoveGraph.h"
#include "obj/Data.h"
#include "os/Debug.h"
#include "utl/BinStream.h"
#include "utl/Symbol.h"

#pragma region MoveCandidate

MoveCandidate::MoveCandidate(Symbol s1, Symbol, Symbol s3, bool b4) : mAdjacencyFlag(0) {
    if (b4)
        mAdjacencyFlag = kMC_HasTransition;
    mAdjacencyFlag |= Adjacency(s3);
    mVariantName = s1.Str();
}

MoveCandidate::MoveCandidate(const MoveCandidate &c) {
    if (c.mAdjacencyFlag & kMC_IsCached) {
        mVariantName = c.mVariant->m_Name.Str();
    } else
        mVariant = c.mVariant;
    mAdjacencyFlag = c.mAdjacencyFlag & ~kMC_IsCached;
}

void MoveCandidate::CacheLinks(MoveGraph *graph) {
    const char *variantName =
        mAdjacencyFlag & kMC_IsCached ? mVariant->m_Name.Str() : mVariantName;
    mVariant = graph->FindNonConstMoveByVariantName(variantName);
    if (!mVariant) {
        MILO_FAIL("Could not find link to %s", variantName);
    }
    mAdjacencyFlag |= kMC_IsCached;
}

void MoveCandidate::Load(BinStream &bs) {
    int rev;
    bs >> rev;
    bs >> mAdjacencyFlag;
    Symbol s1, s2, s3;
    bs >> s1;
    bs >> s2;
    bs >> s3;
    mVariantName = s2.Str();
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
        return kMC_Adjacency_Original;
    else if (s == easy_med_adjacent)
        return kMC_Adjacency_EasyMed;
    else if (s == auto_score)
        return kMC_Adjacency_AutoScore;
    else if (s == anim_hot_or_not)
        return kMC_Adjacency_AnimHotOrNot;
    else if (s == fake)
        return kMC_Adjacency_Fake;
    else {
        MILO_NOTIFY("MoveCandidate has unknown source '%s'", s.Str());
        return kMC_Adjacency_Fake;
    }
}

#pragma endregion
#pragma region MoveVariant

MoveVariant::MoveVariant(MoveGraph *graph, const MoveVariant *other, MoveParent *parent) {
    m_Name = other->m_Name;
    mParent = parent;
    mPrevCandidates = other->mPrevCandidates;
    mNextCandidates = other->mNextCandidates;
    mSongName = other->mSongName;
    m_fAbps = other->m_fAbps;
    m_HamMoveName = other->m_HamMoveName;
    m_HamMoveMiloName = other->m_HamMoveMiloName;
    mGenre = other->mGenre;
    mEra = other->mEra;
    mFlags = other->mFlags & ~1;
    const char *toName;
    if (other->mLinkedTo) {
        const MoveVariant *to = other->mLinkedTo;
        toName = to->m_Name.Str();
    } else {
        toName = nullptr;
    }
    mLinkedToName = toName;
    const char *fromName;
    if (other->mLinkedFrom) {
        const MoveVariant *from = other->mLinkedFrom;
        fromName = from->m_Name.Str();
    } else {
        fromName = nullptr;
    }
    mLinkedFromName = fromName;
    mDelta = other->mDelta;
    graph->mVariantsByName[m_Name] = this;
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
    m_Name = cfg->Sym(0);
    mParent = parent;
    mSongName = cfg->FindArray(song_name)->Sym(1);
    m_fAbps = cfg->FindArray(average_beats_per_second)->Float(1);
    mGenre = cfg->FindArray(genre)->Sym(1);
    mEra = cfg->FindArray(era)->Sym(1);
    mFlags = 0;
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
        mLinkedToName = cfg->FindArray(linked_to)->Sym(1).Str();
    else
        mLinkedTo = nullptr;
    if (cfg->FindArray(linked_from, false))
        mLinkedFromName = cfg->FindArray(linked_from)->Sym(1).Str();
    else
        mLinkedFrom = nullptr;

    if (cfg->FindArray(position_offset, false)) {
        mDelta.x = cfg->FindArray(position_offset)->Float(1);
        mDelta.y = cfg->FindArray(position_offset)->Float(2);
        mDelta.z = cfg->FindArray(position_offset)->Float(3);
    } else {
        mDelta.Zero();
    }
    m_HamMoveName = cfg->FindArray(ham_move_name)->Sym(1);
    m_HamMoveMiloName = cfg->FindArray(ham_move_milo_name)->Sym(1);
    mPrevCandidates.clear();
    mNextCandidates.clear();

    DataArray *prevCandsArr = cfg->FindArray(prev_candidates, true);
    for (int i = 1; i < prevCandsArr->Size(); i++) {
        Symbol s1;
        if (prevCandsArr->Array(i)->Type(0) == kDataInt) {
            s1 = MakeString("%i", prevCandsArr->Array(i)->Int(0));
        } else {
            s1 = prevCandsArr->Array(i)->Sym(0);
        }
        Symbol s2;
        if (prevCandsArr->Array(i)->Type(1) == kDataInt) {
            s2 = MakeString("%i", prevCandsArr->Array(i)->Int(1));
        } else {
            s2 = prevCandsArr->Array(i)->Sym(1);
        }
        bool b3 = prevCandsArr->Array(i)->Int(2);
        Symbol s3;
        if (prevCandsArr->Array(i)->Size() > 3) {
            s3 = prevCandsArr->Array(i)->Sym(3);
        } else {
            s3 = original_adjacent;
        }
        mPrevCandidates.push_back(MoveCandidate(s1, s2, s3, b3));
    }
    DataArray *nextCandsArr = cfg->FindArray(next_candidates, true);
    for (int i = 1; i < nextCandsArr->Size(); i++) {
        Symbol s1;
        if (nextCandsArr->Array(i)->Type(0) == kDataInt) {
            s1 = MakeString("%i", nextCandsArr->Array(i)->Int(0));
        } else {
            s1 = nextCandsArr->Array(i)->Sym(0);
        }
        Symbol s2;
        if (nextCandsArr->Array(i)->Type(1) == kDataInt) {
            s2 = MakeString("%i", nextCandsArr->Array(i)->Int(1));
        } else {
            s2 = nextCandsArr->Array(i)->Sym(1);
        }
        bool b3 = nextCandsArr->Array(i)->Int(2);
        Symbol s3;
        if (nextCandsArr->Array(i)->Size() > 3) {
            s3 = nextCandsArr->Array(i)->Sym(3);
        } else {
            s3 = original_adjacent;
        }
        mNextCandidates.push_back(MoveCandidate(s1, s2, s3, b3));
    }

    graph->mVariantsByName[m_Name] = this;
}

MoveVariant::~MoveVariant() {
    mPrevCandidates.clear();
    mNextCandidates.clear();
}

Difficulty MoveVariant::GetDifficulty() const { return mParent->GetDifficulty(); }

bool MoveVariant::IsRest() const {
    static Symbol Rest("Rest.move");
    static Symbol rest("rest.move");
    static Symbol groove("groove");
    if (m_HamMoveName == Rest || m_HamMoveName == rest || m_HamMoveName == groove)
        return true;

    String move = m_HamMoveName.Str();
    if (move.contains("finish")) {
        return true;
    }
    return false;
}

void MoveVariant::CacheLinks(MoveGraph *graph) {
    mLinkedTo = graph->FindMoveByVariantName(mLinkedToName);
    mLinkedFrom = graph->FindMoveByVariantName(mLinkedFromName);
    FOREACH (it, mPrevCandidates) {
        it->CacheLinks(graph);
    }
    FOREACH (it, mNextCandidates) {
        it->CacheLinks(graph);
    }
}

void MoveVariant::Load(BinStream &bs, MoveGraph *graph, MoveParent *parent) {
    int rev;
    bs >> rev;
    mParent = parent;
    bs >> mDelta;
    bs >> m_Name;
    bs >> m_HamMoveName;
    bs >> m_HamMoveMiloName;
    bs >> mGenre;
    bs >> mEra;
    bs >> mSongName;
    bs >> m_fAbps;
    bs >> mFlags;

    bool isSym;
    bs >> isSym;
    if (isSym) {
        Symbol s;
        bs >> s;
        mLinkedToName = s.Str();
    } else {
        mLinkedTo = nullptr;
    }
    if (rev >= 1) {
        bs >> isSym;
        if (!(isSym)) {
            mLinkedFrom = nullptr;
        } else {
            Symbol s;
            bs >> s;
            mLinkedFromName = s.Str();
        }
    } else {
        mLinkedFrom = nullptr;
    }

    unsigned int numCandidates;
    bs >> numCandidates;
    auto &prevCandidates = mPrevCandidates;
    prevCandidates.resize(numCandidates);
    for (int i = 0; i < numCandidates; i++) {
        mPrevCandidates[i].Load(bs);
    }
    bs >> numCandidates;
    auto &nextCandidates = mNextCandidates;
    nextCandidates.resize(numCandidates);
    for (int i = 0; i < numCandidates; i++) {
        nextCandidates[i].Load(bs);
    }
    graph->mVariantsByName[m_Name] = this;
}

bool MoveVariant::IsValidForMinigame() const {
    if (IsRest()) {
        return false;
    }
    if ((int)mNextCandidates.size() < 1) {
        return false;
    }
    if (IsOmitedFromMinigames())
        return false;
    return !IsFinalPose();
}
