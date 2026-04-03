#include "os/Joypad.h"
#include "obj/Data.h"
#include "obj/DataFunc.h"
#include "obj/Msg.h"
#include "os/Debug.h"
#include "obj/Object.h"
#include "os/JoypadMsgs.h"
#include "os/System.h"
#include "os/User.h"

namespace {
    class KeyboardJoypadExporter {
    public:
        virtual ~KeyboardJoypadExporter() {}
    };

    bool gJoypadDisabled[4]; // 0x0
    DataArray *gControllersCfg; // 0x4
    DataArray *gButtonMeanings; // 0x8
    int gPadsToKeepAlive; // 0xc
    int gPadsToKeepAliveNext; // 0x10
    int gKeepAliveCountdown; // 0x14
    unsigned int gHolmesPressed; // 0x18
    Hmx::Object *gJoypadMsgSource; // 0x1c
    bool gJoypadLibInitialized; // 0x20
    KeyboardJoypadExporter *gKeyboardExporter; // 0x24
    JoypadData gJoypadData[kNumJoypads]; // 0x28

    int gKeepaliveThresholdMs = -1;
    bool gExportMsgs = true;
    unsigned int gNotifyMask = 0x8F0;
}

JoypadData::JoypadData()
    : mButtons(0), mNewPressed(0), mNewReleased(0), mUser(nullptr), mConnected(false),
      mForceFeedback(true), mCanForceFeedback(0), mWireless(0), mNeedsVelocityDecay(0),
      mNumAnalogSticks(0), mTranslateSticks(false), mIgnoreButtonMask(0),
      mGreenCymbalMask(0), mYellowCymbalMask(0), mBlueCymbalMask(0),
      mSecondaryPedalMask(0), mCymbalMask(0), mIsDrum(false), mType(kJoypadNone),
      mControllerType(), mDistFromRest(0), mHasGreenCymbal(false),
      mHasYellowCymbal(false), mHasBlueCymbal(false), mHasSecondaryPedal(false),
      mBreedCallbackHandler(0), mpCallbackBreedData(0), mEpwDataLeft(0), mEpwDataSize(0),
      mEpwMaxPacketBytes(0), mEpwAckWait(0), mBreedDataThrottle(0), mEpwAcknowledged(0),
      mKeepaliveMs(0) {
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            mSticks[i][j] = 0;
        }
    }
    for (int i = 0; i < 2; i++) {
        mTriggers[i] = 0;
    }
}

float JoypadData::GetAxis(Symbol axis) const {
    static Symbol lx("LX");
    static Symbol ly("LY");
    static Symbol rx("RX");
    static Symbol ry("RY");
    static Symbol tl("TL");
    static Symbol tr("TR");
    static Symbol sx("SX");
    static Symbol sy("SY");
    static Symbol sz("SZ");
    if (axis == lx)
        return LX();
    else if (axis == ly)
        return LY();
    else if (axis == rx)
        return RX();
    else if (axis == ry)
        return RY();
    else if (axis == tl)
        return LT();
    else if (axis == tr)
        return RT();
    else if (axis == sx)
        return SX();
    else if (axis == sy)
        return SY();
    else if (axis == sz)
        return SZ();
    else
        MILO_FAIL("Bad axis %s in JoypadData::GetAxis()\n");
    return 0.0f;
}

int JoypadData::FloatToBucket(float f) const {
    if (f < 0.11f)
        return 0;
    if (f < 0.31f)
        return 6;
    if (f < 0.46f)
        return 5;
    if (f < 0.61f)
        return 4;
    if (f < 0.77f)
        return 3;
    if (f < 0.89f)
        return 2;
    return 1;
}

int JoypadData::GetVelocityBucket(Symbol axis) const {
    float ax = GetAxis(axis);
    if (ax < 0.0f)
        ax += 1.0f;
    return FloatToBucket(ax);
}

void JoypadTerminateCommon() {
    gJoypadLibInitialized = false;
    RELEASE(gJoypadMsgSource);
    RELEASE(gKeyboardExporter);
}

