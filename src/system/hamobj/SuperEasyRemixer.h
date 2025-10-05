#pragma once
#include "hamobj/OriginalChoreoRemixer.h"
#include "obj/Object.h"
#include "utl/MemMgr.h"

class SuperEasyRemixer : public OriginalChoreoRemixer {
public:
    // Hmx::Object
    OBJ_CLASSNAME(OriginalChoreoRemixer);
    OBJ_SET_TYPE(OriginalChoreoRemixer);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // OriginalChoreoRemixer
    virtual void Reset();
    virtual void Init();

    OBJ_MEM_OVERLOAD(0x14)
    NEW_OBJ(SuperEasyRemixer)

    void SendDowngradeDatapoint(Symbol, int, Symbol, int, int, int, Symbol);
    static void LoadAllVariants();

protected:
    SuperEasyRemixer();

    virtual std::vector<const MoveParent *> &GetMoveParentsByDifficulty(int);
    virtual std::vector<const MoveVariant *> &GetMoveVariantsByDifficulty(int);

    void SaveSuperEasyMoveParents();
    void DumpSongLayout();

    std::vector<const MoveParent *> mSuperEasyParents; // 0x10c
    std::vector<const MoveVariant *> mSuperEasyVariants; // 0x118
    bool unk124; // 0x124
};
