#include "gesture/ArcDetector.h"
#include "gesture/BaseSkeleton.h"
#include "math/Vec.h"
#include "os/Debug.h"
#include "rndobj/Rnd.h"
#include "utl/Std.h"
#include <stddef.h>

static int _hoverTime = 600;
static float _acceptablePathErrorRatio = 0.9f;
static float _swipeRetentionFactor = 0.5f;
static float _yErrorReduction = 0.2f;

ArcDetector::ArcDetector() {
    mCurveStart.Set(0, 0, 0);
    mArcSize = 0;
    mArcHeight = 0;
    mDesiredLength = 0.15f;
    mInitialized = false;
    mNeedSwipeReset = false;
    mHoverTimer = _hoverTime;
    Clear();
}

ArcDetector::~ArcDetector() {}

void ArcDetector::ResetHoverTimer() { mHoverTimer = _hoverTime; }

void ArcDetector::Initialize(
    SkeletonSide side, SkeletonJoint j1, SkeletonJoint j2, float f4
) {
    mDesiredLength = f4;
    mDirection = side;
    mInitialized = true;
    mTrackJoint = j1;
    mRefJoint = j2;
}

void ArcDetector::Update(const Skeleton &skel, int playernum) {
    if (!mInitialized) {
        MILO_ASSERT(false, 74);
    }
    if (!skel.IsTracked()) {
        Clear();
        return;
    }
    const Vector3 &body_maybe = skel.TrackedJoints()[mRefJoint].mPos[0],
                  &hand_maybe = skel.TrackedJoints()[mTrackJoint].mPos[0];
    float hpos_x = hand_maybe.x - body_maybe.x, hpos_y = hand_maybe.y - body_maybe.y,
          hpos_z = hand_maybe.z - body_maybe.z; // why
    Vector3 hand_pos(hpos_x, hpos_y, hpos_z); // in body coords? i think
    mLastRelativeSwipePos = hand_pos;
    if (mJointPath.empty()) {
        TryToStartSwipe(hand_pos, skel);
    } else if (mNeedSwipeReset) {
        Vector3 unk = *mJointPath.begin();
        Clear();
        mJointPath.insert(mJointPath.begin(), hand_pos);
        if (mDirection == kSkeletonLeft && hpos_x >= unk.x + 0.01f) {
            mNeedSwipeReset = false;
        }
        if (mDirection == kSkeletonRight && hpos_x <= unk.x - 0.01f) {
            mNeedSwipeReset = false;
        }
    } else {
        mCurveStart = GetCurveStart();
        Vector3 v = mJointPath.front();
        v.x = hpos_x - v.x;
        v.y = hpos_y - v.y;
        v.z = hpos_z - v.z;
        if (LengthSquared(v) > 0.0001f) {
            mJointPath.insert(mJointPath.begin(), hand_pos);
        }
        const Vector3 &body_maybe = skel.TrackedJoints()[mRefJoint].mPos[0],
                      &hand_maybe = skel.TrackedJoints()[mTrackJoint].mPos[0];
        float hpos_x_recalc = hand_maybe.x - body_maybe.x,
              hpos_z_recalc = hand_maybe.z - body_maybe.z;
        float len = Length(Vector2(hpos_x_recalc, hpos_z_recalc));
        mArcSize = (len + mArcSize) / 2;
    }
    mArcHeight = skel.TrackedJoints()[mTrackJoint].mPos[0].y
        - skel.TrackedJoints()[mRefJoint].mPos[0].y;
    CullPath();
    mMaxSwipePercent = Max(mMaxSwipePercent, GetSwipeAmount());
    if (!IsPathAcceptable()) {
        SwipeFailed(skel);
    }
    if (GetSwipeAmount() < 0.1f) {
        mHoverTimer = Max(0, mHoverTimer - playernum);
    }
}

Vector3 ArcDetector::GetCurveStart() const {
    MILO_ASSERT(!mJointPath.empty(), 0xE9);
    return Vector3((mDirection != kSkeletonLeft ? -1 : 1) * mArcSize, mArcHeight, 0);
}

