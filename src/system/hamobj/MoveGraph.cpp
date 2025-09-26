#include "hamobj/MoveGraph.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "utl/BinStream.h"

MoveGraph::~MoveGraph() {
    mLayoutData = nullptr;
    Clear();
}

BEGIN_HANDLERS(MoveGraph)
    HANDLE_EXPR(has_variant, mMoveVariants.find(_msg->Sym(2)) != mMoveVariants.end())
    HANDLE_EXPR(get_layout_data, mLayoutData)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_LOADS(MoveGraph)
    while (bs.Eof() != NotEof) {
        Timer::Sleep(100);
    }
    LOAD_REVS(bs)
    ASSERT_REVS(0, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    int numParents;
    bs >> numParents;
    for (int i = 0; i < numParents; i++) {
        while (bs.Eof() != NotEof) {
            Timer::Sleep(100);
        }
        MoveParent *parent = new MoveParent();
        parent->Load(bs, this);
        mMoveParents[parent->unk4] = parent;
    }
    CacheLinks();
    mLayoutData->Load(bs);
END_LOADS

void MoveGraph::Clear() {
    for (std::map<Symbol, MoveParent *>::iterator it = mMoveParents.begin();
         it != mMoveParents.end();
         ++it) {
        RELEASE(it->second);
    }
    mMoveParents.clear();
    mMoveVariants.clear();
}

void MoveGraph::ImportMoveData(DataArray *pMoveData) {
    MILO_ASSERT(pMoveData, 0x52);
    for (int i = 0; i < pMoveData->Size(); i++) {
        DataArray *pParentConfig = pMoveData->Array(i);
        MILO_ASSERT(pParentConfig, 0x57);
        MoveParent *pParent = new MoveParent(this, pParentConfig);
        MILO_ASSERT(pParent, 0x5A);
        mMoveParents[pParent->unk4] = pParent;
        for (std::vector<MoveVariant *>::iterator it = pParent->mVariants.begin();
             it != pParent->mVariants.end();
             ++it) {
            mMoveVariants[(*it)->Name()] = *it;
        }
    }
    CacheLinks();
}

void MoveGraph::CacheLinks() {
    for (std::map<Symbol, MoveParent *>::iterator it = mMoveParents.begin();
         it != mMoveParents.end();
         ++it) {
        it->second->CacheLinks(this);
    }
}

MoveVariant *MoveGraph::FindNonConstMoveByVariantName(Symbol name) const {
    std::map<Symbol, MoveVariant *>::const_iterator it = mMoveVariants.find(name);
    if (it != mMoveVariants.end())
        return it->second;
    else
        return nullptr;
}

const MoveVariant *MoveGraph::FindMoveByVariantName(Symbol name) const {
    return FindNonConstMoveByVariantName(name);
}
