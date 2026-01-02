#pragma once
#include "NavListNode.h"
#include "NavListSort.h"

class SortNodeFind {
public:
    SortNodeFind(const NavListSortNode *);
    bool operator()(const NavListSortNode *) const;

protected:
    Symbol mToken; // 0x0
    NavListNodeType mType; // 0x4
};

class ChallengeSort : public NavListSort {};
