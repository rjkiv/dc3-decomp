#include "gesture/ArcDetector.h"
#include "gesture/BaseSkeleton.h"
#include "os/Debug.h"
#include "rndobj/Rnd.h"
#include "utl/Std.h"
#include <stddef.h>

float ArcDetector::_swipeRetentionFactor = 0.5;
float ArcDetector::_acceptablePathErrorRatio = 0.89999998;
int sDefaultHoverTimer = 600;

ArcDetector::ArcDetector()
    : unk18(0, 0, 0), unk28(0), unk2c(0), unk30(0.15f), unk34(0), unk35(0),
      unk3c(sDefaultHoverTimer) {
    Clear();
}

ArcDetector::~ArcDetector() {}

void ArcDetector::ResetHoverTimer() { unk3c = sDefaultHoverTimer; }

void ArcDetector::Initialize(
    SkeletonSide side, SkeletonJoint j1, SkeletonJoint j2, float f4
) {
    unk30 = f4;
    mSide = side;
    unk34 = true;
    unk8 = j1;
    unkc = j2;
}

void ArcDetector::Update(const Skeleton &skel, int playernum) {
    if (!unk34) {
        MILO_ASSERT(false, 74);
    }
    if (!skel.IsTracked()) {
        Clear();
        return;
    }
    const Vector3 &body_maybe = skel.TrackedJoints()[unkc].mJointPos[0],
                  &hand_maybe = skel.TrackedJoints()[unk8].mJointPos[0];
    float hpos_x = hand_maybe.x - body_maybe.x, hpos_y = hand_maybe.y - body_maybe.y,
          hpos_z = hand_maybe.z - body_maybe.z; // why
    Vector3 hand_pos(hpos_x, hpos_y, hpos_z); // in body coords? i think
    unk40 = hand_pos;
    if (mJointPath.empty()) {
        TryToStartSwipe(hand_pos, skel);
    } else if (unk35) {
        Vector3 unk = *mJointPath.begin();
        Clear();
        mJointPath.insert(mJointPath.begin(), hand_pos);
        if (mSide == kSkeletonLeft && hpos_x >= unk.x + 0.01f) {
            unk35 = false;
        }
        if (mSide == kSkeletonRight && hpos_x <= unk.x - 0.01f) {
            unk35 = false;
        }
    } else {
        Vector3 v = unk18 = GetCurveStart();
        v.x -= hpos_x;
        v.y -= hpos_y;
        v.z -= hpos_z;
        if (LengthSquared(v) > 0.0001f) {
            mJointPath.insert(mJointPath.begin(), hand_pos);
        }
        const Vector3 &body_maybe = skel.TrackedJoints()[unkc].mJointPos[0],
                      &hand_maybe = skel.TrackedJoints()[unk8].mJointPos[0];
        float hpos_x_recalc = hand_maybe.x - body_maybe.x,
              hpos_z_recalc = hand_maybe.z - body_maybe.z;
        unk28 = (Vector2(hpos_x_recalc, hpos_z_recalc).Length() + unk28) / 2;
    }
    unk2c = skel.TrackedJoints()[unk8].mJointPos[0].y
        - skel.TrackedJoints()[unkc].mJointPos[0].y;
    CullPath();
    unk38 = Max(unk38, GetSwipeAmount());
    if (!IsPathAcceptable()) {
        SwipeFailed(skel);
    }
    if (GetSwipeAmount() < 0.1f) {
        unk3c = Max(0, unk3c - playernum);
    }
}

Vector3 ArcDetector::GetCurveStart() const {
    MILO_ASSERT(!mJointPath.empty(), 0xE9);
    return Vector3((mSide)*unk28, unk2c, 0);
}

void ArcDetector::Clear() {
    unk38 = 0;
    mJointPath.clear();
    unk2c = 0;
    unk28 = 0;
}

void ArcDetector::PrintJointPath() const {
    MILO_LOG("*** Hand path:\n");
    FOREACH (it, mJointPath) {
        MILO_LOG("%f, %f, %f,\n", it->x, it->y, it->z);
    }
    MILO_LOG("GetPathLength() %f\n", GetPathLength());
    MILO_LOG(
        "pow(GetPathLength(), _swipeRetentionFactor + 1) %f\n",
        pow(GetPathLength(), _swipeRetentionFactor + 1)
    );
    MILO_LOG("GetPathError() %f\n", GetPathError());
    MILO_LOG(
        "GetPathError() / _acceptablePathErrorRatio %f\n",
        GetPathError() / _acceptablePathErrorRatio
    );
    MILO_LOG("GetSwipeAmount() %f\n", GetSwipeAmount());
}

