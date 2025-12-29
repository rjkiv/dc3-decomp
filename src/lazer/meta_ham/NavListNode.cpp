#include "meta_ham/NavListNode.h"
#include "game/GameMode.h"
#include "meta_ham/HamStarsDisplay.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rndobj/Mat.h"
#include "ui/UILabel.h"
#include "ui/UIListCustom.h"
#include "ui/UIListLabel.h"
#include "ui/UIListMesh.h"
#include "utl/Symbol.h"
#include <cstdio>

#pragma region NavListNode

NavListNode::NavListNode(NavListItemSortCmp *cmp) : mCmp(cmp), mParent(0) {}
NavListNode::~NavListNode() { RELEASE(mCmp); }

int NavListNode::Compare(const NavListNode *n, NavListNodeType t) const {
    return mCmp->Compare(n->mCmp, t);
}

void NavListNode::GetID(DataArray *a) {
    a->Resize(0);
    if (mParent) {
        mParent->GetID(a);
    }
    int size = a->Size();
    a->Resize(size + 1);
    DataNode &n = a->Node(size);
    n = GetToken();
}

#pragma endregion
#pragma region NavListSortNode

NavListSortNode::NavListSortNode(NavListItemSortCmp *cmp)
    : NavListNode(cmp), mShortcut(0), mStartIx(-1) {}

NavListSortNode::~NavListSortNode() { RELEASE(mCmp); }

BEGIN_HANDLERS(NavListSortNode)
    HANDLE_EXPR(album_art_path, GetAlbumArtPath())
    HANDLE_EXPR(get_node_type, GetType())
    HANDLE_EXPR(get_index, mStartIx)
    HANDLE_EXPR(get_token, GetToken())
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

void NavListSortNode::FinishSort(NavListSort *sort) {
    FOREACH (it, mChildren) {
        (*it)->FinishSort(sort);
    }
}

void NavListSortNode::Text(UIListLabel *listLabel, UILabel *label) const {
    label->SetTextToken(gNullStr);
}

void NavListSortNode::Custom(UIListCustom *custom, Hmx::Object *obj) const {
    if (custom->Matches("stars")) {
        HamStarsDisplay *pStarsDisplay = dynamic_cast<HamStarsDisplay *>(obj);
        MILO_ASSERT(pStarsDisplay, 0xec);
        pStarsDisplay->SetShowing(false);
    }
}

void NavListSortNode::DeleteAll() {
    FOREACH (it, mChildren) {
        (*it)->DeleteAll();
        RELEASE(*it);
    }
    mChildren.clear();
}

void NavListSortNode::Renumber(std::vector<NavListSortNode *> &nodes) {
    mStartIx = nodes.size();
    FOREACH (it, mChildren) {
        (*it)->Renumber(nodes);
    }
}

void NavListSortNode::FinishBuildList(NavListSort *sort) {
    FOREACH (it, mChildren) {
        (*it)->FinishBuildList(sort);
    }
}

void NavListSortNode::SetShortcut(NavListShortcutNode *shortcut) { mShortcut = shortcut; }

#pragma endregion
#pragma region NavListShortcutNode

NavListShortcutNode::NavListShortcutNode(
    NavListItemSortCmp *cmp, Symbol token, bool localize
)
    : NavListNode(cmp), mToken(token), mLocalizeToken(localize), mDateTime(nullptr) {
    mParent = nullptr;
}

NavListShortcutNode::~NavListShortcutNode() { delete mDateTime; }

void NavListShortcutNode::DeleteAll() {
    FOREACH (it, mChildren) {
        (*it)->DeleteAll();
        RELEASE(*it);
    }
    mChildren.clear();
}

void NavListShortcutNode::FinishSort(NavListSort *sort) {
    FOREACH (it, mChildren) {
        (*it)->FinishSort(sort);
    }
}

void NavListShortcutNode::FinishBuildList(NavListSort *sort) {
    FOREACH (it, mChildren) {
        (*it)->FinishBuildList(sort);
    }
}

