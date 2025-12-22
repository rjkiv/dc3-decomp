#pragma once
#include "obj/Data.h"
#include "obj/Object.h"
#include "stl/_vector.h"
#include "ui/UIListProvider.h"
#include "utl/Symbol.h"

class MovieProvider : public UIListProvider, public Hmx::Object {
public:
    // Hmx::Object
    virtual ~MovieProvider();
    virtual DataNode Handle(DataArray *, bool);

    // UIListProvider
    virtual void Text(int, int, UIListLabel *, UILabel *) const;
    virtual Symbol DataSymbol(int) const;

    MovieProvider();
    void UpdateList();

protected:
    std::vector<Symbol> mMovies; // 0x30
};
