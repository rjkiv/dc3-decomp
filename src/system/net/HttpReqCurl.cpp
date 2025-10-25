#include "net/HttpReqCurl.h"
#include "net/HttpReq.h"
#include "os/Debug.h"

HttpReqCurl::HttpReqCurl(ReqType rt, unsigned int ui, unsigned short us, char const *c)
    : HttpReq(rt, ui, us, c), unk50(0), mReq(0), unk70(0), unk72(0), unk74(10000) {
    unk58 = 0;
    unk5c = 0;
}

HttpReqCurl::HttpReqCurl(ReqType rt, char const *c1, unsigned short us, char const *c2)
    : HttpReq(rt, c1, us, c2), unk50(0), mReq(0), unk70(0), unk72(0), unk74(10000) {
    unk58 = 0;
    unk5c = 0;
}

HttpReqCurl::~HttpReqCurl() {}

void HttpReqCurl::Start() {}

void HttpReqCurl::Do() { MILO_ASSERT(mReq, 0x122); }

void HttpReqCurl::Reset() {}

char *HttpReqCurl::DetachBuffer() { return 0; }

unsigned int HttpReqCurl::GetBufferSize() {
    MILO_ASSERT(mReq, 0x141);
    return unk5c;
}

void HttpReqCurl::SetTimeout(unsigned int) {}

void HttpReqCurl::SetSSLCertPath(const char *path) {
    MILO_ASSERT(path, 0x100);
    unk60 = path;
}

void HttpReqCurl::SetSSLCertName(const char *name) {
    MILO_ASSERT(name, 0x108);
    unk68 = name;
}

void HttpReqCurl::SetSSLVerifyPeer(unsigned short value) {
    MILO_ASSERT(value <= 1, 0x110);
    unk70 = value;
}

void HttpReqCurl::SetSSLVerifyHost(unsigned short value) {
    MILO_ASSERT(value <= 2, 0x118);
    unk72 = value;
}
