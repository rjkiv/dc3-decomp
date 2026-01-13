#pragma once
#include "movie/Movie.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "obj/PropSync.h"
#include "rndobj/Draw.h"
#include "rndobj/Poll.h"
#include "rndobj/Tex.h"
#include "utl/BinStream.h"
#include "utl/FilePath.h"
#include "utl/Loader.h"

class TexMovie : public RndDrawable, public RndPollable {
public:
    // Hmx::Object
    virtual ~TexMovie();
    virtual void Copy(Hmx::Object const *, Hmx::Object::CopyType);
    virtual bool Replace(ObjRef *, Hmx::Object *);
    OBJ_CLASSNAME(TexMovie);
    OBJ_SET_TYPE(TexMovie);
    OBJ_MEM_OVERLOAD(0x18);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Load(BinStream &);

    // RndDrawable
    virtual void DrawPreClear();
    virtual void UpdatePreClearState();

    // RndPollable
    virtual void Poll();
    virtual void Enter();
    virtual void Exit();

    void SetPaused(bool);
    void Reset();
    bool IsEmpty() const;
    void DrawToTexture();
    void SetFile(FilePath const &);
    NEW_OBJ(TexMovie);
    static void Init() { REGISTER_OBJ_FACTORY(TexMovie); }

    void SetVolume(float vol) { mMovie.SetVolume(vol); }
    void AddFader(Fader *f) { mMovie.Faders()->Add(f); }
    bool IsOpen() const { return mMovie.IsOpen(); }
    Movie &GetMovie() { return mMovie; }

protected:
    ObjOwnerPtr<RndTex> mTex; // 0x48 ObjOwnerPtr | 0x54, RndTex
    bool unk5c;
    bool unk5d;
    bool unk5e;
    bool unk5f;
    FilePath sRoot;
    Movie mMovie; // 0x68

    TexMovie();
    void DoBeginMovieFromFile(BinStream *, LoaderPos);
    DataNode OnPlayMovie(DataArray *);
    DataNode OnGetRenderTextures(DataArray *);
};
