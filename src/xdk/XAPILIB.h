#pragma once
// grouped together .h's inferred from official MSVC documentation
// create/add to these when you need a function that's shared with Windows
#include "xapilibi/debugapi.h"
#include "xapilibi/errhandlingapi.h"
#include "xapilibi/fileapi.h"
#include "xapilibi/handleapi.h"
#include "xapilibi/ioapiset.h"
#include "xapilibi/processenv.h"
#include "xapilibi/processthreadsapi.h"
#include "xapilibi/stringapiset.h"
#include "xapilibi/synchapi.h"
#include "xapilibi/sysinfoapi.h"
#include "xapilibi/timezoneapi.h"
#include "xapilibi/winbase.h"
#include "xapilibi/winerror.h"
#include "xapilibi/xinput.h"
// inferred .h for Xbox 360 specific functions
// condensed into one single .h to avoid dependency hell
#include "xapilibi/xbox.h"

// Don't try to include any of the above headers directly.
// Instead, if you need something from XAPILIB, just include "xdk/XAPILIB.h"
// That way, you don't have to worry about which XAPILIB header to include
// for your particular use case.
