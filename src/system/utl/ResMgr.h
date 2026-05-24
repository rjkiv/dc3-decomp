#pragma once
#include "os/Debug.h"
#include "utl/CRC.h"
#include "utl/Std.h"
#include <map>

template <class T>
class RefRes {
private:
    int mRefs; // 0x0
    T *mRes; // 0x4
public:
    RefRes() : mRefs(0), mRes(0) {}

    T *Data() { return mRes; }
    void AddRef() { mRefs++; }
    void SetData(T *data) { mRes = data; }
    int NumRefs() const { return mRefs; }
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
            if (data.NumRefs()) {
                MILO_LOG("%d: %d\n", it->first, data.NumRefs());
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
        res.SetData(data);
        res.AddRef();
    }

    bool ReleaseRes(Hmx::CRC);

protected:
    std::map<Hmx::CRC, RefRes<T> > mResources; // 0x4
};
