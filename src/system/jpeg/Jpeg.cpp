#include "jpeg/Jpeg.h"
#include "jpeg/jpeglib.h"
#include "os/Debug.h"
#include "utl/Licenses.h"

Licenses sLicense("system/src/jpeg", Licenses::kRequirementNotification);

namespace {
    void JpegInitDestination(jpeg_compress_struct *s) {
        jpeg_destination_mgr *dest = s->dest;
        MILO_ASSERT(dest, 0x8b);
        // i have no idea if this is right
        // it's assuming dest is an array of jpeg_destination_mgr structs
        dest->next_output_byte = dest[1].next_output_byte;
        dest->free_in_buffer = dest[1].free_in_buffer;
    }
    unsigned char JpegEmptyOutputBuffer(jpeg_compress_struct *s) {
        MILO_ASSERT(false, 0x94);
        return 0;
    }
    void JpegTermDestination(jpeg_compress_struct *s) {
        jpeg_destination_mgr *dest = s->dest;
        MILO_ASSERT(dest, 0x9c);
        dest[1].free_in_buffer = dest->free_in_buffer;
    }
};

bool LoadBitmapIntoJpeg(char *, int, int, int, void *, int &) {
    jpeg_error_mgr err;
    jpeg_std_error(&err);
    jpeg_compress_struct compress;
    jpeg_CreateCompress(&compress, 0x3E, sizeof(jpeg_compress_struct));

    return true;
}
