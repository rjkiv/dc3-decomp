#pragma once
#include "os/Debug.h"
#include "utl/CRC.h"
#include "utl/Std.h"
#include <map>

template <class T>
class RefRes {
private:
    unsigned int mRefCount; // 0x0
    T *mData; // 0x4
public:
    RefRes() : mRefCount(0), mData(0) {}
    unsigned int RefCount() const { return mRefCount; }
    void AddRef() { mRefCount++; }
    // bool RemoveRef();
    T *&Data() { return mData; }
};

template <class T>
class ResMgr {
public:
    virtual ~ResMgr() {}
    virtual void OnReleaseResource(void *) = 0;
    virtual void Dump() {
        MILO_LOG("Resource Count : %d \n", mResources.size());
        MILO_LOG("-------------------------------------------\n");
        FOREACH (it, mResources) {
            RefRes<T> &data = it->second;
            if (data.RefCount()) {
                MILO_LOG("%d: %d\n", it->first, data.RefCount());
            }
        }
        MILO_LOG("\n\n");
    }

    T *Get(Hmx::CRC key) {
        auto it = mResources.find(key);
        if (it != mResources.end()) {
            it->second.AddRef();
            return it->second.Data();
        } else {
            return nullptr;
        }
    }

    void ReserveRes(Hmx::CRC key, T *data) {
        RefRes<T> &res = mResources[key];
        MILO_ASSERT(res.Data() == NULL, 0x50);
        res.Data() = data;
        res.AddRef();
    }

    bool ReleaseRes(Hmx::CRC);

protected:
    std::map<Hmx::CRC, RefRes<T> > mResources; // 0x4
};
