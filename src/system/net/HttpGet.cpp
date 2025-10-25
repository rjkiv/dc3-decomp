#include "net/HttpGet.h"
#include "stl/_vector.h"
#include "utl/MemMgr.h"
#include "utl/Str.h"

static float const kDefaultTimeoutMs = 5000.0f;

namespace {
    bool ValidateHeader(char *, int, int *, int *) { return false; }
    char *GetNextLine(char *, int *) { return 0; }
    int LineLength(char *, int) { return 1; }
    bool StrIStartsWith(String const &, char const *) { return false; }
    char *ParseHeader(char *, int, std::vector<String> *) { return 0; }
    unsigned int ParseStatusCode(std::vector<String> const &) { return 1; }
    int GetContentLength(std::vector<String> const &) { return 1; }
};

HttpPost::HttpPost(unsigned int, unsigned short, char const *, unsigned char) {}

HttpPost::~HttpPost() {}

void HttpPost::SetContent(char const *) {}

void HttpPost::SetContentLength(unsigned int) {}

bool HttpPost::CanRetry() { return false; }

void HttpPost::StartSending() {}

void HttpPost::Sending() {}

HttpGet::HttpGet(unsigned int ui, unsigned short us, char const *c1, char const *c2)
    : unk8(0), unkc(c1), unk14(us), unk18(-1), unk1c(false), unk50(kDefaultTimeoutMs),
      unk54(ui), unk58(c2), unk60(0), unk64(0), unk6c(0), unk70(0), unk74(0), unk78(0),
      unk7c(0), unk80(-1) {
    AddRequiredHeaders();
}

HttpGet::HttpGet(
    unsigned int ui, unsigned short us, char const *c1, unsigned char uc, char const *c2
)
    : unk8(0), unkc(c1), unk14(us), unk18(-1), unk1c(uc & 3), unk50(kDefaultTimeoutMs),
      unk54(ui), unk58(c2), unk60(0), unk64(0), unk6c(0), unk70(0), unk74(0), unk78(0),
      unk7c(0) {}

HttpGet::~HttpGet() { SafeShutdown(); }

bool HttpGet::IsDownloaded() { return false; }

bool HttpGet::HasFailed() { return false; }

char *HttpGet::DetachBuffer() { return 0; }

void HttpGet::Send() {}

void HttpGet::Poll() {}

void HttpGet::SetTimeout(float) {}

unsigned int HttpGet::GetBufferSize() { return 1; }

bool HttpGet::CanRetry() { return 1; }

void HttpGet::StartSending() {}

void HttpGet::Sending() {}

void HttpGet::StartReceiving() {}

void HttpGet::SafeDisconnect() {}

void HttpGet::SafeShutdown() {
    SafeDisconnect();
    if (unk6c != 0) {
        MemFree(&unk6c);
        unk6c = 0;
    }
    unk70 = 0;
    unk74 = 0;
}

void HttpGet::StartConnection() {}

bool HttpGet::HasTimedOut() { return false; }

void HttpGet::SetState(HttpGet::State) {}

void HttpGet::AddRequiredHeaders() {}
