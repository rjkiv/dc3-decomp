#include "char/CharHair.h"
#include "char/CharCollide.h"
#include "char/Character.h"
#include "math/Geo.h"
#include "math/Mtx.h"
#include "math/Rot.h"
#include "math/Trig.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "rndobj/Poll.h"
#include "rndobj/Trans.h"
#include "rndobj/Wind.h"
#include "utl/BinStream.h"
#include "world/Dir.h"

CharHair *gHair;
CharHair::Strand *gStrand;

#pragma region CharHair::Point

CharHair::Point::Point(Hmx::Object *owner)
    : bone(owner), collides(owner), length(0), radius(0), outerRadius(0), sideLength(-1) {
    pos.Zero();
    force.Zero();
    lastFriction.Zero();
    lastZ.Zero();
    pose.Zero();
}

CharHair::Point::Point(const CharHair::Point &pt) : bone(pt.bone), collides(pt.collides) {
    pos = pt.pos;
    bone = pt.bone;
    length = pt.length;
    collides = pt.collides;
    radius = pt.radius;
    outerRadius = pt.outerRadius;
    force = pt.force;
    lastFriction = pt.lastFriction;
    lastZ = pt.lastZ;
    sideLength = pt.sideLength;
    pose = pt.pose;
}

BEGIN_CUSTOM_PROPSYNC(CharHair::Point)
    SYNC_PROP(bone, o.bone)
    SYNC_PROP(length, o.length)
    SYNC_PROP(collides, o.collides)
    SYNC_PROP(radius, o.radius)
    SYNC_PROP(outer_radius, o.outerRadius)
    SYNC_PROP(side_length, o.sideLength)
END_CUSTOM_PROPSYNC

void operator<<(BinStream &bs, const CharHair::Point &p) {
    bs << p.pos;
    bs << p.bone;
    bs << p.length;
    bs << p.radius;
    bs << p.outerRadius;
    bs << p.sideLength;
    bs << p.pose;
}

void operator>>(BinStreamRev &d, CharHair::Point &pt) {
    d >> pt.pos;
    d >> pt.bone;
    d >> pt.length;
    if (d.rev < 3) {
        int x;
        char buf[0x100];
        d >> x;
        d.stream.ReadString(buf, 0xff);
    } else if (d.rev == 3) {
        int x;
        d >> x;
    }
    d >> pt.radius;
    if (d.rev > 1) {
        d >> pt.outerRadius;
    } else {
        pt.outerRadius = 0;
    }
    if (d.rev < 9 && d.rev > 5) {
        float x;
        d >> x;
        pt.radius += x;
        pt.outerRadius += x;
    }
    char buf[0x100];
    if (d.rev == 6) {
        d.stream.ReadString(buf, 0xff);
    }
    if (d.rev < 8) {
        pt.sideLength = -1;
        if (d.rev > 5) {
            int x;
            d >> x >> x;
        }
    } else {
        bool b = false;
        if (d.rev < 9) {
            d >> b;
        }
        d >> pt.sideLength;
        if (d.rev < 9 && !b) {
            pt.sideLength = -1;
        }
    }
    if (d.rev > 9) {
        d >> pt.pose;
    }
    pt.collides.clear();
    pt.force.Zero();
    pt.lastFriction.Zero();
    pt.lastZ.Zero();
}

#pragma endregion CharHair::Point
#pragma region CharHair::Strand

BEGIN_CUSTOM_PROPSYNC(CharHair::Strand)
    gStrand = &o;
    SYNC_PROP_SET(root, o.mRoot.Ptr(), o.SetRoot(_val.Obj<RndTransformable>()))
    SYNC_PROP_SET(angle, o.mAngle, o.SetAngle(_val.Float()))
    SYNC_PROP(points, o.mPoints)
    SYNC_PROP(hookup_flags, o.mHookupFlags)
    SYNC_PROP(show_spheres, o.mShowSpheres)
    SYNC_PROP(show_collide, o.mShowCollide)
    SYNC_PROP(show_pose, o.mShowPose)
