#include "os/ContentMgr.h"
#include "obj/Data.h"

#include "obj/Object.h"
#include "utl/Loader.h"
#include "os/ThreadCall.h"

bool Content::Contains(const char *str) { return strstr(str, Root()) != nullptr; }
bool ContentMgr::RefreshDone() const { return mState == kDiscoveryEnumerating; }

bool ContentMgr::RefreshInProgress() {
    return mState != kDone && mState != kDiscoveryEnumerating && mState != 7
        && mState != 6;
}

Hmx::Object *ContentMgr::SetReadFailureHandler(Hmx::Object *handler) {
    Hmx::Object *old = mReadFailureHandler;
    mReadFailureHandler = handler;
    return old;
}

void ContentMgr::RefreshSynchronously() {
    StartRefresh();
    while (RefreshInProgress()) {
        PollRefresh();
        TheLoadMgr.Poll();
        ThreadCallPoll();
    }
}

void ContentMgr::OnReadFailure(bool b1, const char *cc) {
    if (mReadFailureHandler) {
        ContentReadFailureMsg msg(b1, cc);
        mReadFailureHandler->Handle(msg, true);
    }
}

bool ContentMgr::Contains(const char *cc, String &str) {
    for (std::list<Content *>::iterator it = mContents.begin(); it != mContents.end();
         ++it) {
        if ((*it)->Contains(cc)) {
            str = (*it)->DisplayName();
            return true;
        }
    }
    return false;
}

DataNode ContentMgr::OnRemoveContent(DataArray *a) {
    mDirty = true;
    for (std::list<String>::iterator it = mExtraContents.begin();
         it != mExtraContents.end();
         it) {
        if (streq(it->c_str(), a->Str(2))) {
            it = mExtraContents.erase(it);
        } else
            ++it;
    }
    return 0;
}

void ContentMgr::RegisterCallback(Callback *cb, bool midRefreshAllowed) {
    MILO_ASSERT(midRefreshAllowed || !RefreshInProgress(), 0x122);
    mCallbacks.push_back(cb);
}

void ContentMgr::UnregisterCallback(Callback *cb, bool midRefreshAllowed) {
    MILO_ASSERT(midRefreshAllowed || !RefreshInProgress(), 0x128);
    mCallbacks.remove(cb);
}

DataNode ContentMgr::OnAddContent(DataArray *da) {
    OnRemoveContent(da);
    mExtraContents.push_back(da->Str(2));
    return 0;
}

BEGIN_HANDLERS(ContentMgr)
    HANDLE_ACTION(start_refresh, StartRefresh())
    HANDLE_EXPR(refresh_done, RefreshDone())
    HANDLE_EXPR(never_refreshed, NeverRefreshed())
    HANDLE(add_content, OnAddContent)
    HANDLE(remove_content, OnRemoveContent)
    HANDLE_EXPR(delete_content, DeleteContent(_msg->Sym(2)))
    HANDLE_EXPR(is_mounted, IsMounted(_msg->Sym(2)))
    HANDLE_ACTION(refresh_synchronously, RefreshSynchronously())
END_HANDLERS
