#include "lazer/meta_ham/HamPanel.h"
#include "gesture/Skeleton.h"
#include "obj/Object.h"
#include "ui/UIComponent.h"
#include "ui/UIPanel.h"

HamPanel::HamPanel() : unk38(0) {}

void HamPanel::Enter() {}

bool HamPanel::Exiting() const { return false; }

void HamPanel::Poll() { UIPanel::Poll(); }

UIComponent *HamPanel::FocusComponent() { return nullptr; }

BEGIN_HANDLERS(HamPanel)
    HANDLE_SUPERCLASS(UIPanel)
END_HANDLERS