void NavListShortcutNode::Renumber(std::vector<NavListSortNode *> &nodes) {
    FOREACH (it, mChildren) {
        (*it)->Renumber(nodes);
    }
}

bool NavListShortcutNode::IsActive() const {
    FOREACH (it, mChildren) {
        if ((*it)->IsActive())
            return true;
    }
    return false;
}

NavListSortNode *NavListShortcutNode::GetFirstActive() {
    FOREACH (it, mChildren) {
        NavListSortNode *node = (*it)->GetFirstActive();
        if (node)
            return node;
    }
    MILO_FAIL("No available result was found for this shortcut.\n");
    return nullptr;
}

#pragma endregion
#pragma region NavListItemNode

NavListSortNode *NavListItemNode::GetFirstActive() {
    return IsEnabled() ? this : nullptr;
}

BEGIN_HANDLERS(NavListItemNode)
    HANDLE_SUPERCLASS(NavListSortNode)
END_HANDLERS

void NavListItemNode::Text(UIListLabel *listLabel, UILabel *label) const {
    label->SetTextToken(gNullStr);
}

RndMat *NavListItemNode::Mat(UIListMesh *mesh) const {
    if (mesh->Matches("header_back") && !mHeader.Null()) {
        return mesh->DefaultMat();
    } else
        return nullptr;
}

void NavListItemNode::Renumber(std::vector<NavListSortNode *> &nodes) {
    mStartIx = nodes.size();
    nodes.push_back(this);
}

void NavListItemNode::FinishBuildList(NavListSort *) { mHeader = gNullStr; }

bool NavListItemNode::IsHeader() const { return !mHeader.Null(); }
Symbol NavListItemNode::HeaderText() const { return mHeader; }

bool NavListItemNode::UseQuickplayPerformer() {
    static Symbol song_select_quickplay("song_select_quickplay");
    static Symbol song_select_story("song_select_story");
    static Symbol song_select_practice("song_select_practice");
    static Symbol song_select_jukebox("song_select_jukebox");
    Symbol prop = TheGameMode->Property("song_select_mode")->Sym();
    return prop == song_select_quickplay || prop == song_select_story
        || prop == song_select_practice || prop == song_select_jukebox;
}

#pragma endregion
#pragma region NavListFunctionNode

BEGIN_HANDLERS(NavListFunctionNode)
    HANDLE_SUPERCLASS(NavListSortNode)
END_HANDLERS

Symbol NavListFunctionNode::Select() { return 0; }

void NavListFunctionNode::Renumber(std::vector<NavListSortNode *> &nodes) {
    mStartIx = nodes.size();
    nodes.push_back(this);
}

#pragma endregion
#pragma region NavListHeaderNode

NavListHeaderNode::NavListHeaderNode(NavListItemSortCmp *cmp, Symbol token, bool localize)
    : NavListSortNode(cmp), unk44(false), mToken(token), mLocalizeToken(localize),
      mDateTime(nullptr), mCollapseIconLabel(nullptr) {}

NavListHeaderNode::~NavListHeaderNode() { delete mDateTime; }

BEGIN_HANDLERS(NavListHeaderNode)
    HANDLE_SUPERCLASS(NavListSortNode)
END_HANDLERS

Symbol NavListHeaderNode::Select() { return SelectChildren(mChildren, 0); }

bool NavListHeaderNode::IsEnabled() const {
    FOREACH (it, mChildren) {
        if ((*it)->IsEnabled())
            return true;
    }
    return false;
}

void NavListHeaderNode::SetItemCountString(UILabel *label) const {
    char buffer[8] = { '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0' };
    sprintf(buffer, "(%d)", 0);
    Symbol sym(buffer);
    label->SetPrelocalizedString(String(sym));
}

void NavListHeaderNode::SetCollapseStateIcon(bool) const {
    UILabel *label = GetCollapseIconLabel();
    if (label) {
        label->SetTextToken(gNullStr);
    }
}
