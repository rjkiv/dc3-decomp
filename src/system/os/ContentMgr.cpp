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
    const char *file, Callback *cb, ContentLocT loc, const char *name
)
    : mFile(file), mCallback(cb), mLocation(loc), mName(name) {}

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
    mState = kDone;
    mReadFailureHandler = nullptr;
}

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
    FOREACH (it, mContents) {
        if ((*it)->Contains(cc)) {
            str = (*it)->DisplayName();
            return true;
        }
    }
    return false;
}

void ContentMgr::RegisterCallback(Callback *cb, bool midRefreshAllowed) {
    MILO_ASSERT(midRefreshAllowed || !RefreshInProgress(), 0x122);
    mCallbacks.push_back(cb);
}

void ContentMgr::UnregisterCallback(Callback *cb, bool midRefreshAllowed) {
    MILO_ASSERT(midRefreshAllowed || !RefreshInProgress(), 0x128);
    mCallbacks.remove(cb);
}

void ContentMgr::AddCallbackFile(const char *c1, const char *c2) {
    const char *str = MakeString("%s/%s", c1, c2);
    mCallbackFiles.push_back(CallbackFile(str, mCallback, mLocation, mName.c_str()));
}

void ContentMgr::RecurseCallback(const char *c1, const char *c2) {
    TheContentMgr.AddCallbackFile(c1, c2);
}

void ContentMgr::PollRefresh() {
    if (mState == 3 || mState == 7 || mState == 6) {
        for (auto it = mContents.begin(); it != mContents.end();) {
            Content::State oldContentState = (*it)->GetState();
            (*it)->Poll();
            Content::State newContentState = (*it)->GetState();
            if (newContentState == 8) {
                MILO_LOG("content operation failed!\n");
                NotifyFailed(*it);
                if (mState == 7) {
                    (*it)->Unmount();
                    ++it;
                    continue;
                }
                if (mState != 3) {
                    ++it;
                    continue;
                }
                it = mContents.erase(it);
            } else {
                if (newContentState == 2 || newContentState == 3
                    || newContentState == 6) {
                    return;
                } else if (newContentState == 4 && oldContentState == 2) {
                    NotifyMounted(*it);
                    return;
                } else if (newContentState == 0 && oldContentState == 3) {
                    NotifyUnmounted(*it);
                    return;
                } else if (newContentState == 7 && oldContentState == 6) {
                    NotifyDeleted(*it);
                    return;
                }
                ++it;
            }
        }
        if (mState == 3) {
            FOREACH (it, mCallbacks) {
                (*it)->ContentAllMounted();
            }
            mState = kDiscoveryCheckIfDone; // 4
        } else {
            mState = kDiscoveryEnumerating; // 1
        }
    }
    if (mState == 4) {
        if (mLoader) {
            if (!mLoader->IsLoaded())
                return;
            mCallback->ContentLoaded(mLoader, mLocation, mName.c_str());
            RELEASE(mLoader);
        } else if (mCallbackFiles.empty()) {
            FOREACH (it, mContents) {
                auto curContentState = (*it)->GetState();

                if (curContentState == 5) {
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
                        if (curContentState == 5) {
                            mRootLoaded--;
                        }
                    }
                } else if (curContentState == 4) {
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
            mState = kMounting; // 5
        }
    } else if (mState == 5) {
        mState = kDiscoveryEnumerating; // 1
        StartRefresh();
        if (!RefreshInProgress()) {
            FOREACH (it, mCallbacks) {
                (*it)->ContentDone();
            }
        }
    }
}

DataNode ContentMgr::OnAddContent(DataArray *da) {
    OnRemoveContent(da);
    mExtraContents.push_back(da->Str(2));
    return 0;
}

DataNode ContentMgr::OnRemoveContent(DataArray *a) {
    mDirty = true;
    for (auto it = mExtraContents.begin(); it != mExtraContents.end();) {
        if (streq(it->c_str(), a->Str(2))) {
            it = mExtraContents.erase(it);
        } else
            ++it;
    }
    return 0;
}
