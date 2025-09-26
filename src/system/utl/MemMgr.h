#pragma once

extern const char *gStlAllocName;
extern bool gStlAllocNameLookup;

void PhysDelta(const char *);
bool MemUseLowestMip();
int _GetFreePhysicalMemory();
int _GetFreeSystemMemory();
const char *MemHeapName(int);
int MemFindAddrHeap(void *);
int GetCurrentHeapNum();
int MemNumHeaps();
int MemFindHeap(const char *);
void MemPushHeap(int);
void MemPopHeap();
void MemForceNewOperatorAlign(int);

#define kNoHeap -3
#define kSystemHeap -1

void MemPushTemp();
void MemPopTemp();

struct MemTemp {
    MemTemp() { MemPushTemp(); }
    ~MemTemp() { MemPopTemp(); }
};

struct MemHeapTracker {
    MemHeapTracker(int x) { MemPushHeap(x); }
    ~MemHeapTracker() { MemPopHeap(); }
};

void *MemTruncate(
    void *mem,
    int size,
    const char *file = "unknown",
    int line = 0,
    const char *name = "unknown"
);
void *_MemAllocTemp(int, const char *, int, const char *, int);
void *MemRealloc(void *, int, char const *, int, char const *, int);
void *MemAlloc(int size, const char *file, int line, const char *name, int align = 0);
void MemFree(
    void *mem, const char *file = "unknown", int line = 0, const char *name = "unknown"
);
void *MemOrPoolAlloc(int size, const char *file, int line, const char *name);
void *MemOrPoolAllocSTL(int size, const char *file, int line, const char *name);
void MemOrPoolFree(
    int,
    void *mem,
    const char *file = "unknown",
    int line = 0,
    const char *name = "unknown"
);
void MemOrPoolFreeSTL(int, void *mem, const char *file, int line, const char *name);

void *operator new(unsigned int);
void *operator new[](unsigned int);
void operator delete(void *v);

// for Hmx::Objects and their derivatives
#define OBJ_MEM_OVERLOAD(line_num)                                                       \
    static void *operator new(unsigned int s) {                                          \
        return MemAlloc(s, __FILE__, line_num, StaticClassName().Str(), 0);              \
    }                                                                                    \
    static void *operator new(unsigned int s, void *place) { return place; }             \
    static void operator delete(void *v) {                                               \
        MemFree(v, __FILE__, line_num, StaticClassName().Str());                         \
    }

// for everything else
#define MEM_OVERLOAD(class_name, line_num)                                               \
    static void *operator new(unsigned int s) {                                          \
        return MemAlloc(s, __FILE__, line_num, #class_name, 0);                          \
    }                                                                                    \
    static void *operator new(unsigned int s, void *place) { return place; }             \
    static void operator delete(void *v) { MemFree(v, __FILE__, line_num, #class_name); }

// #define NEW_ARRAY_OVERLOAD                                                               \
//     void *operator new[](size_t t) { return _MemAlloc(t, 0); }                           \
//     void *operator new[](size_t, void *place) { return place; }

// #define DELETE_ARRAY_OVERLOAD                                                            \
//     void operator delete[](void *v) { _MemFree(v); }
