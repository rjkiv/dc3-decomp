#pragma once
#include "movie/Movie.h"
#include "obj/Data.h"
#include "obj/DataFile.h"
#include "obj/Object.h"
#include "obj/PropSync.h"
#include "rndobj/Anim.h"
#include "stl/_vector.h"
#include "ui/UILabel.h"
#include "ui/UIPanel.h"

class MoviePanel : public UIPanel {
public:
    // Hmx::Object
    OBJ_CLASSNAME(MoviePanel);
    OBJ_SET_TYPE(MoviePanel);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Load();
    virtual void SetTypeDef(DataArray *);

    // UIPanel
    virtual void Draw();
    virtual void Enter();
    virtual void Exit();
    virtual void Poll();
    virtual bool IsLoaded() const;
    virtual void Unload();
    virtual void FinishLoad();

    static bool sUseSubtitles;
    MoviePanel();
    void ShowMenu(bool);

    bool unk38;
    bool unk39;
    bool unk3a;
    bool unk3b;
    int unk3c;
    const char *mCurrentMovie; // 0x40
    std::vector<const char *> mMovies; // 0x44
    std::list<const char *> mRecent; // 0x50
    Movie unk58;
    DataLoader *unk60;
    DataArray *unk64;
    int unk68;
    bool unk6c;
    UILabel *unk70;
    RndAnimatable *unk74;
    bool unk78;
    float unk7c;
    bool unk80;

protected:
    // UIPanel
    virtual void SetPaused(bool);

    void HideHint();
    void ShowHint();
    void PlayMovie();
    void ChooseMovie();
};
