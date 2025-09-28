#pragma once
#include "hamobj/Difficulty.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "utl/BinStream.h"
#include <map>

class MoveParent;
class MoveVariant;

typedef bool MoveVariantFunc(const MoveVariant *, void *);

class MoveGraph : public Hmx::Object {
    friend class MoveVariant;

public:
    MoveGraph() {}
    // Hmx::Object
    virtual ~MoveGraph();
    OBJ_CLASSNAME(MoveGraph)
    OBJ_SET_TYPE(MoveGraph)
    virtual DataNode Handle(DataArray *, bool);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);

    void Clear();
    void CacheLinks();
    MoveVariant *FindNonConstMoveByVariantName(Symbol) const;
    const MoveVariant *FindMoveByVariantName(Symbol) const;
    bool FindVariantPair(
        const MoveVariant *&,
        const MoveVariant *&,
        const MoveParent *,
        const MoveParent *,
        const MoveVariant *,
        const MoveVariant *,
        Symbol,
        bool
    ) const;
    bool HasVariantPair(const MoveParent *, const MoveParent *) const;
    void
    GatherVariants(std::vector<const MoveVariant *> *, MoveVariantFunc *, void *) const;
    void ImportMoveData(DataArray *);
    MoveGraph &operator=(const MoveGraph &);
    const std::map<Symbol, MoveParent *> &MoveParents() const { return mMoveParents; }
    const std::map<Symbol, MoveVariant *> &MoveVariants() const { return mMoveVariants; }

private:
    std::map<Symbol, MoveParent *> mMoveParents; // 0x2c
    std::map<Symbol, MoveVariant *> mMoveVariants; // 0x44
    DataArrayPtr mLayoutData; // 0x5c
};

class MoveParent {
    friend class MoveGraph;

public:
    MoveParent();
    MoveParent(const MoveParent *);
    MoveParent(MoveGraph *, DataArray *);
    virtual ~MoveParent();

    void CacheLinks(MoveGraph *);
    void Load(BinStream &, MoveGraph *);
    bool IsValidForMiniGame() const;
    const MoveVariant *PickRandomVariant() const;
    bool HasFinalMoveVariant() const;
    bool HasRestMoveVariant() const;
    bool HasPrevAdjacent(const MoveParent *) const;
    bool HasGenre(Symbol) const;
    bool HasEra(Symbol) const;
    bool HasCategory(Symbol) const;
    Difficulty GetDifficulty() const { return mDifficulty; }

private:
    void PopulateAdjacentParents();

    Symbol unk4; // 0x4
    Difficulty mDifficulty; // 0x8
    bool unkc; // 0xc
    std::vector<MoveVariant *> mVariants; // 0x10
    std::vector<Symbol> mGenreFlags; // 0x1c
    std::vector<Symbol> mEraFlags; // 0x28
    std::vector<const MoveParent *> unk34; // 0x34
    std::vector<const MoveParent *> mPrevAdjacents; // 0x40
};

union MoveVariantValue {
    const MoveVariant *mVariant;
    const char *mVariantName;
};

struct MoveCandidate {
    MoveCandidate() {
        mValue.mVariant = nullptr;
        mAdjacencyFlag = 0;
    }
    MoveCandidate(Symbol, Symbol, Symbol, bool);
    MoveCandidate(const MoveCandidate &c);

    void CacheLinks(MoveGraph *);
    void Load(BinStream &);

    static unsigned int Adjacency(Symbol);

    MoveVariantValue mValue; // 0x0
    unsigned int mAdjacencyFlag; // 0x4
};

class MoveVariant {
    friend class MoveCandidate;

public:
    MoveVariant() {}
    MoveVariant(MoveGraph *, DataArray *, MoveParent *);
    MoveVariant(MoveGraph *, const MoveVariant *, MoveParent *);
    ~MoveVariant();

    bool IsValidForMinigame() const;
    Difficulty GetDifficulty() const;
    bool IsRest() const;
    void CacheLinks(MoveGraph *);
    void Load(BinStream &, MoveGraph *, MoveParent *);
    Symbol Name() const { return mVariantName; }
    Symbol HamMoveName() const { return mHamMoveName; }
    Symbol Genre() const { return mGenre; }
    bool IsFinalPose() const { return mFlags & 8; }
    MoveParent *Parent() const { return mMoveParent; }

private:
    Vector3 mPositionOffset; // 0x0
    Symbol mVariantName; // 0x10
    MoveParent *mMoveParent; // 0x14
    std::vector<MoveCandidate> mPrevCandidates; // 0x18
    std::vector<MoveCandidate> mNextCandidates; // 0x24
    Symbol mHamMoveName; // 0x30
    Symbol mHamMoveMiloName; // 0x34
    MoveVariantValue mLinkedTo; // 0x38
    MoveVariantValue mLinkedFrom; // 0x3c
    Symbol mGenre; // 0x40
    Symbol mEra; // 0x44
    Symbol mSongName; // 0x48
    float mAvgBeatsPerSec; // 0x4c
    unsigned int mFlags; // 0x50
    // mFlags flags:
    // & 2 = scored
    // & 8 = final_pose
    // & 0x10 = suppress_guide_gesture
    // & 0x20 = omit_minigame
    // & 0x40 = useful
    // & 0x80 = suppress_practice_options
};
