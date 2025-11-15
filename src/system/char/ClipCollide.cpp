#include "char/ClipCollide.h"
#include "CharClipSet.h"
#include "char/CharClip.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "utl/Symbol.h"

ClipCollide::ClipCollide()
    : mReports(), mGraph(0), mChar(this), mCharPath(""), mWaypoint(this),
      mPosition("front"), mClip(this), mReportString(), mWorldLines(false),
      mMoveCamera(true), mMode() {
    mGraph = RndGraph::Get(this);
}

ClipCollide::~ClipCollide() { mGraph->Free(this, false); }

BEGIN_PROPSYNCS(ClipCollide)
    SYNC_PROP_MODIFY(character, mChar, SyncChar())
    SYNC_PROP_MODIFY(pick_character, mCharPath, SyncChar())
    SYNC_PROP_MODIFY(waypoint, mWaypoint, SyncWaypoint())
    SYNC_PROP_MODIFY(position, mPosition, SyncWaypoint())
    SYNC_PROP_MODIFY(mode, mMode, SyncMode())
    SYNC_PROP(clip, mClip)
    SYNC_PROP_SET(clips, Clips(), )
    SYNC_PROP_SET(pick_report, mReportString, PickReport(_val.Str()))
    SYNC_PROP(world_lines, mWorldLines)
    SYNC_PROP(move_camera, mMoveCamera)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(ClipCollide)
    SAVE_REVS(1, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    bs << mChar;
    bs << mCharPath;
    bs << mWaypoint;
    bs << mPosition;
END_SAVES

BEGIN_LOADS(ClipCollide)
    LOAD_REVS(bs)
    ASSERT_REVS(1, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    bs >> mChar;
    bs >> mCharPath;
    bs >> mWaypoint;
    bs >> mPosition;
    mClip = 0;
END_LOADS

BEGIN_COPYS(ClipCollide)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(ClipCollide)
END_COPYS

void ClipCollide::SetTypeDef(DataArray *da) {
    Hmx::Object::SetTypeDef(da);
    if (da) {
        DataArray *modesArr = da->FindArray("modes");
        DataArray *strArray = modesArr->Array(1);
        mMode = strArray->Sym(0);
    }
}

void ClipCollide::SyncChar() {
    if (mChar) {
        if (!mCharPath.empty()) {
            FilePath fp(mCharPath.c_str());
            FilePath curproxy(mChar->ProxyFile());
            if (fp != curproxy) {
                mChar->SetProxyFile(fp, false);
            }
        }
    }
    SyncWaypoint();
}

void ClipCollide::SyncWaypoint() {
    if (!mChar || !mWaypoint)
        return;
    static Symbol front("front");
    static Symbol back("back");
    static Symbol left("left");
    static Symbol right("right");
}

void ClipCollide::ClearReport() {
    mGraph->Reset();
    mReports.clear();
    mReportString = "";
    SyncMode();
}

void ClipCollide::SyncMode() {
    if (!mMode.Null()) {
        Message msg("set_mode", mMode);
        Handle(msg, true);
    }
}

void ClipCollide::Demonstrate() {}

bool ClipCollide::ValidWaypoint(Waypoint *w) {
    static Message vw("valid_waypoint", 0);
    vw[0] = w;
    DataNode handled = Handle(vw, true);
    if (handled.Type() == kDataUnhandled)
        return true;
    else
        return handled.Int();
}

bool ClipCollide::ValidClip(CharClip *clip) {
    if (!mWaypoint)
        return true;
    else {
        static Message vw("valid_clip", 0, 0);
        vw[0] = clip;
        vw[1] = mWaypoint.Ptr();
        DataNode handled = Handle(vw, true);
        if (handled.Type() == kDataUnhandled)
            return true;
        else
            return handled.Int();
    }
}

void ClipCollide::TestChars() {
    if (mChar) {
        if (TypeDef()) {
            DataArray *charsArr = TypeDef()->FindArray("chars", false);
            if (charsArr) {
                DataArray *arr = charsArr->Array(1);
                for (int i = 0; i < arr->Size(); i++) {
                    String str(arr->Str(i));
                    if (!str.empty()) {
                        mCharPath = str;
                        SyncChar();
                        TestWaypoints();
                    }
                }
            }
        }
    }
}

void ClipCollide::TestWaypoints() {
    if (!mChar)
        return;
    for (ObjDirItr<Waypoint> it(Dir(), true); it != 0; ++it) {
        if (ValidWaypoint(it)) {
            mWaypoint = it;
            TestClips();
        }
    }
}

void ClipCollide::TestClips() {
    if (!mWaypoint || !mChar)
        return;
    for (ObjDirItr<CharClip> it(Clips(), true); it != 0; ++it) {
        if (ValidClip(it)) {
            const char *directions[4] = { "front", "back", "left", "right" };
            for (int i = 0; i < 4; i++) {
                mPosition = directions[i];
                mClip = it;
                Collide();
            }
        }
    }
}

ObjectDir *ClipCollide::Clips() { return !mChar ? nullptr : mChar->Driver()->ClipDir(); }

void ClipCollide::Collide() { bool b1 = true; }

void ClipCollide::AddReport(Vector3 v) {
    Report report;
    String proxy(mChar->ProxyFile());
    strcpy(report.name, FileGetBase(proxy.c_str()));
    strcpy(report.clip, mClip->Name());
    report.waypoint = mWaypoint;
    report.position = mPosition;
    report.pos = v;
    strcpy(report.charPath, mCharPath.c_str());
    mReports.push_back(report);
    if (mGraph) {
        mGraph->AddSphere(v, 3.0f, Hmx::Color(0, 0, 1));
        mGraph->AddString3D(MakeString("%d", mReports.size()), v, Hmx::Color(1, 1, 1));
    }
}

void ClipCollide::PickReport(const char *cc) {
    mReportString = cc;
    int idx = atoi(cc);
    if (idx > 0) {
        Report &curReport = mReports[idx - 1];
        mCharPath = curReport.charPath;
        SyncChar();
        mWaypoint = curReport.waypoint;
        mClip = mChar->Driver()->ClipDir()->Find<CharClip>(curReport.clip, true);
        mPosition = curReport.position;
        Demonstrate();
    }
}

DataNode ClipCollide::OnVenueName(DataArray *da) {
    String str;
    return str; // ????????
}

DataNode ClipCollide::OnListReport(DataArray *da) {
    DataArray *arr = new DataArray(mReports.size() + 1);
    arr->Node(0) = "";
    for (int i = 0; i < mReports.size(); i++) {
        arr->Node(i + 1) = MakeString(
            "%d %s %s %s",
            i + 1,
            mReports[i].clip,
            mReports[i].waypoint->Name(),
            mReports[i].name
        );
    }
    DataNode ret(arr, kDataArray);
    arr->Release();
    return ret;
}

DataNode ClipCollide::OnListClips(DataArray *da) {
    std::list<CharClip *> cliplist;
    ObjectDir *clipDir = Clips();
    if (clipDir) {
        for (ObjDirItr<CharClip> it(clipDir, true); it != 0; ++it) {
            if (ValidClip(it))
                cliplist.push_back(it);
        }
        cliplist.sort(ObjNameSort());
    }
    DataArray *arr = new DataArray(cliplist.size() + 1);
    arr->Node(0) = NULL_OBJ;
    int idx = 1;
    for (std::list<CharClip *>::iterator it = cliplist.begin(); it != cliplist.end();
         it++) {
        arr->Node(idx++) = *it;
    }
    DataNode ret(arr, kDataArray);
    arr->Release();
    return ret;
}

DataNode ClipCollide::OnListWaypoints(DataArray *da) {
    std::list<Waypoint *> waylist;
    for (ObjDirItr<Waypoint> it(Dir(), true); it != 0; ++it) {
        if (ValidWaypoint(it))
            waylist.push_back(it);
    }
    waylist.sort(ObjNameSort());
    DataArray *arr = new DataArray(waylist.size() + 1);
    arr->Node(0) = NULL_OBJ;
    int idx = 1;
    for (std::list<Waypoint *>::iterator it = waylist.begin(); it != waylist.end();
         it++) {
        arr->Node(idx++) = *it;
    }
    DataNode ret(arr, kDataArray);
    arr->Release();
    return ret;
}

BEGIN_HANDLERS(ClipCollide)
    HANDLE(list_clips, OnListClips)
    HANDLE(list_waypoints, OnListWaypoints)
    HANDLE(list_report, OnListReport)
    HANDLE_ACTION(demonstrate, Demonstrate())
    HANDLE_ACTION(collide, Collide())
    HANDLE_ACTION(test_clips, TestClips())
    HANDLE_ACTION(test_waypoints, TestWaypoints())
    HANDLE_ACTION(test_chars, TestChars())
    HANDLE_ACTION(clear_report, ClearReport())
    HANDLE(venue_name, OnVenueName)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS
