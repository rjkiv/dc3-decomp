#pragma once

void *_MemAlloc(int size, int align);
void *MemAlloc(int size, const char *file, int line, const char *function, int align);
void _MemFree(void *);
void MemFree(void *, const char *file, int line, const char *function);
