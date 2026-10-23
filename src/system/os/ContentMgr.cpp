#include "os/ContentMgr.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/File.h"
#include "utl/Loader.h"
#include "os/ThreadCall.h"
#include "utl/Std.h"

bool Content::Contains(const char *str) { return !(str - strstr(str, Root())); }

ContentMgr::CallbackFile::CallbackFile(
    const char *f, Callback *c, ContentLocT l, const char *n
)
    : mFile(f), mCallback(c), mLocation(l), mName(n) {}

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

void ContentMgr::Init() {
    SetName("content_mgr", ObjectDir::Main());
    mRootLoaded = 0;
    mDirty = true;
    mState = kInit;
    mReadFailureHandler = nullptr;
}

bool ContentMgr::RefreshDone() const { return mState == kDone; }

bool ContentMgr::RefreshInProgress() {
    return mState != kInit && mState != kDone && mState != kMounting
        && mState != kDeleting;
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

void ContentMgr::OnReadFailure(bool corrupt, const char *package_name) {
    if (mReadFailureHandler) {
        ContentReadFailureMsg msg(corrupt, package_name);
        mReadFailureHandler->Handle(msg, true);
    }
}

bool ContentMgr::Contains(const char *path, String &contentName) {
    FOREACH (it, mContents) {
        if ((*it)->Contains(path)) {
            contentName = (*it)->DisplayName();
            return true;
        }
    }
    return false;
}

void ContentMgr::RegisterCallback(Callback *c, bool midRefreshAllowed) {
    MILO_ASSERT(midRefreshAllowed || !RefreshInProgress(), 0x122);
    mCallbacks.push_back(c);
}

void ContentMgr::UnregisterCallback(Callback *c, bool midRefreshAllowed) {
    MILO_ASSERT(midRefreshAllowed || !RefreshInProgress(), 0x128);
    mCallbacks.remove(c);
}

void ContentMgr::AddCallbackFile(const char *dir, const char *file) {
    const char *str = MakeString("%s/%s", dir, file);
    mCallbackFiles.push_back(CallbackFile(str, mCallback, mLocation, mName.c_str()));
}

void ContentMgr::RecurseCallback(const char *dir, const char *file) {
    TheContentMgr.AddCallbackFile(dir, file);
}

void ContentMgr::PollRefresh() {
    if (mState == kDiscoveryMounting || mState == kMounting || mState == kDeleting) {
        for (auto it = mContents.begin(); it != mContents.end();) {
            Content::State oldContentState = (*it)->GetState();
            (*it)->Poll();
            Content::State newContentState = (*it)->GetState();
            if (newContentState == Content::kFailed) {
                MILO_LOG("content operation failed!\n");
                NotifyFailed(*it);
                if (mState == kMounting) {
                    (*it)->Unmount();
                    ++it;
                    continue;
                }
                if (mState != kDiscoveryMounting) {
                    ++it;
                    continue;
                }
                it = mContents.erase(it);
            } else {
                if (newContentState == Content::kMounting
                    || newContentState == Content::kUnmounting
                    || newContentState == Content::kContentDeleting) {
                    return;
                } else if (
                    newContentState == Content::kMounted
                    && oldContentState == Content::kMounting
                ) {
                    NotifyMounted(*it);
                    return;
                } else if (
                    newContentState == Content::kUnmounted
                    && oldContentState == Content::kUnmounting
                ) {
                    NotifyUnmounted(*it);
                    return;
                } else if (
                    newContentState == Content::kDeleted
                    && oldContentState == Content::kContentDeleting
                ) {
                    NotifyDeleted(*it);
                    return;
                }
                ++it;
            }
        }
        if (mState == kDiscoveryMounting) {
            FOREACH (it, mCallbacks) {
                (*it)->ContentAllMounted();
            }
            mState = kDiscoveryLoading;
        } else {
            mState = kDone;
        }
    }
    if (mState == kDiscoveryLoading) {
        if (mLoader) {
            if (!mLoader->IsLoaded())
                return;
            mCallback->ContentLoaded(mLoader, mLocation, mName.c_str());
            RELEASE(mLoader);
        } else if (mCallbackFiles.empty()) {
            FOREACH (it, mContents) {
                Content::State curContentState = (*it)->GetState();

                if (curContentState == Content::kAlwaysMounted) {
                    if (mRootLoaded > 0) {
                    here:
                        FOREACH (cit, mCallbacks) {
                            mCallback = *cit;
                            mLocation = (*it)->Location();
                            mName = (*it)->FileName();
                            if ((*cit)->ContentDir()) {
                                String str(
                                    FileMakePath((*it)->Root(), (*cit)->ContentDir())
                                );
                                String str2(
                                    MakeString("%s/%s", str, (*cit)->ContentPattern())
                                );
                                FileEnumerate(
                                    str.c_str(),
                                    ContentMgr::RecurseCallback,
                                    false,
                                    str2.c_str(),
                                    false
                                );
                            }
                            if ((*cit)->HasContentAltDirs()) {
                                std::vector<String> *altDirs = (*cit)->ContentAltDirs();
                                const char *pattern = (*cit)->ContentPattern();
                                static DataNode &n = DataVariable("extra_songs");
                                int num = n.Int() ? altDirs->size() : 2;
                                auto altDir = altDirs->begin();
                                for (int i = 0; altDir != altDirs->end() && i < num;
                                     i++, altDir++) {
                                    String str(
                                        FileMakePath((*it)->Root(), altDir->c_str())
                                    );
                                    String str2(MakeString("%s/%s", str, pattern));
                                    FileEnumerate(
                                        str.c_str(),
                                        ContentMgr::RecurseCallback,
                                        false,
                                        str2.c_str(),
                                        false
                                    );
                                }
                            }
                        }
                        if (curContentState == Content::kAlwaysMounted) {
                            mRootLoaded--;
                        }
                    }
                } else if (curContentState == Content::kMounted) {
                    goto here;
                }
            }
        }
        if (!mCallbackFiles.empty()) {
            MILO_ASSERT(mLoader == NULL, 0xED);
            mCallback = mCallbackFiles.front().mCallback;
            mLocation = mCallbackFiles.front().mLocation;
            mName = mCallbackFiles.front().mName;
            mLoader = TheLoadMgr.AddLoader(mCallbackFiles.front().mFile, kLoadFront);
            mCallbackFiles.pop_front();
            MILO_ASSERT(mLoader, 0xF4);
        } else {
            mState = kDiscoveryCheckIfDone;
        }
    } else if (mState == kDiscoveryCheckIfDone) {
        mState = kDone;
        StartRefresh();
        if (!RefreshInProgress()) {
            FOREACH (it, mCallbacks) {
                (*it)->ContentDone();
            }
        }
    }
}

DataNode ContentMgr::OnAddContent(DataArray *msg) {
    OnRemoveContent(msg);
    mExtraContents.push_back(msg->Str(2));
    return 0;
}

DataNode ContentMgr::OnRemoveContent(DataArray *msg) {
    mDirty = true;
    for (auto it = mExtraContents.begin(); it != mExtraContents.end();) {
        if (streq(it->c_str(), msg->Str(2))) {
            it = mExtraContents.erase(it);
        } else
            ++it;
    }
    return 0;
}
