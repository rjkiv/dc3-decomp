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
    u32 Data_() { return reinterpret_cast<u32>(mRes); } // hack
    void AddRef() { mRefs++; }
    void SetData(T *data) { mRes = data; }
    uint NumRefs() const { return mRefs; }
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
        do {
            if (!(res.Data_() == NULL)) {
                TheDebugFailer << MakeString(
                    kAssertStr,
                    "/drvs/sda1/projects_2/dc3-decomp/src/system/utl/ResMgr.h",
                    0x50,
                    "res.Data() == NULL"
                );
            }
        } while (0);
        res.SetData(data);
        res.AddRef();
    }

    bool ReleaseRes(Hmx::CRC);

protected:
    std::map<Hmx::CRC, RefRes<T> > mResources; // 0x4
};
