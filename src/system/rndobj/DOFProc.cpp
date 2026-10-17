#include "rndobj/DOFProc.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "rndobj/Cam.h"

DOFProc *TheDOFProc;

DOFProc::DOFProc() {}
DOFProc::~DOFProc() {}

void DOFProc::Init() {
    DOFProc::Register();
    if (!TheDOFProc) {
        TheDOFProc = Hmx::Object::New<DOFProc>();
        static DataNode &n = DataVariable("the_dof_proc");
        n = TheDOFProc;
    }
}

void DOFProc::Terminate() {
    RELEASE(TheDOFProc);
    static DataNode &n = DataVariable("the_dof_proc");
    n = NULL_OBJ;
}

BEGIN_HANDLERS(DOFProc)
    HANDLE_ACTION(unset, UnSet())
    HANDLE_ACTION(
        set,
        Set(_msg->Obj<RndCam>(2),
            _msg->Float(3),
            _msg->Float(4),
            _msg->Float(5),
            _msg->Float(6))
    )
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS
