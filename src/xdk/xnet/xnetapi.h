#pragma once
#include "../win_types.h"
#include "winsockx.h"

#ifdef __cplusplus
extern "C" {
#endif

INT XNetRandom(BYTE *pb, UINT cb);
DWORD XNetGetEthernetLinkStatus();

DWORD XNetGetConnectStatus(const IN_ADDR ina);
INT XNetUnregisterInAddr(const IN_ADDR ina);
INT XNetConnect(const IN_ADDR ina);

DWORD XNetGetTitleXnAddr(XNADDR *pxna);
INT XNetXnAddrToMachineId(const XNADDR *pxnaddr, ULONGLONG *pqwMachineId);

INT XNetServerToInAddr(const IN_ADDR ina, DWORD dwServiceId, IN_ADDR *pina);

#ifdef __cplusplus
}
#endif
