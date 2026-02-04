#include "hamobj/MoveGraph.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "utl/BinStream.h"
#include "utl/Std.h"
#include "utl/Symbol.h"

MoveGraph::~MoveGraph() {
    mLayoutData = nullptr;
    Clear();
}

BEGIN_HANDLERS(MoveGraph)
    HANDLE_EXPR(has_variant, mMoveVariants.find(_msg->Sym(2)) != mMoveVariants.end())
    HANDLE_EXPR(get_layout_data, mLayoutData)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_COPYS(MoveGraph)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(MoveGraph)
    BEGIN_COPYING_MEMBERS
        *this = *c;
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(0, 0)

BEGIN_LOADS(MoveGraph)
    while (bs.Eof() != NotEof) {
        Timer::Sleep(100);
    }
    LOAD_REVS(bs)
    ASSERT_REVS(0, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    int numParents;
    d >> numParents;
    for (int i = 0; i < numParents; i++) {
        while (bs.Eof() != NotEof) {
            Timer::Sleep(100);
        }
        MoveParent *parent = new MoveParent();
        parent->Load(d.stream, this);
        mMoveParents[parent->Name()] = parent;
    }
    CacheLinks();
    mLayoutData->Load(d.stream);
END_LOADS

MoveGraph &MoveGraph::operator=(const MoveGraph &graph) {
    Clear();
    FOREACH (it, graph.mMoveParents) {
        MoveParent *cur = it->second;
        MoveParent *parent = new MoveParent(cur);
        FOREACH (v, cur->Variants()) {
            MoveVariant *variant = new MoveVariant(this, *v, parent);
            parent->AddVariant(variant);
            parent->AddGenre(variant->Genre());
            parent->AddEra(variant->Era());
            mMoveVariants[variant->Name()] = variant;
        }
        if (parent->Variants().size() != 0) {
            mMoveParents[parent->Name()] = parent;
        } else {
            delete parent;
        }
    }
    CacheLinks();
    mLayoutData = graph.mLayoutData;
    return *this;
}

void MoveGraph::Clear() {
    FOREACH (it, mMoveParents) {
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
        mMoveParents[pParent->Name()] = pParent;
        FOREACH (it, pParent->Variants()) {
            mMoveVariants[(*it)->Name()] = *it;
        }
    }
    CacheLinks();
}

void MoveGraph::CacheLinks() {
    FOREACH (it, mMoveParents) {
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

void MoveGraph::GatherVariants(
    std::vector<const MoveVariant *> *vars, MoveVariantFunc *func, void *v3
) const {
    vars->reserve(mMoveParents.size() * 10);
    FOREACH (it, mMoveParents) {
        MoveParent *curParent = it->second;
        FOREACH (var, curParent->Variants()) {
            if (!func || func(*var, v3)) {
                vars->push_back(*var);
            }
        }
    }
}

bool MoveGraph::HasVariantPair(const MoveParent *p1, const MoveParent *p2) const {
    const MoveVariant *v1;
    const MoveVariant *v2;
    return FindVariantPair(v1, v2, p1, p2, nullptr, nullptr, gNullStr, true);
}

bool MoveGraph::FindVariantPair(
    const MoveVariant *&vref1,
    const MoveVariant *&vref2,
    const MoveParent *p1,
    const MoveParent *p2,
    const MoveVariant *v1,
    const MoveVariant *v2,
    Symbol s,
    bool b8
) const {
    if (p1) {
        auto it = mMoveParents.find(p1->Name());
        if (it == mMoveParents.end()) {
            return false;
        } else {
            p1 = it->second;
        }
    }
    if (p2) {
        auto it = mMoveParents.find(p2->Name());
        if (it == mMoveParents.end()) {
            return false;
        } else {
            p2 = it->second;
        }
    }
    if (v1) {
        auto it = mMoveVariants.find(v1->Name());
        if (it != mMoveVariants.end() && it->second->Parent() != p1) {
            v1 = it->second;
        } else {
            v1 = nullptr;
        }
    }
    if (v2) {
        auto it = mMoveVariants.find(v2->Name());
        if (it != mMoveVariants.end() && it->second->Parent() != p2) {
            v2 = it->second;
        } else {
            v2 = nullptr;
        }
    }
    if (p1 && p2) {
        vref1 = 0;
        vref2 = 0;
    }
    return 0;
}
