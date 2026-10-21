#pragma once
#include "utl/Symbol.h"

class PhysMemTypeTracker {
public:
    PhysMemTypeTracker(Symbol type);
    ~PhysMemTypeTracker();

private:
    bool mSet; // 0x0
};

int PhysicalUsage();
void PhysicalFree(void *ptr);
int ForceLinkXMemFuncs();

void *PhysicalAllocTracked(
    unsigned long size, unsigned long flags, const char *file, int line, const char *type
);
void PhysicalFreeTracked(void *ptr, const char *file, int line, const char *type);
