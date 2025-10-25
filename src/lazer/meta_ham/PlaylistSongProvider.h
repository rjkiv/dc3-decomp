#pragma once
#include "Playlist.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "ui/UILabel.h"
#include "ui/UIListLabel.h"
#include "ui/UIListProvider.h"
#include "utl/Symbol.h"

class PlaylistSongProvider : public UIListProvider, public Hmx::Object {
public:
    // UIListProvider
    virtual DataNode Handle(DataArray *, bool);
    virtual ~PlaylistSongProvider();
    virtual void Text(int, int, UIListLabel *, UILabel *) const;
    virtual Symbol DataSymbol(int) const;
    virtual int NumData() const;

    PlaylistSongProvider();
    void UpdateList(Playlist const *, bool);

    const Playlist *unk30;
    bool unk34;
};
