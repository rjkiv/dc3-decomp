#include "ChallengeSort.h"

SortNodeFind::SortNodeFind(const NavListSortNode *node) {
    mToken = node->GetToken();
    mType = node->GetType();
}

bool SortNodeFind::operator()(const NavListSortNode *node) const {
    Symbol token = node->GetToken();
    if (token == mToken && node->GetType() == mType) return true;
    else return false;
}