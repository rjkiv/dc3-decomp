#include "net/HttpReq.h"
#include "macros.h"
#include "stl/_map.h"

HttpReq::HttpReq(ReqType r, unsigned int ui, unsigned short us, char const *c)
    : mHostName(), mIPAddr(ui), unk10(us), mURL(c), unk1c(r), unk28(0), unk2c(0),
      unk30(0), unk4c(-1) {}

HttpReq::HttpReq(ReqType r, char const *c1, unsigned short us, char const *c2)
    : mHostName(c1), mIPAddr(0), unk10(us), mURL(c2), unk1c(r), unk28(0), unk2c(0),
      unk30(0), unk4c(-1) {}

HttpReq::~HttpReq() { RELEASE(unk28); }

void HttpReq::SetContent(const char *c) { unk28 = c; }

void HttpReq::SetContentLength(unsigned int ui) { unk2c = ui; }

void HttpReq::Reset() {
    unk4c = -1;
    unk30 = 0;
}

void HttpReq::SetType(ReqType r) { unk1c = r; }

void HttpReq::SetUserAgent(const char *c) { unk20 = c; }

bool HttpReq::HasFailed() { return unk4c == 4; }

bool HttpReq::HasSucceeded() { return unk4c == 3; }

void HttpReq::SetCookies(std::map<String, String> const &map) { unk34 = map; }

unsigned int HttpReq::GetStatusCode() { return unk30; }
