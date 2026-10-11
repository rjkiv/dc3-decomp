#include "Instarank.h"
#include "os/Debug.h"
#include "utl/Locale.h"
#include "utl/LocaleOrdinal.h"
#include "utl/Symbol.h"

const char *Instarank::Str() const {
    static Symbol ir_still_rank("ir_still_rank");
    static Symbol ir_at_rank("ir_at_rank");
    static Symbol ir_unranked("ir_unranked");
    static Symbol ir_number_one("ir_number_one");
    static Symbol ir_not_best("ir_not_best");
    static Symbol ir_among_friends("ir_among_friends");
    static Symbol ir_among_all("ir_among_all");

    const char *among_group =
        Localize(mFriendMode ? ir_among_friends : ir_among_all, nullptr, TheLocale);
    const char *ord = LocalizeOrdinal(
        rankArg, LocaleGenderMasculine, LocaleSingular, false, gNullStr, TheLocale
    );
    const char *score = LocalizeSeparatedInt(scoreArg, TheLocale);
    const char *ret;
    if (fmtToken == 'd') {
        ret = MakeString(Localize(ir_unranked, nullptr, TheLocale), strArg, score);
    } else if (fmtToken == 'c') {
        ret = MakeString(
            Localize(ir_number_one, nullptr, TheLocale),
            LocalizeOrdinal(
                1, LocaleGenderMasculine, LocaleSingular, false, gNullStr, TheLocale
            ),
            among_group
        );
    } else if (fmtToken == 'b') {
        ret = MakeString(
            Localize(ir_not_best, nullptr, TheLocale), ord, among_group, strArg, score
        );
    } else if (fmtToken == 'a') {
        ret = MakeString(
            Localize(ir_at_rank, nullptr, TheLocale), ord, among_group, strArg, score
        );
    } else if (fmtToken == 'e') {
        ret = MakeString(
            Localize(ir_still_rank, nullptr, TheLocale), ord, among_group, strArg, score
        );
    } else {
        ret = "";
        MILO_NOTIFY("unrecognized instarank token: %c\n", fmtToken);
    }
    return ret;
}
