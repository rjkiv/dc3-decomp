#include "char/ClipCollide.h"
#include "CharClipSet.h"
#include "char/CharClip.h"
#include "char/CharServoBone.h"
#include "char/CharUtl.h"
#include "char/Waypoint.h"
#include "math/Vec.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "rndobj/Draw.h"
#include "rndobj/Mat.h"
#include "rndobj/Mesh.h"
#include "rndobj/Trans.h"
#include "utl/Symbol.h"

void clipcollideunusedlmao(Transform &xfm) {
    xfm.LookAt(Vector3(0, 0, 0), Vector3(1, 1, 1));
}

ClipCollide::ClipCollide()
    : mReports(), mGraph(0), mChar(this), mCharPath(""), mWaypoint(this),
      mPosition("front"), mClip(this), mReportString(), mWorldLines(false),
      mMoveCamera(true), mMode() {
    mGraph = RndGraph::Get(this);
}

ClipCollide::~ClipCollide() { mGraph->Free(this, false); }

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

INIT_REVS(1, 0)

BEGIN_LOADS(ClipCollide)
    LOAD_REVS(bs)
    ASSERT_REVS(1, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    d >> mChar;
    d >> mCharPath;
    d >> mWaypoint;
    d >> mPosition;
    mClip = nullptr;
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
    if (mChar && mWaypoint) {
        static Symbol front("front");
        static Symbol back("back");
        static Symbol left("left");
        static Symbol right("right");
        mChar->Enter();
        Waypoint *w = mWaypoint;
        mChar->Teleport(w);
        Transform xfm = w->WorldXfm();
        float r = w->YRadius();
        if (r <= 0) {
            r = w->Radius();
        }
        if (mPosition == front) {
            ScaleAddEq(xfm.v, xfm.m.y, r);
        } else if (mPosition == back) {
            ScaleAddEq(xfm.v, xfm.m.y, -r);
        } else if (mPosition == left) {
            ScaleAddEq(xfm.v, xfm.m.x, w->Radius());
        } else {
            ScaleAddEq(xfm.v, xfm.m.x, -w->Radius());
        }
        mChar->SetLocalXfm(xfm);
    }
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

void ClipCollide::Demonstrate() {
    bool valid = mChar && mWaypoint && mClip;
    if (valid) {
        SyncWaypoint();
        mChar->Driver()->Play(mClip, 2, -1.0f, 1e+30f, 0.0f);
    }
}

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
    if (mChar) {
        for (ObjDirItr<Waypoint> it(Dir(), true); it != nullptr; ++it) {
            if (ValidWaypoint(it)) {
                mWaypoint = it;
                TestClips();
            }
        }
    }
}

void ClipCollide::TestClips() {
    if (mWaypoint && mChar) {
        for (ObjDirItr<CharClip> it(Clips(), true); it != nullptr; ++it) {
            if (ValidClip(it)) {
                const char *directions[4] = { "front", "back", "left", "right" };
                for (int i = 0; i < 4; i++) {
                    mPosition = Symbol(directions[i]);
                    mClip = it;
                    Collide();
                }
            }
        }
    }
}

ObjectDir *ClipCollide::Clips() { return !mChar ? nullptr : mChar->Driver()->ClipDir(); }

void ClipCollide::Collide() {
    bool valid = mChar && mWaypoint && mClip;
    if (valid) {
        mChar->SetShowing(false);
        RndDrawable *drawDir = dynamic_cast<RndDrawable *>(Dir());
        const char *bones[3] = { "bone_L-ankle", "bone_R-ankle", "bone_pos_guitar" };
        RndTransformable *boneTranses[3];
        for (int i = 0; i < 3; i++) {
            boneTranses[i] = CharUtlFindBoneTrans(bones[i], mChar);
        }
        SyncWaypoint();
        CharServoBone *servo = mChar->BoneServo();
        float addVal = 0;
        Vector3 vd0[3];
        for (float beat = mClip->StartBeat(); beat <= mClip->EndBeat(); beat += 1) {
            mClip->ScaleDown(*servo, 0);
            mClip->ScaleAdd(*servo, 1, beat, addVal);
            servo->Poll();
            for (int i = 0; i < 3; i++) {
                Vector3 v150 = boneTranses[i]->WorldXfm().v;
                if (i == 2) {
                    ScaleAddEq(v150, boneTranses[i]->WorldXfm().m.z, 2.5f);
                }
                if (addVal > 0) {
                    if (mWorldLines && mGraph) {
                        mGraph->AddLine(vd0[i], v150, Hmx::Color(1, 0, 0), false);
                    }
                    Segment segment;
                    segment.start = vd0[i];
                    segment.end = v150;
                    float t;
                    Plane p;
                    RndDrawable *collided = drawDir->Collide(segment, t, p);
                    if (collided) {
                        Vector3 v130;
                        Interp(segment.start, segment.end, t, v130);
                        bool donotadd = v130.z < mChar->WorldXfm().v.z + 1.0f;
                        if (!donotadd) {
                            RndMesh *mesh = dynamic_cast<RndMesh *>(collided);
                            if (mesh) {
                                RndMat *mat = mesh->Mat();
                                if (mat) {
                                    if (!mat->GetDiffuseTex() && mat->Alpha() <= 0) {
                                        donotadd = true;
                                    }
                                }
                            }
                        }
                        if (!donotadd) {
                            AddReport(v130);
                        }
                    }
                }
                vd0[i] = v150;
            }
            addVal = 1;
        }
        mChar->SetShowing(true);
    }
}

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
        Report &cur = mReports[i];
        arr->Node(i + 1) =
            MakeString("%d %s %s %s", i + 1, cur.clip, cur.waypoint->Name(), cur.name);
    }
    DataNode ret = arr;
    arr->Release();
    return ret;
}

DataNode ClipCollide::OnListClips(DataArray *da) {
    std::list<CharClip *> cliplist;
    ObjectDir *clipDir = Clips();
    if (clipDir) {
        for (ObjDirItr<CharClip> it(clipDir, true); it != nullptr; ++it) {
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
    DataNode ret = arr;
    arr->Release();
    return ret;
}

DataNode ClipCollide::OnListWaypoints(DataArray *da) {
    std::list<Waypoint *> waylist;
    for (ObjDirItr<Waypoint> it(Dir(), true); it != nullptr; ++it) {
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
    DataNode ret = arr;
    arr->Release();
    return ret;
}
