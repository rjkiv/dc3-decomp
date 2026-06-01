#pragma once
#include "win_types.h"
#include "xdk/xapilibi/guiddef.h"

#ifdef __cplusplus
extern "C" {
#endif

DEFINE_IID(IUnknown, 00000000, 0000, 0000, C0, 00, 00, 00, 00, 00, 00, 46);

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
