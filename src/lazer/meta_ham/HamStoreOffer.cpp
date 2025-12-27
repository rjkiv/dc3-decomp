#include "meta_ham/HamStoreOffer.h"
#include "meta/SongMgr.h"
#include "meta/StoreOffer.h"
#include "obj/Data.h"
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

BEGIN_HANDLERS(HamStoreOffer)
    HANDLE_EXPR(difficulty, Difficulty())
    HANDLE_EXPR(art_path, mAlbumArtPath.c_str())
    HANDLE_EXPR(preview_path, mPreviewPath.c_str())
    HANDLE_SUPERCLASS(StoreOffer)
END_HANDLERS
