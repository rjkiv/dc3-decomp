#include "VorbisMem.h"
#include "utl/Licenses.h"
#include "utl/MemMgr.h"
#include <cstring>

static const char *sVorbisMemName = "Ogg_Internal";
Licenses sLicense("system/src/oggvorbis", Licenses::kRequirementNotification);

void *OggMalloc(int i) { return _MemAllocTemp(i, __FILE__, 0x1C, sVorbisMemName, 0); }

void *OggCalloc(int i1, int i2) {
    void *tmp = _MemAllocTemp(i1 * i2, __FILE__, 0x1C, sVorbisMemName, 0);
    memset(tmp, 0, i1 * i2);
    return tmp;
}

void *OggRealloc(void *v, int i) {
    MemDoTempAllocations tmp;
    return MemRealloc(v, i, __FILE__, 0x2B, sVorbisMemName, 0);
}

void OggFree(void *v) { MemFree(v); }