void JoypadSubscribe(Hmx::Object *obj) {
    if (gJoypadMsgSource) {
        gJoypadMsgSource->AddSink(obj, Symbol());
    }
}

void JoypadUnsubscribe(Hmx::Object *obj) {
    if (gJoypadMsgSource) {
        gJoypadMsgSource->RemoveSink(obj, Symbol());
    }
}

JoypadData *JoypadGetPadData(int pad_num) {
    MILO_ASSERT(0 <= pad_num && pad_num < kNumJoypads, 0x47B);
    return &gJoypadData[pad_num];
}

bool JoypadVibrate(int pad) { return JoypadGetPadData(pad)->mForceFeedback; }

bool JoypadIsConnectedPadNum(int padNum) {
    if (padNum == -1)
        return false;
    else
        return gJoypadData[padNum].mConnected;
}

namespace {
    bool IsJoypadDetectMatch(DataArray *detect_cfg, const JoypadData &data) {
        static Symbol type("type");
        static Symbol button("button");
        static Symbol stick("stick");
        static Symbol trigger("trigger");
        static Symbol X("X");
        static Symbol Y("Y");
        static Symbol OR("or");
        static Symbol AND("and");
        Symbol sym = detect_cfg->Sym(0);
        if (sym == type) {
            return detect_cfg->Int(1) == (int)data.mType;
        } else if (sym == button) {
            return data.Pressed((JoypadButton)detect_cfg->Int(1));
        } else if (sym == stick) {
            int i4 = 0;
            Symbol axis_sym = detect_cfg->Sym(2);
            if (axis_sym == X) {
                i4 = 0;
            } else if (axis_sym == Y) {
                i4 = 1;
            } else
                MILO_FAIL("bad axis %s in controller detect array\n", axis_sym);
            int i3 = detect_cfg->Int(1);
            float f7 = detect_cfg->Float(3);
            return f7 == data.mSticks[i3][i4];
        } else if (sym == trigger) {
            return detect_cfg->Float(2) == data.mTriggers[detect_cfg->Int(1)];
        } else if (sym == OR) {
            for (int i = 1; i < detect_cfg->Size(); i++) {
                if (IsJoypadDetectMatch(detect_cfg->Array(i), data))
                    return true;
            }
            return false;
        } else if (sym == AND) {
            for (int i = 1; i < detect_cfg->Size(); i++) {
                if (!IsJoypadDetectMatch(detect_cfg->Array(i), data))
                    return false;
            }
            return true;
        } else {
            MILO_FAIL("Unknown keyword '%s' in joypad detect block", sym);
            return false;
        }
    }

    void Export(const Message &msg) {
        if (gExportMsgs) {
            gJoypadMsgSource->Handle(msg, false);
        }
    }

    DataNode OnJoypadSetVibrate(DataArray *arr) {
        JoypadSetVibrate(arr->Int(1), arr->Int(2) != 0);
        return 1;
    }

    DataNode OnJoypadVibrate(DataArray *arr) { return JoypadVibrate(arr->Int(1)); }

    DataNode OnJoypadControllerTypePadNum(DataArray *arr) {
        return JoypadControllerTypePadNum(arr->Int(1));
    }

    DataNode OnJoypadIsConnectedPadNum(DataArray *arr) {
        return JoypadIsConnectedPadNum(arr->Int(1));
    }

    DataNode OnJoypadIsButtonDownPadNum(DataArray *arr) {
        int pad = arr->Int(1);
        MILO_ASSERT_RANGE(pad, 0, kNumJoypads, 0x7F);
        return gJoypadData[pad].Pressed((JoypadButton)arr->Int(2));
    }

    DataNode OnJoypadStageKitRaw(DataArray *arr) {
        arr->Int(2);
        arr->Int(1);
        // probably a stub
        return 1;
    }

    DataNode OnJoypadIsCalbertGuitar(DataArray *arr) {
        return JoypadIsCalbertGuitar(arr->Int(1)) != 0;
    }

    DataNode DataJoypadReset(DataArray *) {
        JoypadReset();
        return 0;
    }
}

