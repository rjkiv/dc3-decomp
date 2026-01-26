/**
 * @file Compress.h
 * @brief Wrapper functions around zlib.
 */
#pragma once

void *ZAlloc(void *, unsigned int, unsigned int);
void ZFree(void *, void *);
void DecompressMem(const void *, int, void *, int &, const char *);
void CompressMem(const void *, int, void *, int &, const char *);
