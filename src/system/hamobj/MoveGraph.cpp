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
    HANDLE_EXPR(has_variant, mVariantsByName.find(_msg->Sym(2)) != mVariantsByName.end())
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
        parent->Load(d, this);
        mNodes[parent->GetName()] = parent;
    }
    CacheLinks();
    mLayoutData->Load(d);
END_LOADS

MoveGraph &MoveGraph::operator=(const MoveGraph &graph) {
    Clear();
    FOREACH (it, graph.mNodes) {
        MoveParent *cur = it->second;
        MoveParent *parent = new MoveParent(cur);
        FOREACH (v, cur->mMoveVariants) {
            MoveVariant *variant = new MoveVariant(this, *v, parent);
            parent->mMoveVariants.push_back(variant);
            if (!parent->HasGenre(variant->mGenre)) {
                parent->mGenreFlags.push_back(variant->mGenre);
            }
            if (!parent->HasEra(variant->mEra)) {
                parent->mEraFlags.push_back(variant->mEra);
            }

            mVariantsByName[variant->GetName()] = variant;
        }
        if (parent->mMoveVariants.size() != 0) {
            mNodes[parent->GetName()] = parent;
        } else {
            delete parent;
        }
    }
    CacheLinks();
    mLayoutData = graph.mLayoutData;
    return *this;
}

void MoveGraph::Clear() {
    FOREACH (it, mNodes) {
        RELEASE(it->second);
    }
    mNodes.clear();
    mVariantsByName.clear();
}

void MoveGraph::ImportMoveData(DataArray *pMoveData) {
    MILO_ASSERT(pMoveData, 0x52);
    for (int i = 0; i < pMoveData->Size(); i++) {
        DataArray *pParentConfig = pMoveData->Array(i);
        MILO_ASSERT(pParentConfig, 0x57);
        MoveParent *pParent = new MoveParent(this, pParentConfig);
        MILO_ASSERT(pParent, 0x5A);
        mNodes[pParent->GetName()] = pParent;
        FOREACH (it, pParent->mMoveVariants) {
            mVariantsByName[(*it)->GetName()] = *it;
        }
    }
    CacheLinks();
}

void MoveGraph::CacheLinks() {
    FOREACH (it, mNodes) {
        it->second->CacheLinks(this);
    }
}

MoveVariant *MoveGraph::FindNonConstMoveByVariantName(Symbol name) const {
    std::map<Symbol, MoveVariant *>::const_iterator it = mVariantsByName.find(name);
    if (it != mVariantsByName.end())
        return it->second;
    else
        return nullptr;
}

const MoveVariant *MoveGraph::FindMoveByVariantName(Symbol name) const {
    return FindNonConstMoveByVariantName(name);
}

void MoveGraph::GatherVariants(
    std::vector<const MoveVariant *> *result, MoveVariantFunc *filter, void *userData
) const {
    result->reserve(mNodes.size() * 10);
    FOREACH (it, mNodes) {
        MoveParent *curParent = it->second;
        FOREACH (var, curParent->mMoveVariants) {
            if (!filter || filter(*var, userData)) {
                result->push_back(*var);
            }
        }
    }
}

bool MoveGraph::HasVariantPair(const MoveParent *p0, const MoveParent *p1) const {
    const MoveVariant *v1;
    const MoveVariant *v2;
    return FindVariantPair(v1, v2, p0, p1, nullptr, nullptr, gNullStr, true);
}

MoveParent *MoveGraph::GetNonConstMoveParent(Symbol s) const {
    auto it = mNodes.find(s);
    if (it != mNodes.end()) {
        return it->second;
    } else {
        return nullptr;
    }
}

bool MoveGraph::FindVariantPair(
    const MoveVariant *&res0,
    const MoveVariant *&res1,
    const MoveParent *p0,
    const MoveParent *p1,
    const MoveVariant *pref0,
    const MoveVariant *pref1,
    Symbol s,
    bool b8
) const {
    if (p0) {
        auto it = mNodes.find(p0->GetName());
        if (it == mNodes.end()) {
            return false;
        } else {
            p0 = it->second;
        }
    }
    if (p1) {
        auto it = mNodes.find(p1->GetName());
        if (it == mNodes.end()) {
            return false;
        } else {
            p1 = it->second;
        }
    }
    if (pref0) {
        auto it = mVariantsByName.find(pref0->GetName());
        if (it != mVariantsByName.end() && it->second->GetParent() == p0) {
            pref0 = it->second;
        } else {
            pref0 = nullptr;
        }
    }
    if (pref1) {
        auto it = mVariantsByName.find(pref1->GetName());
        if (it != mVariantsByName.end() && it->second->GetParent() == p1) {
            pref1 = it->second;
        } else {
            pref1 = nullptr;
        }
    }
    if (p0 && p1) {
        int u6 = 0;
        FOREACH (v, p0->mMoveVariants) {
            MoveVariant *var1 = *v;
            FOREACH (c, var1->mNextCandidates) {
                const MoveVariant *var2 = c->mVariant;
                int u9;
                if (var2->GetParent() == p1) {
                    u9 = 0x200;
                    if (var2->GetSongName() == s) {
                        u9 = 0x220;
                    }
                    if (var1->GetSongName() == s) {
                        u9 |= 0x40;
                    }
                    if (var2 == pref1) {
                        u9 |= 0x80;
                    }
                    if (var1 == pref0) {
                        u9 |= 0x100;
                    }
                    if (c->HasTransition()) {
                        u9 |= 4;
                    }
                    int u4 = c->Adjacency();
                    if (u4 == 4) {
                        u9 |= 0x10;
                    }
                    if (u4 == 8) {
                        u9 |= 8;
                    }
                    if (u4 == 0x10) {
                        u9 |= 1;
                    }
                    if (u4 == 0x20) {
                        u9 |= 2;
                    }
                    int tmp = u6;
                    if (u6 < u9) {
                        u6 = u9;
                        if (u9 != tmp) {
                            res0 = var1;
                            res1 = var2;
                            if (b8) {
                                return true;
                            }
                        }
                    }
                }
            }
        }
        return u6;
    } else if (p0) {
        auto &vars = p0->mMoveVariants;
        if (!vars.empty()) {
            if (pref0) {
                res0 = pref0;
            } else {
                res0 = p0->mMoveVariants.front();
                if (!s.Null()) {
                    for (int i = 0; i < p0->mMoveVariants.size(); i++) {
                        if (p0->mMoveVariants[i]->GetSongName() == s) {
                            res0 = p0->mMoveVariants[i];
                            break;
                        }
                    }
                }
            }
            return true;
        }
    } else if (p1) {
        auto &vars = p1->mMoveVariants;
        if (!vars.empty()) {
            if (pref1) {
                res1 = pref1;
            } else {
                res1 = p1->mMoveVariants.front();
                if (!s.Null()) {
                    for (int i = 0; i < p1->mMoveVariants.size(); i++) {
                        if (p1->mMoveVariants[i]->GetSongName() == s) {
                            res1 = p1->mMoveVariants[i];
                            break;
                        }
                    }
                }
            }
            return true;
        }
    }
    return false;
}
