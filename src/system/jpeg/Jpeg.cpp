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

bool LoadBitmapIntoJpeg(char *pixels, int w, int h, int bpp, void *buffer, int &bytes) {
    jpeg_error_mgr err;
    jpeg_compress_struct compress;
    compress.err = jpeg_std_error(&err);
    jpeg_CreateCompress(&compress, 0x3E, sizeof(jpeg_compress_struct));
    jpeg_destination_mgr dest_mgr;
    memset(&dest_mgr, 0, sizeof(jpeg_destination_mgr));
    dest_mgr.next_output_byte = 0;
    dest_mgr.free_in_buffer = 0;
    dest_mgr.init_destination = JpegInitDestination;
    dest_mgr.empty_output_buffer = JpegEmptyOutputBuffer;
    dest_mgr.term_destination = JpegTermDestination;
    compress.dest = &dest_mgr;
    compress.in_color_space = JCS_RGB;
    compress.image_width = w;
    compress.image_height = h;
    compress.input_components = bpp;
    jpeg_set_defaults(&compress);
    jpeg_start_compress(&compress, true);
    int i1 = compress.input_components * w;
    while (compress.next_scanline < compress.image_height) {
        unsigned char *ptr = (unsigned char *)pixels + compress.next_scanline * i1;
        jpeg_write_scanlines(&compress, (JSAMPARRAY)&ptr, 1);
    }
    jpeg_finish_compress(&compress);
    return true;
}
