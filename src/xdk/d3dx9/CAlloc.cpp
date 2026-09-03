#include "CAlloc.h"
#include "xapilibi/xbox.h"

namespace D3DXCore {
    CAlloc::CAlloc(uint cbReserve, uint cbCommit)
        : m_pbRegion(nullptr), m_cbPage(ROUNDUP(cbCommit, 4096)), m_cbAlloc(0),
          m_cbCommit(0), m_cbReserve(0) {
        // m_cbReserveMin = (cbReserve == 0 ? 0x100000 : 0);
    }
    CAlloc::~CAlloc() {
        while (m_pbRegion) {
            auto old_region = m_pbRegion;
            m_pbRegion = *reinterpret_cast<u8 **>(m_pbRegion); // kinda fake
            XMemFree(old_region, 0x24810000);
        }
    }
}
