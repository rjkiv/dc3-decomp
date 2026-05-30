#pragma once
#include "utl/Symbol.h"

class PhysMemTypeTracker {
public:
    PhysMemTypeTracker(Symbol);
    ~PhysMemTypeTracker();

private:
    bool unk0;
};

int PhysicalUsage();
void PhysicalFree(void *);
int ForceLinkXMemFuncs();

void *PhysicalAllocTracked(unsigned long, unsigned long, const char *, int, const char *);
void PhysicalFreeTracked(void *, const char *, int, const char *);