void JoypadInitCommon(DataArray *joypad_config) {
    gJoypadMsgSource = Hmx::Object::New<Hmx::Object>();

    float thresh;
    joypad_config->FindData("threshold", thresh);
    joypad_config->FindData("keepalive_ms", gKeepaliveThresholdMs);
    for (int i = 0; i < 4; i++) {
        gJoypadData[i].mDistFromRest = thresh;
        gJoypadDisabled[i] = false;
    }

    DataArray *ignores = joypad_config->FindArray("ignore");
    for (int i = 1; i < ignores->Size(); i++) {
        int nodeInt = ignores->Int(i);
        if (nodeInt >= 0 && nodeInt < 4) {
            gJoypadDisabled[nodeInt] = true;
        }
    }
    gControllersCfg = joypad_config->FindArray("controllers");
    gButtonMeanings = joypad_config->FindArray("button_meanings");
    DataRegisterFunc("joypad_reset", DataJoypadReset);
    DataRegisterFunc("joypad_vibrate", OnJoypadVibrate);
    DataRegisterFunc("joypad_set_vibrate", OnJoypadSetVibrate);
    DataRegisterFunc("joypad_controller_type_padnum", OnJoypadControllerTypePadNum);
    DataRegisterFunc("joypad_is_connected_padnum", OnJoypadIsConnectedPadNum);
    DataRegisterFunc("joypad_is_button_down", OnJoypadIsButtonDownPadNum);
    DataRegisterFunc("joypad_stage_kit_raw", OnJoypadStageKitRaw);
    DataRegisterFunc("joypad_is_calbert_guitar", OnJoypadIsCalbertGuitar);
    gJoypadLibInitialized = true;
}

void TranslateSticksToButs(JoypadData &data, unsigned int &mask) {
    float dist = data.mDistFromRest;
    for (int i = 0; i < kNumAnalogSticks; i++) {
        if (data.mSticks[i][0] > dist) {
            mask |= 1 << (kPad_LStickRight + (i * 4));
        } else if (data.mSticks[i][0] < -dist) {
            mask |= 1 << (kPad_LStickLeft + (i * 4));
        }
        if (data.mSticks[i][1] > dist) {
            mask |= 1 << (kPad_LStickDown + (i * 4));
        } else if (data.mSticks[i][1] < -dist) {
            mask |= 1 << (kPad_LStickUp + (i * 4));
        }
    }
}

int GetUsersPadNum(const LocalUser *user) {
    for (int i = 0; i < 4; i++) {
        if (!gJoypadDisabled[i] && gJoypadData[i].mUser == user)
            return i;
    }
    return -1;
}

int JoypadGetUsersPadNum(const LocalUser *user) { return GetUsersPadNum(user); }

bool JoypadIsCalbertGuitar(int padNum) {
    JoypadType ty = gJoypadData[padNum].mType;
    if (ty == kJoypadXboxHxGuitarRb2 || ty == kJoypadPs3HxGuitarRb2
        || ty == kJoypadWiiHxGuitarRb2 || ty == kJoypadXboxButtonGuitar
        || ty == kJoypadPs3ButtonGuitar || ty == kJoypadWiiButtonGuitar)
        return true;
    else
        return false;
}

int JoypadData::GetPressureBucket(JoypadButton b) const {
    MILO_ASSERT(int(b) < kNumPressureButtons, 0x154);
    float val = mPressures[b];
    if (mType == kJoypadPs3RoDrums) {
        val = val * 255.0f;
        val = 1.0f - Clamp<float>(0.0f, 100.0f, val - 22.0f) / 100.0f;
    }
    return FloatToBucket(val);
    return 0;
}

