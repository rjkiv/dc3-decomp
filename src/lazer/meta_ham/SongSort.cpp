#include "SongSort.h"
#include "meta_ham/NavListNode.h"
#include "ui/UILabel.h"
#include "ui/UIListLabel.h"

SongSort::SongSort() {};

void SongSort::BuildTree() {};

void SongSort::DeleteItemList() {};

void SongSort::BuildItemList() {};

void SongSort::SetHighlightedIx(int i1) {};

void SongSort::SetHighlightItem(const NavListSortNode *node) {};

void SongSort::UpdateHighlight() {};

void SongSort::OnSelectShortcut(int i1) {};

void SongSort::Text(int i1, int i2, UIListLabel *listlabel, UILabel *uilabel) const {};

Symbol SongSort::DetermineHeaderSymbolFromSong(Symbol sym) { return sym; };