void ArcDetector::Clear() {
    mMaxSwipePercent = 0;
    mJointPath.clear();
    mArcHeight = 0;
    mArcSize = 0;
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
    if (mMaxSwipePercent > 0.5)
        mNeedSwipeReset = true;
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
            if (mDirection == kSkeletonLeft && cur.x >= first) {
                other.push_back(cur);
            }
            if (mDirection == kSkeletonRight && cur.x <= first) {
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
    float inv_static = 1.0f / _yErrorReduction;
    FOREACH_CONST (joint, mJointPath) {
        Vector3 this_joint = *joint;
        // the hand location relative to this joint.
        float jointspace_x = mCurveStart.x - this_joint.x;
        if (mDirection == kSkeletonRight) {
            jointspace_x *= -1.0f;
        }
        float jspace_x_scl = jointspace_x * mArcSize;
        float jointspace_z = mCurveStart.z - this_joint.z;
        float sqrt_jspace_x = jspace_x_scl * 2 - (jointspace_x * jointspace_x);
        if (sqrt_jspace_x <= 0) {
            sqrt_jspace_x = 0;
        } else {
            sqrt_jspace_x = sqrt(sqrt_jspace_x);
        }
        jointspace_z -= sqrt_jspace_x;
        float jspace_y = this_joint.y - mArcHeight;
        accumulator += LengthSquared(Vector3(0, jointspace_z, jspace_y * inv_static));
    }
    return accumulator;
}

void ArcDetector::TryToStartSwipe(const Vector3 &startpos, const Skeleton &skel) {
    MILO_ASSERT(mJointPath.empty(), 139);
    bool joint_not_tracked = true;
    if (skel.TrackedJoints()[mTrackJoint].mConfidence != kConfidenceTracked
        || skel.TrackedJoints()[mRefJoint].mConfidence != kConfidenceTracked) {
        joint_not_tracked = false;
    }
    if (joint_not_tracked) {
        mJointPath.push_front(startpos);
        const Vector3 &ucjoint = skel.TrackedJoints()[mRefJoint].mPos[0];
        const Vector3 &u8joint = skel.TrackedJoints()[mTrackJoint].mPos[0];
        mArcSize = Length(Vector2(u8joint.x - ucjoint.x, u8joint.z - ucjoint.z));
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
            float f0 = i * mArcSize * 0.02f;
            float f13 = mArcSize * f0;
            // next two lines: pretty sure this is some weird intrin stuff
            int x = mDirection == kSkeletonLeft ? -1 : 1; // why is this an int...?

            f13 = f13 * 2.0f - (f0 * f0);
            if (f13 <= 0) {
                f13 = 0;
            } else {
                f13 = sqrt(f13);
            }
            v.Set(x * f0, 0.0f, -f13);
            joints2draw.insert(joints2draw.begin(), v);
        }
        Add(mCurveStart, skel.TrackedJoints()[mRefJoint].mPos[0], v);
        DrawPath(joints2draw, viz, Hmx::Color(1, 1, 0), v);
    }
    DrawPath(
        mJointPath, viz, Hmx::Color(1, 0, 1), skel.TrackedJoints()[mRefJoint].mPos[0]
    );
}

bool ArcDetector::IsPathAcceptable() const {
    static float _startRatio = 0.5f;
    if (1 >= mJointPath.size()) {
        return true;
    } else {
        if (!IsLockedIn()) {
            const Vector3 &front = mJointPath.front();
            const Vector3 &back = mJointPath.back();
            float vecYsub = front.y - back.y;
            float vecXsub = (mDirection != kSkeletonLeft ? 1 : -1) * (front.x - back.x);
            float vecZsub = front.z - back.z;
            if (vecXsub < 0) {
                return false;
            }
            if (vecYsub != 0) {
                float f3 = 1.0f / vecYsub;
                if (f3 * vecXsub < _startRatio && vecYsub < _startRatio) {
                    return false;
                }
            }
        } else {
            return 0.0f < GetSwipeAmount();
        }
    }
    return true;
}