int ButtonToVelocityBucket(JoypadData *data, JoypadButton btn) {
    static Symbol LX("LX");
    static Symbol LY("LY");
    static Symbol RX("RX");
    static Symbol RY("RY");
    switch (data->mType) {
    case kJoypadXboxDrumsRb2:
        switch (btn) {
        case kPad_Xbox_B:
            return data->GetVelocityBucket(LX);
        case kPad_Xbox_Y:
            return data->GetVelocityBucket(LY);
        case kPad_Xbox_X:
            return data->GetVelocityBucket(RX);
        case kPad_Xbox_A:
            return data->GetVelocityBucket(RY);
        default:
            return 0;
        }
        break;
    case kJoypadXboxDrums:
        switch (btn) {
        case kPad_Xbox_B:
            return data->GetVelocityBucket(LY);
        case kPad_Xbox_Y:
            return data->GetVelocityBucket(RX);
        case kPad_Xbox_X:
            return data->GetVelocityBucket(RX);
        case kPad_Xbox_A:
            return data->GetVelocityBucket(LY);
        default:
            return 0;
        }
        break;
    case kJoypadPs3HxDrums:
    case kJoypadPs3HxDrumsRb2:
    case kJoypadWiiHxDrumsRb2:
        switch (btn) {
        case kPad_Xbox_B:
        case kPad_Xbox_Y:
        case kPad_Xbox_X:
        case kPad_Xbox_A:
            return data->GetPressureBucket(btn);
        default:
            return 0;
        }
    default:
        return 0;
    }
}

void JoypadSetVibrate(int pad, bool vibrate) {
    JoypadSetActuatorsImp(pad, 0, 0);
    JoypadGetPadData(pad)->mForceFeedback = vibrate;
}

void AssociateUserAndPad(LocalUser *iUser, int iPadNum) {
    MILO_ASSERT_RANGE(iPadNum, 0, kNumJoypads, 0x4DB);
    gJoypadData[iPadNum].mUser = iUser;
}

void ResetAllUsersPads() {
    for (int i = 0; i < 4; i++)
        AssociateUserAndPad(nullptr, i);
}

LocalUser *JoypadGetUserFromPadNum(int iPadNum) {
    MILO_ASSERT_RANGE(iPadNum, 0, kNumJoypads, 0x4F2);
    return gJoypadData[iPadNum].mUser;
}

bool JoypadIsControllerTypePadNum(int padNum, Symbol controller_type) {
    MILO_ASSERT(padNum != -1, 0x500);
    JoypadData &data = gJoypadData[padNum];
    MILO_ASSERT(gControllersCfg, 0x503);
    DataArray *type_cfg = gControllersCfg->FindArray(controller_type, false);
    if (!type_cfg)
        return false;
    DataArray *detect_cfg = type_cfg->FindArray("detect");
    if (detect_cfg->Size() == 1
        || IsJoypadDetectMatch(detect_cfg->Array(1), gJoypadData[padNum])) {
        data.mControllerType = controller_type;
        data.mNumAnalogSticks = type_cfg->FindInt("num_analog_sticks");
        MILO_ASSERT((data.mNumAnalogSticks >= 0) && (data.mNumAnalogSticks <= kNumAnalogSticks), 0x50F);
        data.mTranslateSticks = type_cfg->FindInt("translate_sticks");
        data.mIgnoreButtonMask = 0;
        DataArray *ignore_arr = type_cfg->FindArray("ignore_buttons", false);
        if (ignore_arr) {
            for (int i = 1; i < ignore_arr->Size(); i++) {
                data.mIgnoreButtonMask |= (1 << ignore_arr->Int(i));
            }
        }
        data.mIsDrum = type_cfg->FindInt("is_drum");
        data.mCymbalMask = type_cfg->FindInt("cymbal_mask");
        data.mGreenCymbalMask = type_cfg->FindInt("green_cymbal_mask");
        data.mYellowCymbalMask = type_cfg->FindInt("yellow_cymbal_mask");
        data.mBlueCymbalMask = type_cfg->FindInt("blue_cymbal_mask");
        data.mSecondaryPedalMask = type_cfg->FindInt("secondary_pedal_mask");
        return true;
    }
    return false;
}

Symbol JoypadControllerTypePadNum(int padNum) {
    static Symbol none("none");
    static Symbol unknown("unknown");
    if (padNum == -1 || gJoypadDisabled[padNum] || !gJoypadData[padNum].mConnected)
        return none;
    JoypadData *theData = &gJoypadData[padNum];
    if (!theData->mControllerType.Null()) {
        return theData->mControllerType;
    } else {
        MILO_ASSERT(gControllersCfg, 0x53C);
        for (int i = 1; i < gControllersCfg->Size(); i++) {
            Symbol sym = gControllersCfg->Array(i)->Sym(0);
            if (JoypadIsControllerTypePadNum(padNum, sym))
                return sym;
        }
        return unknown;
    }
}

