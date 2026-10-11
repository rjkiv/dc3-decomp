#include "MQSongSortByCharacter.h"

#include "hamobj/HamGameData.h"
#include "HamSongMgr.h"
#include "MQSongSortNode.h"
#include "meta_ham/NavListNode.h"
#include "utl/Symbol.h"

int MQSongCharCmp::Compare(const NavListItemSortCmp *cmp, NavListNodeType type) const {
    const MQSongCharCmp *mqCmp;
    switch (type) {
    case kNodeShortcut:
        return 0;
        break;
    case kNodeHeader:
        mqCmp = cmp->GetMQSongCharCmp();
        return strcmp(mCharacter.Str(), mqCmp->mCharacter.Str());
        break;
    case kNodeItem:
        mqCmp = cmp->GetMQSongCharCmp();
        return strcmp(mSong.Str(), mqCmp->mSong.Str());
        break;
    default:
        MILO_FAIL("invalid type of node comparison.\n");
        return 0;
        break;
    }
}

NavListHeaderNode *MQSongSortByCharacter::NewHeaderNode(NavListItemNode *node) const {
    auto cmp = node->GetCmp()->GetMQSongCharCmp();
    Symbol p1 = cmp->mSong;
    Symbol p2 = cmp->mCharacter;
    MQSongCharCmp *songCharCmp = new MQSongCharCmp(p1, p2);
    Symbol c = p2;
    Symbol sym(MakeString("mqheader_%s", c));
    return new MQSongHeaderNode(songCharCmp, sym, true);
}

NavListShortcutNode *MQSongSortByCharacter::NewShortcutNode(NavListItemNode *node) const {
    Symbol p1 = node->GetCmp()->GetMQSongCharCmp()->mSong;
    Symbol p2 = node->GetCmp()->GetMQSongCharCmp()->mCharacter;
    MQSongCharCmp *songCharCmp = new MQSongCharCmp(p1, p2);
    return new NavListShortcutNode(songCharCmp, p1, true);
}

NavListItemNode *MQSongSortByCharacter::NewItemNode(void *node) const {
    Symbol *pSongSymbol = static_cast<Symbol *>(node);
    Symbol outfitChar = GetOutfitCharacter(
        TheHamSongMgr.Data(TheHamSongMgr.GetSongIDFromShortName(*pSongSymbol))->Outfit(),
        true
    );
    MQSongCharCmp *songCharCmp = new MQSongCharCmp(*pSongSymbol, outfitChar);
    return new MQSongSortNode(songCharCmp, *pSongSymbol, outfitChar);
}
