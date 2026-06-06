#pragma once
#include "MoveGraph.h"
#include "hamobj/MoveGraph.h"
#include "hamobj/SongUtl.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "utl/MemMgr.h"
#include <vector>

class SongPattern {
public:
    SongPattern() : mNumMoves(0) {
        mElements.clear();
        mMoveParents.clear();
    }
    ~SongPattern() {}

    void ClearMoves() {
        mNumMoves = 0;
        FOREACH (it, mMoveParents) {
            *it = nullptr;
        }
    }

    /** "The name of this pattern" */
    Symbol mName; // 0x0
    /** "The measure range when this pattern first appears" */
    Range mInitialMeasureRange; // 0x4
    /** "Pattern elements" */
    std::vector<Symbol> mElements; // 0xc
    std::vector<const MoveParent *> mMoveParents; // 0x18
    int mNumMoves; // 0x24
};

class SongSection {
public:
    /** "Measure start and end" */
    Range mMeasureRange; // 0x0
    /** "Pattern start and end" */
    Range mPatternRange; // 0x8
    /** "The pattern this section is using" */
    Symbol mPattern; // 0x10
    SongPattern *unk14; // 0x14
};

struct MoveReplacer {
    MoveReplacer() : unk0(Symbol()), unk4(Symbol()), unk8(nullptr) {}
    MoveReplacer(Symbol from, Symbol to = Symbol())
        : unk0(from), unk4(to), unk8(nullptr) {}
    void SetToFrom(Symbol to, Symbol from) {
        unk4 = to;
        unk0 = from;
    }
    Symbol unk0; // 0x0 - from
    Symbol unk4; // 0x4 - to
    const MoveParent *unk8; // 0x8
    std::vector<int> mMeasures; // 0xc
};

/** "Song layout" */
class SongLayout : public Hmx::Object {
public:
    // Hmx::Object
    virtual ~SongLayout();
    OBJ_CLASSNAME(SongLayout);
    OBJ_SET_TYPE(SongLayout);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);

    OBJ_MEM_OVERLOAD(0x47)
    NEW_OBJ(SongLayout)

    void SetDefaultPattern(int);
    void SetDefaultReplacer();
    void ClearChosenPatterns();
    void AddPatternMove(int, Symbol);
    void SetReplacerMove(int, Symbol);
    void ClearAllPatternMoves();
    int FirstUnfilledPattern() const;
    int FirstUnfilledReplacer() const;
    int ReplacerFirstMeasure(int) const;
    void DumpPatterns() const;
    bool MeasureInPattern(int, int) const;
    void ClearMoves(int);

    int NumReplacers() const { return mMoveReplacers.size(); }
    const std::vector<SongSection> &SongSections() const { return mSongSections; }

    DataNode GetPatternName(int) const;

protected:
    SongLayout();

    DataNode AddPattern(DataArray *);
    DataNode AddSection(DataArray *);

    /** "Patterns of a song layout section" */
    std::vector<SongPattern> mSongPatterns; // 0x2c
    /** "Sections of a song layout" */
    std::vector<SongSection> mSongSections; // 0x38
    std::vector<MoveReplacer> mMoveReplacers; // 0x44
};
