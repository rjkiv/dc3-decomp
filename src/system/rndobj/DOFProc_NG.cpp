#include "rndobj/DOFProc_NG.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rndobj/DOFProc.h"
#include "rndobj/Rnd.h"
#include "rndobj/Rnd_NG.h"
#include "rndobj/Tex.h"

NgDOFProc::NgDOFProc()
    : mEnabled(0), unk34(1), unk38(0), mFocalPlane(1), mBlurDepth(1), mMinBlur(0),
      mMaxBlur(1) {
    TheRnd.RegisterPostProcessor(this);
    MILO_ASSERT(TheNgRnd.PreProcessTexture(), 0x41);
    int w = TheNgRnd.PreProcessTexture()->Width() >> 2;
    int h = TheNgRnd.PreProcessTexture()->Height() >> 2;
    mBlurTex[0] = Hmx::Object::New<RndTex>();
    mBlurTex[0]->SetBitmap(w, h, TheRnd.Bpp(), RndTex::kRenderedNoZ, false, nullptr);
    mBlurTex[1] = mBlurTex[0];
}

NgDOFProc::~NgDOFProc() {
    RELEASE(mBlurTex[0]);
    TheRnd.UnregisterPostProcessor(this);
}

void NgDOFProc::Init() {
    REGISTER_OBJ_FACTORY(NgDOFProc);
    if (TheDOFProc && !dynamic_cast<NgDOFProc *>(TheDOFProc)) {
        RELEASE(TheDOFProc);
        TheDOFProc = Hmx::Object::New<DOFProc>();
        MILO_ASSERT(dynamic_cast< NgDOFProc* >(TheDOFProc) != NULL, 0x175);
        static DataNode &n = DataVariable("the_dof_proc");
        n = TheDOFProc;
    }
}

void NgDOFProc::Terminate() {
    RELEASE(TheDOFProc);
    static DataNode &n = DataVariable("the_dof_proc");
    n = NULL_OBJ;
}
