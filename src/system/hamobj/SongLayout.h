#pragma once
#include "MoveGraph.h"
#include "hamobj/SongUtl.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "utl/MemMgr.h"

class SongPattern {
    friend bool
    PropSync(SongPattern &o, DataNode &_val, DataArray *_prop, int _i, PropOp _op);

public:
    SongPattern() : unk24(0) {
        mElements.clear();
        mMoveParents.clear();
    }
    ~SongPattern() {}

private:
    /** "The name of this pattern" */
    Symbol mName; // 0x0
    /** "The measure range when this pattern first appears" */
    Range mInitialMeasureRange; // 0x4
    /** "Pattern elements" */
    std::vector<Symbol> mElements; // 0xc
    std::vector<const MoveParent *> mMoveParents; // 0x18
    int unk24; // 0x24
};

class SongSection {
    friend bool
    PropSync(SongSection &o, DataNode &_val, DataArray *_prop, int _i, PropOp _op);

public:
private:
    /** "Measure start and end" */
    Range mMeasureRange; // 0x0
    /** "Pattern start and end" */
    Range mPatternRange; // 0x8
    /** "The pattern this section is using" */
    Symbol mPattern; // 0x10
    int unk14;
};

struct MoveReplacer {
    int unk0;
    int unk4;
    int unk8;
    std::vector<int> unkc;
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
    int NumReplacers() const { return mMoveReplacers.size(); }

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
