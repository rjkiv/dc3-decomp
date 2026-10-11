#pragma once
#include "obj/Data.h"
#include "obj/Object.h"
#include "ui/UIListProvider.h"
#include "utl/Symbol.h"

class MovieProvider : public UIListProvider, public Hmx::Object {
public:
    // Hmx::Object
    virtual ~MovieProvider();
    OBJ_CLASSNAME(MovieProvider)
    OBJ_SET_TYPE(MovieProvider)
    virtual DataNode Handle(DataArray *, bool);
    // UIListProvider
    virtual void Text(int, int, UIListLabel *, UILabel *) const;
    virtual Symbol DataSymbol(int) const;
    virtual int NumData() const { return m_vMovies.size(); }

    void UpdateList();

    NEW_OBJ(MovieProvider)

private:
    std::vector<Symbol> m_vMovies; // 0x30
};
