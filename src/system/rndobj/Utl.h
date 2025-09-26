#pragma once
#include "rndobj/Draw.h"
#include "rndobj/Trans.h"
#include "math/Vec.h"
#include "math/Color.h"
#include <vector>

void SetLocalScale(RndTransformable *, const Vector3 &);

DataNode GetNormalMapTextures(ObjectDir *);
DataNode GetRenderTextures(ObjectDir *);

void ResetColors(std::vector<Hmx::Color> &colors, int newNumColors);
void RndScaleObject(Hmx::Object *, float, float);

bool SortDraws(RndDrawable *, RndDrawable *);

void CreateAndSetMetaMat(RndMat *);

void UtilDrawSphere(const Vector3 &, float, const Hmx::Color &, RndMat *);
void UtilDrawLine(const Vector2 &, const Vector2 &, const Hmx::Color &);
void UtilDrawString(const char *, const Vector3 &, const Hmx::Color &);
