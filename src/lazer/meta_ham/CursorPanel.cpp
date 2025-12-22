#include "meta_ham/CursorPanel.h"
#include "flow/PropertyEventProvider.h"
#include "meta_ham/PassiveMessagesPanel.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "utl/Symbol.h"

CursorPanel::CursorPanel() {}

CursorPanel::~CursorPanel() {}

void CursorPanel::Poll() {
    PassiveMessagesPanel::Poll();
    static Symbol ui_crown_player("ui_crown_player");
    const DataNode *pCrownPlayerNode = TheHamProvider->Property(ui_crown_player, true);
    MILO_ASSERT(pCrownPlayerNode, 0x1f);
}

BEGIN_HANDLERS(CursorPanel)
    HANDLE_SUPERCLASS(PassiveMessagesPanel)
END_HANDLERS
