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
private:
    MoveParent *GetNonConstMoveParent(Symbol) const;

public:
    MoveGraph() { mNodes.clear(); }
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
    const MoveParent *GetMoveParent(Symbol s) const { return GetNonConstMoveParent(s); }
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

    NEW_OBJ(MoveGraph);

    std::map<Symbol, MoveParent *> mNodes; // 0x2c
    std::map<Symbol, MoveVariant *> mVariantsByName; // 0x44
    DataArrayPtr mLayoutData; // 0x5c
};

class MoveParent {
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
    Symbol GetName() const { return mName; }
    void SetSuperEasy(bool b) { mSuperEasy = b; }

private:
    void PopulateAdjacentParents();

    Symbol mName; // 0x4
    Difficulty mDifficulty; // 0x8
    bool mSuperEasy; // 0xc
public:
    std::vector<MoveVariant *> mMoveVariants; // 0x10
    std::vector<Symbol> mGenreFlags; // 0x1c
    std::vector<Symbol> mEraFlags; // 0x28
    std::vector<const MoveParent *> mNext; // 0x34
    std::vector<const MoveParent *> mPrev; // 0x40
};

struct MoveCandidate {
    enum Flags {
        kMC_IsCached = 1,
        kMC_HasTransition = 2,
        kMC_Adjacency_Fake = 0,
        kMC_Adjacency_Original = 4,
        kMC_Adjacency_EasyMed = 8,
        kMC_Adjacency_AutoScore = 0x10,
        kMC_Adjacency_AnimHotOrNot = 0x20,
        kMC_Adjacency_Mask = 0x3c,
    };
    MoveCandidate() {
        mVariant = nullptr;
        mAdjacencyFlag = 0;
    }
    MoveCandidate(Symbol, Symbol, Symbol, bool);
    MoveCandidate(const MoveCandidate &c);

    void CacheLinks(MoveGraph *);
    void Load(BinStream &);
    unsigned int HasTransition() const { return mAdjacencyFlag & kMC_HasTransition; }
    unsigned int IsCached() const { return mAdjacencyFlag & kMC_IsCached; }
    unsigned int Adjacency() const { return mAdjacencyFlag & kMC_Adjacency_Mask; }

    static unsigned int Adjacency(Symbol);

    union {
        const MoveVariant *mVariant;
        const char *mVariantName;
    }; // 0x0
private:
    unsigned int mAdjacencyFlag; // 0x4
};

class MoveVariant {
    enum Flags {
        kMV_IsCached = 0,
        kMV_IsScored = 1,
        kMV_IsParadiddle = 2,
        kMV_IsFinalPose = 3,
        kMV_SuppressGuideGesture = 4,
        kMV_OmitMinigame = 5,
        kMV_IsUseful = 6,
        kMV_SuppressPracticeOptions = 7,
    };

public:
    MoveVariant() {}
    MoveVariant(MoveGraph *, DataArray *, MoveParent *);
    MoveVariant(MoveGraph *, const MoveVariant *, MoveParent *);
    ~MoveVariant();

    // int32_t NumPrevCandidates() const;
    // int32_t NumNextCandidates() const;
    // const MoveCandidate* GetPrevCandidate(int32_t) const;
    // const MoveCandidate* GetNextCandidate(int32_t) const;
    // const MoveVariant* GetLinkedTo() const;
    // const MoveVariant* GetLinkedFrom() const;
    // bool IsScored() const;
    // bool IsFinalPose() const;
    // bool IsOmitedFromMinigames() const;
    // bool SuppressGuideGesture() const;
    // bool SuppressPracticeOptions() const;
    // bool IsUseful() const;
    // void SetUseful(bool);
    // bool HasNext(const MoveVariant*) const;
    // bool HasCategory(Symbol) const;
    // bool IsFirstLink() const;
    // const MoveCandidate* FindNext(const MoveVariant*) const;
    // MoveCandidate* FindNext(const MoveVariant*);
    // const MoveCandidate* FindPrev(const MoveVariant*) const;
    // MoveCandidate* FindPrev(const MoveVariant*);
    // bool HasPrevCandidate(const MoveVariant*) const;
    // bool HasNextCandidate(const MoveVariant*) const;

    bool IsValidForMinigame() const;
    Difficulty GetDifficulty() const;
    bool IsRest() const;
    void CacheLinks(MoveGraph *);
    void Load(BinStream &, MoveGraph *, MoveParent *);
    Symbol GetName() const { return m_Name; }
    Symbol GetHamMoveName() const { return m_HamMoveName; }
    bool IsFinalPose() const { return mFlags & (1 << kMV_IsFinalPose); }
    bool IsOmitedFromMinigames() const { return mFlags & (1 << kMV_OmitMinigame); }
    MoveParent *GetParent() const { return mParent; }
    Symbol GetSongName() const { return mSongName; }

    // every use of this so far has just been (0,0,0)
    Vector3 mDelta; // 0x0
    Symbol m_Name; // 0x10
    MoveParent *mParent; // 0x14
    std::vector<MoveCandidate> mPrevCandidates; // 0x18
    std::vector<MoveCandidate> mNextCandidates; // 0x24
    Symbol m_HamMoveName; // 0x30
    Symbol m_HamMoveMiloName; // 0x34
    union {
        const MoveVariant *mLinkedTo;
        const char *mLinkedToName;
    }; // 0x38
    union {
        const MoveVariant *mLinkedFrom;
        const char *mLinkedFromName;
    }; // 0x3c
    Symbol mGenre; // 0x40
    Symbol mEra; // 0x44
    Symbol mSongName; // 0x48
    float m_fAbps; // 0x4c - average beats per sec

private:
    unsigned int mFlags; // 0x50
};
