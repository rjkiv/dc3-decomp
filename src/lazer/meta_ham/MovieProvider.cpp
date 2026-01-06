#include "meta_ham/MovieProvider.h"
#include "meta_ham/ProfileMgr.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "ui/UILabel.h"
#include "ui/UIListLabel.h"
#include "utl/Symbol.h"

MovieProvider::~MovieProvider() {}

void MovieProvider::Text(
    int, int i_iData, UIListLabel *uiListLabel, UILabel *uiLabel
) const {
    MILO_ASSERT(i_iData < NumData(), 0x48);
    Symbol dataSym = DataSymbol(i_iData);
    if (uiListLabel->Matches("label")) {
        uiLabel->SetTextToken(dataSym);
    } else {
        uiLabel->SetTextToken(uiListLabel->GetDefaultText());
    }
}

Symbol MovieProvider::DataSymbol(int i_iData) const {
    MILO_ASSERT_RANGE(i_iData, 0, NumData(), 0x5e);
    return mMovies[i_iData];
}

void MovieProvider::UpdateList() {
    mMovies.clear();
    static Symbol movie_intro("movie_intro");
    static Symbol campaign_intro("campaign_intro");
    static Symbol era01("era01");
    static Symbol era02("era02");
    static Symbol era03("era03");
    static Symbol era04("era04");
    static Symbol era05("era05");
    static Symbol era06("era06");
    static Symbol era01_intro("era01_intro");
    static Symbol era03_intro("era03_intro");
    static Symbol era04_intro("era04_intro");
    static Symbol era02_intro("era02_intro");
    static Symbol era05_intro("era05_intro");
    static Symbol tan_intro("tan_intro");
    static Symbol tan_outro("tan_outro");
    static Symbol credits("credits");
    mMovies.push_back(movie_intro);
    if (TheProfileMgr.HasAnyEraSongBeenPlayed(era01))
        mMovies.push_back(era01_intro);
    if (TheProfileMgr.HasAnyEraSongBeenPlayed(era02))
        mMovies.push_back(era02_intro);
    if (TheProfileMgr.HasAnyEraSongBeenPlayed(era03))
        mMovies.push_back(era03_intro);
    if (TheProfileMgr.HasAnyEraSongBeenPlayed(era04))
        mMovies.push_back(era04_intro);
    mMovies.push_back(credits);
}

BEGIN_HANDLERS(MovieProvider)
    HANDLE_ACTION(update_list, UpdateList())
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS
