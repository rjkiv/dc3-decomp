#include "ui/LocalePanel.h"
#include "ui/UI.h"
#include "ui/UIListLabel.h"
#include "utl/Locale.h"
#include "utl/Std.h"

LocalePanel::LocalePanel() {}

int LocalePanel::NumData() const { return mEntries.size(); }

bool LocalePanel::IsActive(int i) const { return !mEntries[i].mLabel.empty(); }

float LocalePanel::GapSize(int, int i, int, int) const {
    if (IsActive(i))
        return 30.0f;
    else
        return 0;
}

UIScreen *LocalePanel::Screen() {
    int i = TheUI->PushDepth();
    return TheUI->ScreenAtDepth(i - 1);
}

Symbol LocalePanel::TokenForLabel(UILabel *label) {
    if (label->TextToken().Null()) {
        return "<no token>";
    } else {
        bool b18 = false;
        Localize(label->TextToken(), &b18, TheLocale);
        if (b18)
            return label->TextToken();
        else
            return "<token not found>";
    }
}

void LocalePanel::AddHeading(const char *cc) {
    Entry entry;
    entry.mHeading = cc;
    mEntries.push_back(entry);
}

void LocalePanel::Enter() {
    mEntries.clear();
    UIScreen *screen = Screen();
    FOREACH (it, screen->PanelList()) {
        AddDirEntries((ObjectDir *)it->mPanel->GetPanelDir(), it->mPanel->Name());
    }
    UIPanel::Enter();
}

void LocalePanel::Text(int i, int j, UIListLabel *listlabel, UILabel *label) const {
    Entry *entry = (Entry *)&mEntries[j];
    if (listlabel->Matches("heading")) {
        label->SetEditText(entry->mHeading.c_str());
    } else if (listlabel->Matches("label")) {
        label->SetEditText(entry->mLabel.c_str());
    } else if (listlabel->Matches("token")) {
        label->SetEditText(entry->mToken.Str());
    } else if (listlabel->Matches("string")) {
        label->SetEditText(entry->mString.c_str());
    }
}

LocalePanel::Entry::Entry() {}

BEGIN_HANDLERS(LocalePanel)
    HANDLE_EXPR(token, mEntries[_msg->Int(2)].mToken)
    HANDLE_EXPR(screen, Screen())
    HANDLE_SUPERCLASS(UIPanel)
END_HANDLERS
