#pragma once
#include "obj/Data.h"
#include "obj/Object.h"
#include "rndobj/Overlay.h"

void CharDeferHighlight(Hmx::Object *o);
void CharInit();
void CharTerminate();

extern float gCharHighlightY;

class CharDebug : public RndOverlay::Callback {
public:
    CharDebug();
    virtual ~CharDebug();

    void Once(Hmx::Object *o);
    void Init();

private:
    virtual float UpdateOverlay(RndOverlay *, float);

    static DataNode OnSetObjects(DataArray *);

    void AddObject(Hmx::Object *o, bool once);
    void SetObjects(DataArray *msg);
    void DisplayObject(Hmx::Object *o);

    ObjPtrList<Hmx::Object> mObjects; // 0x4
    ObjPtrList<Hmx::Object> mOnce; // 0x18
    RndOverlay *mOverlay; // 0x2c
};

extern CharDebug TheCharDebug;
