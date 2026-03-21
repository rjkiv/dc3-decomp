#include "meta_ham/HamStoreOffer.h"
#include "hamobj/Difficulty.h"
#include "meta/SongMgr.h"
#include "meta/Sorting.h"
#include "meta/StoreOffer.h"
#include "obj/Data.h"
#include "os/DateTime.h"
#include "os/Debug.h"
#include "utl/Symbol.h"

HamStoreOffer::HamStoreOffer(DataArray *d, SongMgr *s) : StoreOffer(d, s) {
    static Symbol preview("preview");
    if (HasData(preview)) {
        mPreviewPath = "previews/";
        Symbol p = preview;
        DataArray *previewArray = mStoreOfferData->FindArray(p);
        mPreviewPath += previewArray->Str(1);
    } else {
        mPreviewPath = gNullStr;
    }

    static Symbol art("art");
    mAlbumArtPath = "album_art/";
    Symbol a = art;
    DataArray *albumArtArray = mStoreOfferData->FindArray(a);
    mAlbumArtPath += albumArtArray->Str(1);
}

HamStoreOffer::~HamStoreOffer() {}

int HamStoreOffer::Difficulty() const {
    static Symbol difficulty("difficulty");
    Symbol s = difficulty;
    DataArray *diffArray = mStoreOfferData->FindArray(s);
    return diffArray->Int(1);
}

bool HamStoreOffer::Cmp(StoreOffer const &rhs, Symbol s) const {
    const HamStoreOffer &other = dynamic_cast<const HamStoreOffer &>(rhs);
    static Symbol by_title("by_title");
    static Symbol by_artist("by_artist");
    static Symbol by_difficulty("by_difficulty");
    static Symbol by_release("by_release");
    static Symbol song("song");
    static Symbol pack("pack");
    static Symbol avatar("avatar");
    if (s == by_title) {
        return AlphaKeyStrCmp(OfferName(), other.OfferName(), true) < 0;
    } else if (s == by_artist) {
        MILO_ASSERT(OfferType() == song, 0x49);
        MILO_ASSERT(other.OfferType() == song, 0x4a);
        int cmp = AlphaKeyStrCmp(ArtistName(), other.ArtistName(), true);
        if (cmp == 0) {
            return Cmp(other, by_title);
        }
        return cmp < 0;
    } else if (s == by_difficulty) {
        MILO_ASSERT(OfferType() == song, 0x53);
        MILO_ASSERT(other.OfferType() == song, 0x54);
        return Difficulty() < other.Difficulty();
    } else if (s == by_release) {
        int cmp = DateTimeCmp(ReleaseDate(), other.ReleaseDate());
        if (cmp == 0) {
            return Cmp(other, by_title);
        }
        return cmp > 0;
    } else {
        MILO_FAIL("Unknown sort type: %s", s);
        return false;
    }
}

BEGIN_HANDLERS(HamStoreOffer)
    HANDLE_EXPR(difficulty, Difficulty())
    HANDLE_EXPR(art_path, mAlbumArtPath.c_str())
    HANDLE_EXPR(preview_path, mPreviewPath.c_str())
    HANDLE_SUPERCLASS(StoreOffer)
END_HANDLERS
