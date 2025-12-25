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
    virtual void SetTypeDef(DataArray *);

    // UIPanel
    virtual void Load();
    virtual void Draw();
    virtual void Enter();
    virtual void Exit();
    virtual void Poll();
    virtual bool IsLoaded() const;
    virtual void Unload();
    virtual void FinishLoad();

    NEW_OBJ(MoviePanel)

    static bool sUseSubtitles;
    MoviePanel();
    void ShowMenu(bool);

protected:
    // UIPanel
    virtual void SetPaused(bool);

    void HideHint();
    void ShowHint();
    void PlayMovie();
    void ChooseMovie();

    bool mPreload; // 0x38
    bool mAudio; // 0x39
    bool mLoop; // 0x3a
    bool mFillWidth; // 0x3b
    int mLanguage; // 0x3c
    const char *mCurrentMovie; // 0x40
    std::vector<const char *> mMovies; // 0x44
    std::list<const char *> mRecent; // 0x50
    Movie mMovie; // 0x58
    DataLoader *mSubtitlesLoader; // 0x60
    DataArray *mSubtitles; // 0x64
    int mCurrentSubtitleIndex; // 0x68
    bool mSubtitleCleared; // 0x6c
    UILabel *mSubtitleLabel; // 0x70
    RndAnimatable *mPauseHintAnim; // 0x74
    bool mShowHint; // 0x78
    float mTimeShowHintStarted; // 0x7c
    bool mShowMenu; // 0x80
};
