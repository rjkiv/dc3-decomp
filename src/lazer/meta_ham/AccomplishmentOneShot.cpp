#include "lazer/meta_ham/AccomplishmentOneShot.h"
#include "AccomplishmentOneShot.h"
#include "hamobj/Difficulty.h"
#include "hamobj/HamPlayerData.h"
#include "lazer/meta_ham/AccomplishmentConditional.h"
#include "obj/Data.h"
#include "os/Debug.h"
#include "utl/Symbol.h"

AccomplishmentOneShot::AccomplishmentOneShot(DataArray *d, int i)
    : AccomplishmentConditional(d, i) {
    Configure(d);
}

AccomplishmentOneShot::~AccomplishmentOneShot() {}

bool AccomplishmentOneShot::AreOneShotConditionsMet(
    HamPlayerData *, HamProfile *, Symbol, Difficulty
) {
    static Symbol stars("stars");
    static Symbol flawless_a("flawless_a");
    static Symbol flawless_b("flawless_b");
    static Symbol nices_a("nices_a");
    static Symbol nices_b("nices_b");
    static Symbol days("days");
    static Symbol weekends("weekends");
    static Symbol hardest_stars("hardest_stars");

    return false;
}

void AccomplishmentOneShot::Configure(DataArray *i_pConfig) {
    MILO_ASSERT(i_pConfig, 0x23);
}
