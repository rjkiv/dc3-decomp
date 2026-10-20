#include "ui/LocalePanel.h"
#include "obj/Dir.h"
#include "ui/PanelDir.h"
#include "ui/UI.h"
#include "ui/UILabel.h"
#include "ui/UIList.h"
#include "ui/UIListLabel.h"
#include "ui/UIListWidget.h"
#include "utl/Locale.h"
#include "utl/Std.h"

namespace {
    struct LabelSort {
        bool operator()(UILabel *l1, UILabel *l2) {
            return stricmp(l1->UILabel::TextToken().Str(), l2->UILabel::TextToken().Str())
                < 0;
        }
    };
}

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
    std::list<PanelRef> &panels = Screen()->PanelList();
    FOREACH (it, panels) {
        AddDirEntries((ObjectDir *)it->mPanel->LoadedDir(), it->mPanel->Name());
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
        label->SetEditText(entry->mText.c_str());
    }
}

BEGIN_HANDLERS(LocalePanel)
    HANDLE_EXPR(token, mEntries[_msg->Int(2)].mToken)
    HANDLE_EXPR(screen, Screen())
    HANDLE_SUPERCLASS(UIPanel)
END_HANDLERS

void LocalePanel::AddDirEntries(ObjectDir *dir, const char *cc) {
    std::vector<UILabel *> labels;
    for (ObjDirItr<UILabel> it(dir, true); it != nullptr; ++it) {
        if (it->Showing()) {
            labels.push_back(it);
        }
    }
    std::sort(labels.begin(), labels.end(), LabelSort());
    if (!labels.empty()) {
        AddHeading(MakeString("%s: %s", cc ? cc : "proxy", PathName(dir)));
    }
    FOREACH (it, labels) {
        UILabel *cur = *it;
        Entry entry;
        entry.mLabel = cur->Name();
        entry.mToken = TokenForLabel(cur);
        entry.mText = cur->Text();
        mEntries.push_back(entry);
    }
    for (ObjDirItr<UIList> it(dir, true); it != nullptr; ++it) {
        if (it->Showing()) {
            AddHeading(MakeString("%s: %s", it->ClassName(), it->Name()));
            const std::vector<UIListWidget *> &widgets = it->GetWidgets();
            for (int i = 0; i < it->NumDisplay(); i++) {
                FOREACH (w, widgets) {
                    UIListLabel *label = dynamic_cast<UIListLabel *>(*w);
                    if (label) {
                        UILabel *el = label->ElementLabel(i);
                        if (el && !el->Text().empty()) {
                            Entry entry;
                            entry.mLabel = MakeString("%i:%s", i, label->MatchName());
                            entry.mToken = TokenForLabel(el);
                            entry.mText = el->Text();
                            mEntries.push_back(entry);
                        }
                    }
                }
            }
        }
    }
    for (ObjDirItr<PanelDir> it(dir, true); it != nullptr; ++it) {
        if (it != dir) {
            AddDirEntries(it, nullptr);
        }
    }
}
