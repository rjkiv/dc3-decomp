#include "jpeg/Jpeg.h"
#include "jpeg/jpeglib.h"
#include "os/Debug.h"
#include "utl/Licenses.h"

Licenses sLicense("system/src/jpeg", Licenses::kRequirementNotification);

namespace {
    void JpegInitDestination(jpeg_compress_struct *s) {
        jpeg_destination_mgr *dest = s->dest;
        MILO_ASSERT(dest, 0x8b);
        dest->next_output_byte = (JOCTET *)dest->buffer;
        dest->free_in_buffer = dest->in_bytes;
    }
    unsigned char JpegEmptyOutputBuffer(jpeg_compress_struct *s) {
        MILO_ASSERT(false, 0x94);
        return 0;
    }
    void JpegTermDestination(jpeg_compress_struct *s) {
        jpeg_destination_mgr *dest = s->dest;
        MILO_ASSERT(dest, 0x9c);
        dest->out_bytes = dest->in_bytes - dest->free_in_buffer;
    }
};

bool LoadBitmapIntoJpeg(char *pixels, int w, int h, int bpp, void *buffer, int &bytes) {
    jpeg_error_mgr err;
    jpeg_compress_struct compress;
    compress.err = jpeg_std_error(&err);
    jpeg_CreateCompress(&compress, 0x3E, sizeof(jpeg_compress_struct));
    jpeg_destination_mgr dest_mgr;
    memset(&dest_mgr, 0, sizeof(jpeg_destination_mgr));
    dest_mgr.buffer = buffer;
    dest_mgr.init_destination = JpegInitDestination;
    dest_mgr.empty_output_buffer = JpegEmptyOutputBuffer;
    dest_mgr.term_destination = JpegTermDestination;
    dest_mgr.in_bytes = bytes;
    compress.dest = &dest_mgr;
    compress.in_color_space = JCS_RGB;
    compress.image_height = h;
    compress.image_width = w;
    compress.input_components = bpp;
    jpeg_set_defaults(&compress);
    jpeg_start_compress(&compress, true);
    int i1 = compress.input_components * w;
    while (compress.next_scanline < compress.image_height) {
        unsigned char *ptr = (unsigned char *)pixels + compress.next_scanline * i1;
        jpeg_write_scanlines(&compress, (JSAMPARRAY)&ptr, 1);
    }
    jpeg_finish_compress(&compress);
    bytes = dest_mgr.out_bytes;
    return true;
}