bool JoypadTypeHasLeftyFlip(Symbol type) {
    static Symbol none("none");
    if (type == none) {
        return false;
    } else {
        static Symbol lefty_flip("lefty_flip");
        DataArray *found = gControllersCfg->FindArray(type)->FindArray(lefty_flip);
        return found->Int(1) != 0;
    }
}

bool JoypadIsShiftButton(int padNum, JoypadButton btn) {
    static Symbol cymbal_shift_button("cymbal_shift_button");
    static Symbol pad_shift_button("pad_shift_button");
    static Symbol guitar_shift_button("guitar_shift_button");
    Symbol controller_type = JoypadControllerTypePadNum(padNum);
    DataArray *type_array = gControllersCfg->FindArray(controller_type, false);
    MILO_ASSERT(type_array, 0x5C6);
    if ((JoypadButton)type_array->FindArray(cymbal_shift_button)->Int(1) == btn) {
        return true;
    } else if ((JoypadButton)type_array->FindArray(pad_shift_button)->Int(1) == btn) {
        return true;
    } else if ((JoypadButton)type_array->FindArray(guitar_shift_button)->Int(1) == btn) {
        return true;
    } else
        return false;
}

JoypadAction ButtonToAction(JoypadButton btn, Symbol sym) {
    static Symbol none("none");
    JoypadAction ret = kAction_None;
    if (sym == none)
        return ret;
    else {
        DataArray *arr = gButtonMeanings->FindArray(sym, false);
        if (arr) {
            arr = arr->FindArray(btn, false);
            if (arr)
                ret = (JoypadAction)arr->Int(1);
        }
        return ret;
    }
}

void JoypadPushThroughMsg(const Message &msg) { Export(msg); }

void JoypadHandleBreedDataResponse(int pad) {
    if (gJoypadData[pad].mpCallbackBreedData) {
        *gJoypadData[pad].mpCallbackBreedData = gJoypadData[pad].mBreedData;
    }
    JoypadBreedDataReadMsg msg(gJoypadData[pad].mUser, kBreedSuccess);
    if (gJoypadData[pad].mBreedCallbackHandler) {
        gJoypadData[pad].mBreedCallbackHandler->Handle(msg, true);
        gJoypadData[pad].mBreedCallbackHandler = nullptr;
    }
}

void JoypadHandleEepromWriteResponse(int pad, JoypadBreedDataStatus status) {
    gJoypadData[pad].mEpwAcknowledged = true;
    if (!gJoypadData[pad].mEpwDataLeft) {
        JoypadBreedDataWriteMsg msg(gJoypadData[pad].mUser, status);
        if (gJoypadData[pad].mBreedCallbackHandler) {
            gJoypadData[pad].mBreedCallbackHandler->Handle(msg, true);
            gJoypadData[pad].mBreedCallbackHandler = nullptr;
        }
    }
}

unsigned int JoypadPollForButton(int pad) {
    if (!gJoypadLibInitialized) {
        return 0;
    } else {
        gExportMsgs = false;
        JoypadPoll();
        std::vector<WaitInfo> waitInfos;
        if (pad == -1) {
            for (int i = 0; i < kNumJoypads; i++) {
                if (!gJoypadDisabled[i]) {
                    WaitInfo info;
                    info.mPadNum = i;
                    info.mButtons = gJoypadData[i].mButtons;
                    waitInfos.push_back(info);
                }
            }
        } else {
            WaitInfo info;
            info.mPadNum = pad;
            info.mButtons = gJoypadData[pad].mButtons;
            waitInfos.push_back(info);
        }
        unsigned int pressedMask = 0;
        FOREACH (it, waitInfos) {
            pressedMask |= gJoypadData[it->mPadNum].mNewPressed;
        }
        if (pad == -1) {
            pressedMask |= gHolmesPressed;
        }
        gExportMsgs = true;
        return pressedMask;
    }
}