END_CUSTOM_PROPSYNC

void CharHair::Strand::Save(BinStream &bs) const {
    bs << mRoot;
    bs << mAngle;
    bs << mPoints;
    bs << mBaseMat;
    bs << mRootMat;
    bs << mHookupFlags;
}

BinStream &operator<<(BinStream &bs, const CharHair::Strand &s) {
    s.Save(bs);
    return bs;
}

void CharHair::Strand::Load(BinStreamRev &d) {
    d >> mRoot;
    d >> mAngle;
    d >> mPoints;
    d >> mBaseMat;
    d >> mRootMat;
    if (d.rev > 2) {
        d >> mHookupFlags;
    } else {
        mHookupFlags = 0;
    }
}

BinStreamRev &operator>>(BinStreamRev &d, CharHair::Strand &s) {
    s.Load(d);
    return d;
}

void CharHair::Strand::SetAngle(float ang) {
    mAngle = ang;
    Hmx::Matrix3 mtx;
    MakeRotMatrixX(ang * DEG2RAD, mtx);
    Multiply(mtx, mBaseMat, mRootMat);
}

void CharHair::Strand::SetRoot(RndTransformable *trans) {
    mRoot = trans;
    if (!mRoot)
        mPoints.resize(0);
    else {
        float len = mPoints.size() != 0 ? mPoints.back().length : 0;
        mBaseMat = mRoot->LocalXfm().m;
        SetAngle(mAngle);

        int depth = 0;
        for (RndTransformable *it = mRoot;; it = it->Children().front()) {
            depth++;
            if (it->Children().empty())
                break;
        }

        mPoints.resize(depth);
        depth = 0;
        for (RndTransformable *it = mRoot;; it = it->Children().front(), depth++) {
            mPoints[depth].bone = it;
            if (it->Children().empty())
                break;
        }

        Point *pt = nullptr;
        for (int i = 1; i < mPoints.size(); i++) {
            pt = &mPoints[i - 1];
            RndTransformable *bone = mPoints[i].bone;
            pt->length = bone->LocalXfm().v.y;
            pt->pos = bone->WorldXfm().v;
        }
        Point *backpt = &mPoints.back();
        backpt->length = len ? len : (pt ? pt->length : gUnitsPerMeter * 0.127f);
        ScaleAdd(
            backpt->bone->WorldXfm().v,
            backpt->bone->WorldXfm().m.y,
            backpt->length,
            backpt->pos
        );
    }
}

#pragma endregion CharHair::Strand
#pragma region CharHair

CharHair::CharHair()
    : mStiffness(0.04), mTorsion(0.1), mInertia(0.7), mGravity(1), mWeight(0.5),
      mFriction(0.3), mWind(1), mFlat(1), mMinSlack(0), mMaxSlack(0), mStrands(this),
      mReset(1), mSimulate(1), mUsePostProc(1), mWindObj(this), mCollides(this),
      mManagedHookup(0) {}

CharHair::~CharHair() {}

