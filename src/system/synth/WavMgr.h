#pragma once
#include "obj/Object.h"
#include "utl/ResMgr.h"

typedef void *(*WavMgrAllocFunc)(int, const char *, int, const char *, int);
typedef void (*WavMgrFreeFunc)(void *, const char *, int, const char *);

class WavMgr : public Hmx::Object, public ResMgr<void> {
public:
    WavMgr();
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);

    bool CreateSample(Hmx::CRC, void *&, int);

    static void SetAllocator(WavMgrAllocFunc, WavMgrFreeFunc);

protected:
    virtual void OnReleaseResource(void *);

    static WavMgrAllocFunc sAlloc;
    static WavMgrFreeFunc sFree;
};

extern WavMgr *TheWavMgr;

void InitWavMgr();
