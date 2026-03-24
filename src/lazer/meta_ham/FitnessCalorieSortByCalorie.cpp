#include "meta_ham/FitnessCalorieSortByCalorie.h"
#include "meta_ham/FitnessCalorieSortMgr.h"
#include "meta_ham/FitnessCalorieSortNode.h"
#include "meta_ham/NavListNode.h"
#include "utl/MakeString.h"
#include "utl/Symbol.h"

int FitnessCalorieSortCmp::Compare(
    NavListItemSortCmp const *cmp, NavListNodeType type
) const {
    return type - kNodeItem ? 0 : -1;
}

NavListShortcutNode *
FitnessCalorieSortByCalorie::NewShortcutNode(NavListItemNode *node) const {
    Symbol s =
        MakeString("calorie_shortcut_%i", ((FitnessCalorieSortNode *)node)->GetUnk48());
    Symbol sym = s;
    FitnessCalorieSortCmp *cmp = new FitnessCalorieSortCmp();
    return new NavListShortcutNode(cmp, sym, true);
}

NavListHeaderNode *
FitnessCalorieSortByCalorie::NewHeaderNode(NavListItemNode *node) const {
    Symbol s =
        MakeString("calorie_header_%i", ((FitnessCalorieSortNode *)node)->GetUnk48());
    Symbol sym = s;
    FitnessCalorieSortCmp *cmp = new FitnessCalorieSortCmp();
    return new FitnessCalorieHeaderNode(cmp, sym, true);
}

NavListItemNode *FitnessCalorieSortByCalorie::NewItemNode(void *p1) const {
    int *i = static_cast<int *>(p1);
    FitnessCalorieSortCmp *cmp = new FitnessCalorieSortCmp();
    return new FitnessCalorieSortNode(cmp, *i);
}
