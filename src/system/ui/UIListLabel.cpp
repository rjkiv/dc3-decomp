#include "ui/UIListLabel.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "ui/UILabel.h"

UIListLabel::UIListLabel() : mLabel(this), unk8c(0) {}

BEGIN_PROPSYNCS(UIListLabel)
    SYNC_PROP(label, mLabel)
    SYNC_PROP(highlight_alt_styles, unk8c)
    SYNC_SUPERCLASS(UIListSlot)
END_PROPSYNCS

BEGIN_SAVES(UIListLabel)
END_SAVES

BEGIN_COPYS(UIListLabel)

END_COPYS

BEGIN_LOADS(UIListLabel)

END_LOADS

const char *UIListLabel::GetDefaultText() const { return gNullStr; }

UILabel *UIListLabel::ElementLabel(int) const { return nullptr; }

UIListSlotElement *UIListLabel::CreateElement(UIList *uilist) { return nullptr; }
