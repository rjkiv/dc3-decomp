#include "net_ham/RCJobDingo.h"
#include "net/DingoJob.h"
#include "obj/Object.h"

RCJob::RCJob(const char *url, Hmx::Object *callback) : DingoJob(url, callback) {}
RCJob::~RCJob() {}

void RCJob::SendCallback(bool success, bool cancelled) {
    if (mCallback) {
        ParseResponse();
        static RCJobCompleteMsg msg(this, false);
        msg[0] = this;
        msg[1] = success;
        mCallback->Handle(msg, true);
    }
}
