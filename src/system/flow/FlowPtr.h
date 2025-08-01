#pragma once

#include "obj/Object.h"
#include "utl/BinStream.h"

class FlowPtrBase {
protected:
    bool RefreshParamObject(void);
    Hmx::Object *GetObject(void);
    Hmx::Object *LoadObject(BinStream &);

public:
    int GetInitialState(Hmx::Object *);
};

ObjectDir *FlowPtrGetLoadingDir(ObjectDir *);

template <typename T>
class FlowPtr {
    T *Get(void);

public:
    FlowPtr(T *, T *) {}
};

template <typename T>
BinStream &operator<<(BinStream &, const FlowPtr<T> &);
