#include "char/CharIKFingers.h"
#include "char/CharWeightable.h"
#include "math/Mtx.h"
#include "obj/Object.h"

CharIKFingers::CharIKFingers()
    : unk30(nullptr), unk44(nullptr), unk58(nullptr), unk6c(0), unk70(0), unk74(1),
      unk75(1), unkf8(0.85), mHandMoveForward(1), mHandPinkyRotation(-0.06),
      mHandThumbRotation(0.23), mHandDestOffset(-0.4), mIsRightHand(1), unk16d(0),
      unk16e(0), mOutputTrans(this), mKeyboardRefBone(this) {
    mFingerDescs.resize(5);
    unk78.Zero();
    unkc8.Zero();
    mHandKeyboardOffset = Vector3(0.3f, -6.0f, 0.4f);
    unk12c = Hmx::Matrix3();
}

CharIKFingers::~CharIKFingers() {}

BEGIN_PROPSYNCS(CharIKFingers)
    SYNC_PROP(is_right_hand, mIsRightHand)
    SYNC_PROP(output_trans, mOutputTrans)
    SYNC_PROP(keyboard_ref_bone, mKeyboardRefBone)
    SYNC_PROP(hand_keyboard_offset, mHandKeyboardOffset)
    SYNC_PROP(hand_thumb_rotation, mHandThumbRotation)
    SYNC_PROP(hand_pinky_rotation, mHandPinkyRotation)
    SYNC_PROP(hand_move_forward, mHandMoveForward)
    SYNC_PROP(hand_dest_offset, mHandDestOffset)
    SYNC_SUPERCLASS(CharWeightable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS
