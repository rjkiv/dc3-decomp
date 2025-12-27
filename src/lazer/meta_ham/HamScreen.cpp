#include "HamScreen.h"
#include "HamUI.h"
#include "ui/UI.h"

BEGIN_CUSTOM_HANDLERS(HamScreen)
//HANDLE_ACTION_IF(get)
HANDLE_SUPERCLASS(UIScreen)
END_CUSTOM_HANDLERS

void HamScreen::Enter(UIScreen *screen) {
    TheHamUI.InitPanels();
    if (TheHamUI.GetHelpBarPanel()->GetState() != UIPanel::kUp) {
        TheHamUI.GetHelpBarPanel()->Enter();
    }
    if (TheHamUI.GetLetterboxPanel() && TheHamUI.GetLetterboxPanel()->GetState() != UIPanel::kUp) {
        TheHamUI.GetLetterboxPanel()->Enter();
    }
    if (TheHamUI.GetBlacklightPanel() != 0 && TheHamUI.GetBlacklightPanel()->GetState() != UIPanel::kUp) {
        TheHamUI.GetBlacklightPanel()->Enter();
    }
    TheHamUI.GetShellInput()->UpdateInputPanel(mFocusPanel);
    UIScreen::Enter(screen);
}

void HamScreen::Exit(UIScreen *screen) {
    UIScreen::Exit(screen);
}

bool HamScreen::Exiting() const {
    return UIScreen::Exiting();
}

bool HamScreen::IsEventDialogOnTop() const {
    UIPanel *event_dialog = TheHamUI.EventDialogPanel();
    MILO_ASSERT(event_dialog, 0x5f);

    if (TheUI->CurrentScreen() != this || event_dialog->GetState() != UIPanel::kUp) {
        return false;
    }
    return true;
}

bool HamScreen::InComponentSelect() const {
    if (IsEventDialogOnTop()) {
        UIComponent *component = TheHamUI.EventDialogPanel()->FocusComponent();
        if (component) {
            return component->GetState() == UIComponent::kSelecting;
        }
    }
    UIComponent *component = TheHamUI.GetOverlayPanel()->FocusComponent();
    if (component) {
        return component->GetState() == UIComponent::kSelecting;
    }
    return UIScreen::InComponentSelect();
}