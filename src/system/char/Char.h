#pragma once
#include "obj/Data.h"
#include "obj/Object.h"
#include "rndobj/Overlay.h"

void CharDeferHighlight(Hmx::Object *);
void CharInit();
void CharTerminate();

extern float gCharHighlightY;

class CharDebug : public RndOverlay::Callback {
public:
    CharDebug();
    virtual ~CharDebug();

    void Once(Hmx::Object *);
    void Init();

private:
    virtual float UpdateOverlay(RndOverlay *, float);

    static DataNode OnSetObjects(DataArray *);

    void AddObject(Hmx::Object *, bool);
    void SetObjects(DataArray *);
    void DisplayObject(Hmx::Object *);

    ObjPtrList<Hmx::Object> mObjects; // 0x4
    ObjPtrList<Hmx::Object> mOnce; // 0x18
    RndOverlay *mOverlay; // 0x2c
};

extern CharDebug TheCharDebug;
