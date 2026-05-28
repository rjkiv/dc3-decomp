#pragma once
#include "win_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct IUnknown { /* Size=0x4 */
    virtual HRESULT QueryInterface(const _GUID &riid, void **ppvObject);
    virtual ULONG AddRef();
    virtual ULONG Release();

    IUnknown(const IUnknown &);
    IUnknown();
    IUnknown &operator=(const IUnknown &);
};

#ifdef __cplusplus
}
#endif
