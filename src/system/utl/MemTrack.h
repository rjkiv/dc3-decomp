#pragma once

void MemTrackReportMemoryAlloc(const char *);
void MemTrackReportMemoryUsage(const char *);
void MemTrackReportClose(const char *);

void *DebugHeapAlloc(int);
void DebugHeapFree(void *);

void BeginMemTrackFileName(const char *);
void EndMemTrackFileName();

void BeginMemTrackObjectName(const char *);
void EndMemTrackObjectName();

void MemTrackInit(int, int, bool);