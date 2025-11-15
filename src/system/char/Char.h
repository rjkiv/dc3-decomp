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
    virtual ~CharDebug();

    CharDebug();
    void Once(Hmx::Object *);
    void Init();

    ObjPtrList<Hmx::Object> mObjects; // 0x4
    ObjPtrList<Hmx::Object> mOnce; // 0x18
    RndOverlay *mOverlay; // 0x2c

private:
    virtual float UpdateOverlay(RndOverlay *, float);

    void AddObject(Hmx::Object *, bool);
    void SetObjects(DataArray *);
    static DataNode OnSetObjects(DataArray *);
    void DisplayObject(Hmx::Object *);
};