void ArcDetector::SwipeFailed(const Skeleton &skeleton) {
    if (unk38 > 0.5)
        unk35 = true;
    Vector3 vec = mJointPath.front();
    Clear();
    TryToStartSwipe(vec, skeleton);
}

bool ArcDetector::IsLockedIn() const {
    static int lock_in_point_req = 2;
    return mJointPath.size() > lock_in_point_req || GetSwipeAmount() > 0.2f;
}

void ArcDetector::CullPath() {
    if (!mJointPath.empty()) {
        std::list<Vector3> other;
        float first = mJointPath.front().x;
        FOREACH (it, mJointPath) {
            const Vector3 &cur = *it;
            if (mSide == kSkeletonLeft && cur.x >= first) {
                other.push_back(cur);
            }
            if (mSide == kSkeletonRight && cur.x <= first) {
                other.push_back(cur);
            }
        }
        mJointPath = other;
    }
}

float ArcDetector::GetPathError() const {
    if (mJointPath.empty()) {
        return 0.0f;
    }
    float accumulator = 0.0f;
    static float unk_static = 2.0f;
    float inv_static = 1.0f / unk_static;
    FOREACH_CONST (joint, mJointPath) {
        Vector3 this_joint = *joint;
        // the hand location relative to this joint.
        float jointspace_x = unk18.x - this_joint.x;
        if (mSide == kSkeletonRight) {
            jointspace_x *= -1.0f;
        }
        float jspace_x_scl = jointspace_x * unk28;
        float jointspace_z = unk18.z - this_joint.z;
        float sqrt_jspace_x = jspace_x_scl * 2 - (jointspace_x * jointspace_x);
        if (sqrt_jspace_x <= 0) {
            sqrt_jspace_x = 0;
        } else {
            sqrt_jspace_x = sqrt(sqrt_jspace_x);
        }
        jointspace_z -= sqrt_jspace_x;
        float jspace_y = this_joint.y - unk2c;
        accumulator += LengthSquared(Vector3(0, jointspace_z, jspace_y * inv_static));
    }
    return accumulator;
}

void ArcDetector::TryToStartSwipe(const Vector3 &startpos, const Skeleton &skel) {
    MILO_ASSERT(mJointPath.empty(), 139);
    bool joint_not_tracked = true;
    if (skel.TrackedJoints()[unk8].mJointConf != kConfidenceTracked
        || skel.TrackedJoints()[unkc].mJointConf != kConfidenceTracked) {
        joint_not_tracked = false;
    }
    if (joint_not_tracked) {
        mJointPath.push_front(startpos);
        const Vector3 &ucjoint = skel.TrackedJoints()[unkc].mJointPos[0];
        const Vector3 &u8joint = skel.TrackedJoints()[unk8].mJointPos[0];
        unk28 = Vector2(u8joint.x - ucjoint.x, u8joint.z - ucjoint.z).Length();
    }
}

float ArcDetector::UpdateOverlay(RndOverlay *ovl, float f) {
    static std::list<Vector3> jointPathCopy;
    if (!mJointPath.empty() && mJointPath.size() > 2) {
        jointPathCopy.clear();
        FOREACH_CONST (orig_joint, mJointPath) {
            jointPathCopy.push_front(*orig_joint);
        }
    }
    if (jointPathCopy.empty()) {
        return f;
    }

    Vector2 v(0, f);
    Hmx::Color c(1, 1, 1);
    TheRnd.DrawStringScreen(MakeString("%f %f"), v, c, true);
    return f;
}

void ArcDetector::Draw(const Skeleton &skel, SkeletonViz &viz) {
    const size_t joint_ct = mJointPath.size();
    if (joint_ct == 0)
        return;
    {
        std::list<Vector3> joints2draw;
        Vector3 v;
        for (int i = 0; i < 100; i++) {
            float f0 = i * unk28 * 0.02f;
            float f13 = unk28 * f0;
            // next two lines: pretty sure this is some weird intrin stuff
            int x = mSide == kSkeletonLeft ? -1 : 1; // why is this an int...?

            f13 = f13 * 2.0f - (f0 * f0);
            if (f13 <= 0) {
                f13 = 0;
            } else {
                f13 = sqrt(f13);
            }
            v.z = -f13;
            v.y = 0.0f;
            v.x = x * f0;
            joints2draw.insert(joints2draw.begin(), v);
        }
        Add(skel.TrackedJoints()[unkc].mJointPos[0], unk18, v);
        DrawPath(joints2draw, viz, Hmx::Color(1, 1, 0), v);
    }
    DrawPath(
        mJointPath, viz, Hmx::Color(1, 0, 1), skel.TrackedJoints()[unkc].mJointPos[0]
    );
}
