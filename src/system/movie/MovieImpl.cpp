#include "movie/MovieImpl.h"
#include "os/Debug.h"
#include "utl/BinStream.h"

bool MovieImpl::BeginFromFile(
    char const *c,
    float f,
    bool b1,
    bool b2,
    bool b3,
    bool b4,
    int i,
    BinStream *stream,
    LoaderPos lp
) {
    if (stream != nullptr && stream->Cached())
        MILO_FAIL("cached load not implemented for stub movie implementation");
    return false;
}

void MovieImpl::Save(BinStream *stream) {
    MILO_ASSERT(stream, 0x30);
    if (stream->Cached()) {
        MILO_FAIL("cached save not implemented for stub movie implementation");
    }
}
