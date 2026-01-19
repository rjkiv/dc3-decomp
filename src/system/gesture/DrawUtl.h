#pragma once
#include "gesture/GestureMgr.h"
#include "rndobj/Mat.h"
#include "rndobj/Tex.h"

RndMat *CreateCameraBufferMat(int, int, RndTex::Type);
void TerminateDrawUtl();
void InitDrawUtl(const GestureMgr &);
void SetDrawSpace(float, float, float);
void DrawGestureMgr(GestureMgr &, LiveCameraInput::BufferType, float);
void DrawSnapshot(const GestureMgr &, int);
bool ToggleDrawSkeletons();
void DrawBufferMat(RndMat *, Hmx::Rect &);
bool UpdateBufferTex(LiveCameraInput *, RndTex *, LiveCameraInput::BufferType, GestureMgr *);