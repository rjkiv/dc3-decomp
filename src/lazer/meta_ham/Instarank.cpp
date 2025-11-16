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
        Localize(unk_0x4 ? ir_among_friends : ir_among_all, nullptr, TheLocale);
    const char *ord = LocalizeOrdinal(
        unk_0x108, LocaleGenderMasculine, LocaleSingular, false, gNullStr, TheLocale
    );
    const char *score = LocalizeSeparatedInt(unk_0x10C, TheLocale);
    char token = unk_0x5;
    if (token == 100) {
        return MakeString(Localize(ir_unranked, nullptr, TheLocale), next_person, score);
    } else if (token == 99) {
        const char *ord = LocalizeOrdinal(
            1, LocaleGenderMasculine, LocaleSingular, false, gNullStr, TheLocale
        );
        return MakeString(Localize(ir_number_one, nullptr, TheLocale), ord, among_group);

    } else if (token == 98) {
        return MakeString(
            Localize(ir_not_best, nullptr, TheLocale), ord, among_group, next_person, score
        );
    } else if (token == 97) {
        return MakeString(
            Localize(ir_at_rank, nullptr, TheLocale), ord, among_group, next_person, score
        );
    } else if (token == 101) {
        return MakeString(
            Localize(ir_still_rank, nullptr, TheLocale),
            ord,
            among_group,
            next_person,
            score
        );
    }
    MILO_NOTIFY("unrecognized instarank token: %c\n", token);
    return "\n";
}
