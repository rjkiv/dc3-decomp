#include "rndobj/Enter.h"
#include "Poll.h"

#include "obj/Object.h"

BEGIN_HANDLERS(RndEnterable)
    HANDLE_SUPERCLASS(RndPollable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS
