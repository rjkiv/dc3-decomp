#include "meta_ham/TitleProvider.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rndobj/Mat.h"
#include "ui/UILabel.h"
#include "ui/UIListLabel.h"
#include "ui/UIListMesh.h"
#include "utl/Symbol.h"

TitleProvider::~TitleProvider() {}

Symbol TitleProvider::DataSymbol(int i_iData) const {
    MILO_ASSERT_RANGE(i_iData, 0, NumData(), 0x44);
    return mItems[i_iData];
}

void TitleProvider::UpdateList(bool) {
    mItems.clear();
    static Symbol title_screen_menu("title_screen_menu");
    static Symbol start_the_party("start_the_party");
    mItems.push_back(title_screen_menu);
    mItems.push_back(start_the_party);
}

void TitleProvider::Text(
    int, int i_iData, UIListLabel *uiListLabel, UILabel *uiLabel
) const {
    MILO_ASSERT(i_iData < NumData(), 0x1f);
    Symbol dataSymbol = DataSymbol(i_iData);
    if (uiListLabel->Matches("label")) {
        uiLabel->SetTextToken(dataSymbol);
    } else {
        uiLabel->SetTextToken(uiListLabel->GetDefaultText());
    }
}

RndMat *TitleProvider::Mat(int, int i_iData, UIListMesh *) const {
    MILO_ASSERT_RANGE(i_iData, 0, NumData(), 0x2e);
    static Symbol start_the_party("start_the_party");
    return nullptr;
}

BEGIN_HANDLERS(TitleProvider)
    HANDLE_ACTION(update_list, UpdateList(_msg->Int(2)))
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS
