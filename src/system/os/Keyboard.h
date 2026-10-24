#pragma once
#include "obj/Object.h"

void KeyboardInitCommon();
void KeyboardTerminateCommon();

void KeyboardInit();
void KeyboardTerminate();
void KeyboardPoll();

void KeyboardSubscribe(Hmx::Object *sink);
void KeyboardUnsubscribe(Hmx::Object *sink);
Hmx::Object *KeyboardOverride(Hmx::Object *override);
void KeyboardSendMsg(int key, bool shift, bool ctrl, bool alt);

#include "obj/Msg.h"

DECLARE_MESSAGE(KeyboardKeyMsg, "key")
KeyboardKeyMsg(int key, bool shift, bool ctrl, bool alt)
    : Message(Type(), key, shift, ctrl, alt) {}
int GetKey() const { return mData->Int(2); }
bool GetShift() const { return mData->Int(3); }
bool GetCtrl() const { return mData->Int(4); }
bool GetAlt() const { return mData->Int(5); }
END_MESSAGE

DECLARE_MESSAGE(KeyboardKeyReleaseMsg, "key_release")
END_MESSAGE

// HMX keyboard mappings from macros.dta
#define KB_ENTER (10)
#define KB_BACKSPACE (8)
#define KB_TAB (9)
#define KB_SPACE (32)
#define KB_a (97)
#define KB_b (98)
#define KB_c (99)
#define KB_d (100)
#define KB_e (101)
#define KB_f (102)
#define KB_g (103)
#define KB_h (104)
#define KB_i (105)
#define KB_j (106)
#define KB_k (107)
#define KB_l (108)
#define KB_m (109)
#define KB_n (110)
#define KB_o (111)
#define KB_p (112)
#define KB_q (113)
#define KB_r (114)
#define KB_s (115)
#define KB_t (116)
#define KB_u (117)
#define KB_v (118)
#define KB_w (119)
#define KB_x (120)
#define KB_y (121)
#define KB_z (122)
#define KB_CAP_LOCK (290)
#define KB_NUM_LOCK (291)
#define KB_SCROLL_LOCK (292)
#define KB_PRINT (300)
#define KB_PAUSE (301)
#define KB_ESCAPE (302)
#define KB_INSERT (310)
#define KB_DELETE (311)
#define KB_HOME (312)
#define KB_END (313)
#define KB_PAGE_UP (314)
#define KB_PAGE_DOWN (315)
#define KB_LEFT (320)
#define KB_RIGHT (321)
#define KB_UP (322)
#define KB_DOWN (323)
#define KB_F1 (401)
#define KB_F2 (402)
#define KB_F3 (403)
#define KB_F4 (404)
#define KB_F5 (405)
#define KB_F6 (406)
#define KB_F7 (407)
#define KB_F8 (408)
#define KB_F9 (409)
#define KB_F10 (410)
#define KB_F11 (411)
#define KB_F12 (412)
