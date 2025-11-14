#include "char/CharDriver.h"
#include "char/CharClip.h"
#include "macros.h"
#include "obj/Object.h"
#include "utl/Symbol.h"

CharDriver::CharDriver()
    : mBones(this), mClips(this), unk58(0), unk5c(this), unk70(this), unk84(this),
      unk98(false), unka8(1e+30), unkac(false), unkb0(1.0f), unkb4(1.0f), unkbc(0),
      unkc0(0), unkc4(false) {}

CharDriver::~CharDriver() {}