BEGIN_HANDLERS(CharHair)
    HANDLE_ACTION(reset, mReset = _msg->Int(2))
    HANDLE_ACTION(hookup, Hookup())
    HANDLE_ACTION(set_cloth, SetCloth(_msg->Int(2)))
    HANDLE_ACTION(freeze_pose, FreezePose())
    HANDLE_SUPERCLASS(RndPollable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(CharHair)
    gHair = this;
    SYNC_PROP(stiffness, mStiffness)
    SYNC_PROP(torsion, mTorsion)
    SYNC_PROP(inertia, mInertia)
    SYNC_PROP(gravity, mGravity)
    SYNC_PROP(weight, mWeight)
    SYNC_PROP(friction, mFriction)
    SYNC_PROP(wind_obj, mWindObj)
    SYNC_PROP(wind, mWind)
    SYNC_PROP(flat, mFlat)
    SYNC_PROP(strands, mStrands)
    SYNC_PROP(simulate, mSimulate)
    SYNC_PROP(min_slack, mMinSlack)
    SYNC_PROP(max_slack, mMaxSlack)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(CharHair)
    SAVE_REVS(0xD, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    bs << mStiffness;
    bs << mTorsion;
    bs << mInertia;
    bs << mGravity;
    bs << mWeight;
    bs << mFriction;
    bs << mMinSlack;
    bs << mMaxSlack;
    bs << mStrands;
    bs << mSimulate;
    bs << mWindObj;
    bs << mWind;
    bs << mFlat;
END_SAVES

BEGIN_COPYS(CharHair)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(CharHair)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mStiffness)
        COPY_MEMBER(mInertia)
        COPY_MEMBER(mGravity)
        COPY_MEMBER(mWeight)
        COPY_MEMBER(mFriction)
        COPY_MEMBER(mTorsion)
        COPY_MEMBER(mStrands)
        COPY_MEMBER(mSimulate)
        COPY_MEMBER(mMinSlack)
        COPY_MEMBER(mMaxSlack)
        COPY_MEMBER(mWindObj)
        COPY_MEMBER(mWind)
        COPY_MEMBER(mFlat)
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(13, 0)

BEGIN_LOADS(CharHair)
    LOAD_REVS(bs);
    ASSERT_REVS(13, 0);
    LOAD_SUPERCLASS(Hmx::Object)
    d >> mStiffness >> mTorsion >> mInertia >> mGravity >> mWeight >> mFriction;
    if (d.rev < 8) {
        mMinSlack = 0.0f;
        mMaxSlack = 0.0f;
    } else {
        d >> mMinSlack >> mMaxSlack;
    }
    d >> mStrands;
    d >> mSimulate;
    if (d.rev > 10) {
        d >> mWindObj;
    }
    if (d.rev > 11) {
        d >> mWind;
    }
    if (d.rev > 12) {
        d >> mFlat;
    }
END_LOADS

void CharHair::SetName(const char *name, ObjectDir *dir) {
    Hmx::Object::SetName(name, dir);
    mUsePostProc = dynamic_cast<Character *>(dir) || dynamic_cast<WorldDir *>(dir);
}

void CharHair::Poll() {
    Character *cur = Character::Current();
    if (cur) {
        if (cur->Synced()) {
            Hookup();
        }
        if (cur->Teleported()) {
            mReset = 1;
        }
        if (cur->LODCheck()) {
            DoReset(0);
            return;
        }
    }
    if (mReset > 0) {
        DoReset(mReset);
    }
    if (TheTaskMgr.DeltaSeconds() != 0) {
        SimulateLoops(1, GetFPS());
    } else {
        SimulateZeroTime();
    }
}

void CharHair::Enter() {
    mReset = 1;
    RndPollable::Enter();
    Hookup();
}

void CharHair::PollDeps(
    std::list<Hmx::Object *> &changedBy, std::list<Hmx::Object *> &change
) {
    for (int i = 0; i < mStrands.size(); i++) {
        changedBy.push_back(mStrands[i].Root());
        change.push_back(mStrands[i].Root());
    }
}

void CharHair::SetCloth(bool b) {
    for (int i = 0; i < mStrands.size(); i++) {
        int mod = Mod(i + 1, mStrands.size());
        Strand &modidx = mStrands[mod];
        for (int j = 0; j < (unsigned int)mStrands[i].NumPoints(); j++) {
            Point &point = mStrands[i].PointAt(j);
            bool cond = b && j < (unsigned int)modidx.NumPoints();
            point.sideLength = cond ? Distance(point.pos, modidx.PointAt(j).pos) : -1.0f;
        }
    }
}

void CharHair::Hookup() {
    if (!mManagedHookup) {
        ObjPtrList<CharCollide> list(this);
        for (ObjDirItr<CharCollide> it(Dir(), true); it != nullptr; ++it) {
            list.push_back(it);
        }
        list.sort(SortCollides());
        Hookup(list);
    }
}

void CharHair::FreezePose() {
    bool oldSim = mSimulate;
    Hookup();
    SimulateLoops(200, 60);
    mSimulate = oldSim;
    FreezePoseRaw();
}

float CharHair::GetFPS() {
    if (mUsePostProc && RndPostProc::Current()
        && RndPostProc::Current()->EmulateFPS() > 0) {
        float fps = RndPostProc::Current()->EmulateFPS();
        return fps != 60 ? 60 - fps : fps;
    } else
        return 60;
}

void CharHair::SimulateLoops(int count, float f) {
    if (mSimulate && mStrands.size()) {
        START_AUTO_TIMER("char_hair");
        FOREACH (it, mCollides) {
            (*it)->SyncWorldState();
        }
        for (int n = 0; n < count; n++) {
            SimulateInternal(f);
        }
    }
}

void CharHair::FreezePoseRaw() {
    for (int i = 0; i < mStrands.size(); i++) {
        Strand &strand = mStrands[i];
        RndTransformable *root = strand.Root();
        if ((int)root && root->TransParent()) {
            ObjVector<Point> &pts = strand.Points();
            Transform tf48(root->TransParent()->WorldXfm());
            Invert(tf48, tf48);
            for (int j = 0; j < pts.size(); j++) {
                Multiply(pts[j].pos, tf48, pts[j].pose);
            }
        }
    }
}

void CharHair::SimulateZeroTime() {
    if (mSimulate) {
        for (int i = 0; i < mStrands.size(); i++) {
            Strand &curStrand = mStrands[i];
            RndTransformable *root = curStrand.Root();
            if (root && curStrand.Root()->TransParent()) {
                Transform tf50;
                tf50.v = curStrand.Root()->WorldXfm().v;
                Multiply(
                    curStrand.RootMat(),
                    curStrand.Root()->TransParent()->WorldXfm().m,
                    tf50.m
                );
                ObjVector<Point> &points = curStrand.Points();
                for (int j = 0; j < points.size(); j++) {
                    Point &curPoint = points[j];
                    Hmx::Matrix3 m78;
                    Subtract(curPoint.pos, tf50.v, m78.y);
                    m78.z = curPoint.lastZ;
                    Normalize(m78, tf50.m);
                    if (curPoint.bone) {
                        curPoint.bone->SetWorldXfm(tf50);
                    }
                    tf50.v = curPoint.pos;
                }
            }
        }
    }
}

void CharHair::DoReset(int reset) {
    for (int i = 0; i < mStrands.size(); i++) {
        Strand &strand = mStrands[i];
        if (strand.Root() && strand.Root()->TransParent()) {
            ObjVector<Point> &pts = strand.Points();
            Transform tf70(strand.Root()->TransParent()->WorldXfm());
            Vector3 v80(strand.Root()->WorldXfm().v);
            Vector3 v8c(strand.Root()->WorldXfm().m.x);
            for (int j = 0; j < pts.size(); j++) {
                Point &pt = pts[j];
                Multiply(pt.pose, tf70, pt.pos);
                Vector3 v98;
                Subtract(pt.pos, v80, v98);
                v80 = pt.pos;
                Cross(v8c, v98, pt.lastZ);
                Normalize(pt.lastZ, pt.lastZ);
                Cross(v98, pt.lastZ, v8c);
                pt.force.Zero();
                pt.lastFriction.Zero();
            }
        }
    }
    bool tmpsim = mSimulate;
    float tmpinert = mInertia;
    float tmpfric = mFriction;
    mSimulate = true;
    mInertia = 0;
    mFriction = 0;
    SimulateLoops(reset, GetFPS());
    mSimulate = tmpsim;
    mFriction = tmpfric;
    mInertia = tmpinert;
    mReset = 0;
}

void CharHair::SimulateInternal(float f) {
    float sixtyover = 60.0f / f;
    float f19 = (1.0f / f) * sixtyover;
    float powed = std::pow(1.0f - mStiffness, sixtyover * sixtyover);
    Vector3 vec134(0, 0, 0);
    if (mWindObj) {
        if (mStrands[0].Root()) {
            float secs = TheTaskMgr.Seconds(TaskMgr::kTaskTRVideo);
            mWindObj->GetWind(mStrands[0].Root()->WorldXfm().v, secs, vec134);
            vec134 *= f19 * 0.5f;
        }
    }
    vec134.z = vec134.z + mGravity * f19 * -3.858268f;

    for (int i = 0; i < mStrands.size(); i++) {
        Strand &modStrand = mStrands[Mod(i + 1, mStrands.size())];
        Strand &thisStrand = mStrands[i];
        if (thisStrand.Root() && thisStrand.Root()->TransParent()) {
            Transform t100;
            t100.v = thisStrand.Root()->WorldXfm().v;
            Multiply(
                thisStrand.RootMat(),
                thisStrand.Root()->TransParent()->WorldXfm().m,
                t100.m
            );
            ObjVector<Point> &points = thisStrand.Points();
            for (int j = 0; j < points.size(); j++) {
                Point &thisPoint = points[j];
                Vector3 v140(thisPoint.pos);
                thisPoint.pos += thisPoint.force;
                thisPoint.pos.x += vec134.x;
                thisPoint.pos.y += vec134.y;
                thisPoint.pos.z += vec134.z;
                if (thisPoint.sideLength >= 0.0f) {
                    Vector3 vRes;
                    Point &modPoint = modStrand.Points()[j];
                    Subtract(thisPoint.pos, modPoint.pos, vRes);
                    float lensq = LengthSquared(vRes);
                    float sidelen = thisPoint.sideLength - mMinSlack;
                    float sidelensq = sidelen * sidelen;
                    if (lensq < sidelensq) {
                        vRes *= (sidelensq / (sidelensq + lensq) - 0.5f);
                        thisPoint.pos += vRes;
                        modPoint.pos -= vRes;
                    } else {
                        float maxslacklen = thisPoint.sideLength + mMaxSlack;
                        float maxslacklensq = maxslacklen * maxslacklen;
                        if (maxslacklen > maxslacklensq) {
                            vRes *= (maxslacklensq / (maxslacklensq + lensq) - 0.5f);
                            thisPoint.pos += vRes;
                            modPoint.pos -= vRes;
                        }
                    }
                }
                Hmx::Matrix3 m128;
                Subtract(thisPoint.pos, t100.v, m128.y);
                float rsa = RecipSqrtAccurate(LengthSquared(m128.y));
                float rsalen = thisPoint.length * rsa - 1.0f;
                if (j > 0) {
                    ScaleAddEq(points[j - 1].force, m128.y, -sixtyover * 0.5f * rsalen);
                }
                ScaleAddEq(thisPoint.pos, m128.y, rsalen);
                Vector3 v158;
                ScaleAdd(t100.v, t100.m.y, thisPoint.length, v158);
                Interp(thisPoint.lastZ, t100.m.z, mTorsion, m128.z);
                if (thisPoint.collides.size() != 0) {
                    float diffRad = thisPoint.outerRadius - thisPoint.radius;
                    float maxRad = Max(thisPoint.radius, thisPoint.outerRadius);
                    for (ObjPtrList<CharCollide>::iterator it =
                             thisPoint.collides.begin();
                         it != thisPoint.collides.end();
                         ++it) {
                        CharCollide *thisCollide = *it;
                        Vector3 v164;
                        float collideRad = thisCollide->GetRadius(thisPoint.pos, v164);
                        switch (thisCollide->GetShape()) {
                        case CharCollide::kCollidePlane: // 0
                            if (maxRad > collideRad) {
                                ScaleAddEq(
                                    thisPoint.pos,
                                    thisCollide->Axis(),
                                    maxRad - collideRad
                                );
                            }
                            break;
                        case CharCollide::kCollideCigar: // 3
                        case CharCollide::kCollideSphere: { // 1
                            float v164sq = LengthSquared(v164);
                            float sumRad = collideRad + maxRad;
                            if (v164sq < sumRad * sumRad) {
                                if (diffRad > 0.0f) {
                                    float v164sqrecip = RecipSqrtAccurate(v164sq);
                                    float cluster = v164sq * v164sqrecip;
                                    float othersumRad = collideRad + thisPoint.radius;
                                    v164 *= -v164sqrecip;
                                    if (cluster < othersumRad) {
                                        m128.z = v164;
                                        ScaleAddEq(
                                            thisPoint.pos, v164, cluster - othersumRad
                                        );
                                    } else
                                        Interp(
                                            m128.z,
                                            v164,
                                            (sumRad - cluster) / diffRad,
                                            m128.z
                                        );
                                } else
                                    ScaleAddEq(
                                        thisPoint.pos,
                                        v164,
                                        sumRad * RecipSqrtAccurate(v164sq) - 1.0f
                                    );
                            }
                            break;
                        }
                        case CharCollide::kCollideInsideCigar: // 4
                        case CharCollide::kCollideInsideSphere: { // 2
                            float v164sq42 = LengthSquared(v164);
                            float minusRad = collideRad - maxRad;
                            if (v164sq42 > minusRad * minusRad) {
                                if (diffRad > 0.0f) {
                                    float v164sqrecip = RecipSqrtAccurate(v164sq42);
                                    float cluster = v164sq42 * v164sqrecip;
                                    float othersumRad = collideRad - thisPoint.radius;
                                    v164 *= -v164sqrecip;
                                    if (cluster > othersumRad) {
                                        m128.z = v164;
                                        ScaleAddEq(
                                            thisPoint.pos, v164, cluster - othersumRad
                                        );
                                    } else
                                        Interp(
                                            m128.z,
                                            v164,
                                            (cluster - minusRad) / diffRad,
                                            m128.z
                                        );
                                } else
                                    ScaleAddEq(
                                        thisPoint.pos,
                                        v164,
                                        minusRad * RecipSqrtAccurate(v164sq42) - 1.0f
                                    );
                            }
                            break;
                        }
                        default:
                            break;
                        }
                    }

                    Scale(m128.y, rsa, t100.m.y);
                    Cross(t100.m.y, m128.z, t100.m.x);
                    t100.m.x *= RecipSqrtAccurate(LengthSquared(t100.m.x));
                    Normalize(t100.m.x, t100.m.x);
                    Cross(t100.m.x, t100.m.y, t100.m.z);
                    thisPoint.lastZ = t100.m.z;
                    if (thisPoint.bone)
                        thisPoint.bone->SetWorldXfm(t100);
                    Subtract(v158, thisPoint.pos, thisPoint.force);
                    Vector3 v170;
                    Subtract(thisPoint.lastFriction, thisPoint.force, v170);
                    thisPoint.lastFriction = thisPoint.force;
                    thisPoint.force *= 1.0f - powed;
                    ScaleAddEq(thisPoint.force, v170, -mFriction);
                    Vector3 v17c;
                    Subtract(thisPoint.pos, v140, v17c);
                    ScaleAddEq(thisPoint.force, v17c, mInertia);
                    t100.v = thisPoint.pos;
                }
            }
        }
    }
}

void CharHair::Hookup(ObjPtrList<CharCollide> &collides) {
    mCollides.clear();
    for (int i = 0; i < mStrands.size(); i++) {
        Strand &curStrand = mStrands[i];
        if (curStrand.Root()) {
            for (int j = 0; j < curStrand.Points().size(); j++) {
                curStrand.PointAt(j).collides.clear();
            }
            FOREACH (it, collides) {
                // more...
            }
        }
    }
}

#pragma endregion CharHair
