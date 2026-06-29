/**
 * @file Compress.h
 * @brief Wrapper functions around zlib.
 */
#pragma once

void DecompressMem(
    const void *in, int in_len, void *out, int &out_len, const char *filename
);
void CompressMem(
    const void *in, int in_len, void *out, int &out_len, const char *filename
);
