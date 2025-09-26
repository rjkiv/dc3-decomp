#pragma once
#include "gesture/GestureMgr.h"
#include "rndobj/Mat.h"
#include "rndobj/Tex.h"

RndMat *CreateCameraBufferMat(int, int, RndTex::Type);
void TerminateDrawUtl();
void InitDrawUtl(const GestureMgr &);
