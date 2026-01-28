#include "Rnd.h"
#include "math/Color.h"
#include "math/Mtx.h"
#include "math/Utl.h"
#include "math/Vec.h"
#include "obj/DataFunc.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/Endian.h"
#include "os/FileCache.h"
#include "os/Platform.h"
#include "os/System.h"
#include "rndobj/Bitmap.h"
#include "rndobj/Cam.h"
#include "rndobj/Dir.h"
#include "rndobj/Draw.h"
#include "rndobj/Env.h"
#include "rndobj/Flare.h"
#include "rndobj/Group.h"
#include "rndobj/Mat.h"
#include "rndobj/Mesh.h"
#include "rndobj/MetaMaterial.h"
#include "rndobj/Part.h"
#include "utl/Std.h"
#include "rndobj/Utl.h"

#include "math/Rand.h"

typedef void (*SplashFunc)(void);

FileCacheHelper gResourceFileCacheHelper;
float gLimitUVRange;
int gDxtCacher;
ObjectDir *sSphereDir;
RndMesh *sSphereMesh;
ObjectDir *sCylinderDir;
RndMesh *sCylinderMesh;
// std::list<BuildPoly> gChildPolys;
// std::list<BuildPoly> gParentPolys;
SplashFunc gSplashPoll;
SplashFunc gSplashSuspend;
SplashFunc gSplashResume;
Vector3 gUtlXfms;

RndGroup *GroupOwner(Hmx::Object *o) {
    if (o) {
        FOREACH (it, o->Refs()) {
            RndGroup *grp = dynamic_cast<RndGroup *>(it->RefOwner());
            if (grp) {
                if (grp->HasObject(o)) {
                    return grp;
                }
            }
        }
    }
    return nullptr;
}

DataNode OnGroupOwner(DataArray *da) { return GroupOwner(da->Obj<Hmx::Object>(1)); }

RndEnviron *FindEnviron(RndDrawable *d) {
    RndGroup *owner = GroupOwner(d);
    if (owner) {
        int i = owner->Draws().size();
        while (--i > 0) {
            if (owner->Draws()[i] == d && i >= 0) {
                for (; i >= 0; i--) {
                    RndEnviron *env = dynamic_cast<RndEnviron *>(owner->Draws()[i]);
                    if (env) {
                        return env;
                    }
                }
            }
        }
        return FindEnviron(owner);
    } else {
        RndDir *rdir = dynamic_cast<RndDir *>(d->Dir());
        if (rdir) {
            std::list<RndDrawable *> children;
            rdir->ListDrawChildren(children);
            if (ListFind(children, d)) {
                return rdir->GetEnv();
            }
        }
        MILO_NOTIFY("Need to find environment of draw parent");
    }
    return nullptr;
}

DataNode DataFindEnviron(DataArray *da) { return FindEnviron(da->Obj<RndDrawable>(1)); }

bool GroupedUnder(RndGroup *grp, Hmx::Object *o) {
    FOREACH (it, grp->Objects()) {
        if (*it == o)
            return true;
        RndGroup *casted = dynamic_cast<RndGroup *>(*it);
        if (casted && GroupedUnder(casted, o))
            return true;
    }
    return false;
}

void SetRndSplasherCallback(SplashFunc func1, SplashFunc func2, SplashFunc func3) {
    gSplashPoll = func1;
    gSplashSuspend = func2;
    gSplashResume = func3;
}

void RndSplasherPoll() {
    if (gSplashPoll)
        gSplashPoll();
}

void RndSplasherSuspend() {
    if (gSplashSuspend)
        gSplashSuspend();
}

void RndSplasherResume() {
    if (gSplashResume)
        gSplashResume();
}

const char *CacheResource(const char *, const Hmx::Object *);

Loader *ResourceFactory(const FilePath &f, LoaderPos p) {
    return new FileLoader(
        f, CacheResource(f.c_str(), nullptr), p, 0, false, true, nullptr, nullptr
    );
}

void RndUtlPreInit() {
    SystemConfig("rnd")->FindData("limit_uv_range", gLimitUVRange, true);
    TheLoadMgr.RegisterFactory("bmp", ResourceFactory);
    TheLoadMgr.RegisterFactory("png", ResourceFactory);
    TheLoadMgr.RegisterFactory("xbv", ResourceFactory);
    TheLoadMgr.RegisterFactory("jpg", ResourceFactory);
    TheLoadMgr.RegisterFactory("tif", ResourceFactory);
    TheLoadMgr.RegisterFactory("tiff", ResourceFactory);
    TheLoadMgr.RegisterFactory("psd", ResourceFactory);
    TheLoadMgr.RegisterFactory("gif", ResourceFactory);
    TheLoadMgr.RegisterFactory("tga", ResourceFactory);
    DataRegisterFunc("find_environ", DataFindEnviron);
    DataRegisterFunc("group_owner", OnGroupOwner);
}

void RndUtlInit() {
    FileCache::RegisterResourceCacheHelper(&gResourceFileCacheHelper);
    if (!UsingCD()) {
        sCylinderDir = DirLoader::LoadObjects(
            FilePath(FileSystemRoot(), "rndobj/cylinder.milo"), 0, 0
        );
    }
    sSphereDir =
        DirLoader::LoadObjects(FilePath(FileSystemRoot(), "rndobj/sphere.milo"), 0, 0);
    if (sSphereDir) {
        sSphereMesh = sSphereDir->Find<RndMesh>("sphere.mesh", true);
    }
    if (sCylinderDir) {
        sCylinderMesh = sSphereDir->Find<RndMesh>("Cylinder.mesh", true);
    }
}

void RndUtlTerminate() {
    RELEASE(sSphereDir);
    sSphereMesh = nullptr;
    RELEASE(sCylinderDir);
    sCylinderMesh = nullptr;
}

MatShaderOptions GetDefaultMatShaderOpts(const Hmx::Object *obj, RndMat *mat) {
    MatShaderOptions opts;
    const RndMesh *mesh = dynamic_cast<const RndMesh *>(obj);
    if (mesh) {
        if (mesh->Mat() == mat) {
            opts.SetLast5(0x12);
            opts.SetHasBones(mesh->NumBones() != 0);
            opts.SetHasAOCalc(mesh->HasAOCalc());
        }
    } else {
        const RndMultiMesh *multimesh = dynamic_cast<const RndMultiMesh *>(obj);
        if (multimesh) {
            const RndMesh *mesh = multimesh->Mesh();
            if (mesh && mesh->Mat()) {
                if (mesh->Mat() == mat) {
                    int mask = mesh->TransConstraint()
                            == RndTransformable::kConstraintFastBillboardXYZ
                        ? 0xD
                        : 0xC;
                    opts.SetLast5(mask);
                    opts.SetHasBones(false);
                    opts.SetHasAOCalc(mesh->HasAOCalc());
                }
            }
        } else {
            const RndParticleSys *partSys = dynamic_cast<const RndParticleSys *>(obj);
            if (partSys) {
                if (partSys->GetMat() == mat) {
                    opts.SetLast5(0xE);
                }
            } else {
                const RndFlare *flare = dynamic_cast<const RndFlare *>(obj);
                if (flare) {
                    if (flare->GetMat() == mat) {
                        opts.SetLast5(6);
                    }
                }
            }
        }
    }
    return opts;
}

const char *MovieExtension(const char *name, Platform p) {
    const char *ext;
    if (stricmp(name, "xbv") == 0) {
        // xbox, pc, ps3, or wii only
        if (p >= kPlatformXBox && p <= kPlatformWii) {
            return "xbv";
        }
        return name;
    } else
        return nullptr;
}

float ConvertFov(float a, float b) {
    float x = tanf(0.5f * a);
    return atanf(b * x) * 2;
}

void PreMultiplyAlpha(Hmx::Color &c) {
    c.red *= c.alpha;
    c.green *= c.alpha;
    c.blue *= c.alpha;
}

int GenerationCount(RndTransformable *t1, RndTransformable *t2) {
    if (t1 && t2) {
        int count = 0;
        for (; t2 != nullptr; t2 = t2->TransParent()) {
            if (t2 == t1)
                return count;
            count++;
        }
    }
    return 0;
}

void CreateAndSetMetaMat(RndMat *mat) {
    MILO_ASSERT(mat, 0x124A);
    if (!mat->GetMetaMaterial()) {
        MetaMaterial *metaMat = mat->CreateMetaMaterial(false);
        mat->SetMetaMat(metaMat, true);
    }
}

bool ShouldStrip(RndTransformable *trans) {
    if (trans) {
        const char *name = trans->Name();
        if (name) {
            return strnicmp("bone_", name, 5) == 0 || strnicmp("exo_", name, 4) == 0
                || strncmp("spot_", name, 5) == 0;
        } else
            return false;
    } else
        return false;
}

bool AnimContains(const RndAnimatable *anim1, const RndAnimatable *anim2) {
    if (anim1 == anim2)
        return true;
    else {
        std::list<RndAnimatable *> children;
        anim1->ListAnimChildren(children);
        for (std::list<RndAnimatable *>::iterator it = children.begin();
             it != children.end();
             ++it) {
            if (AnimContains(*it, anim2))
                return true;
        }
        return false;
    }
}

RndMat *GetMat(RndDrawable *draw) {
    std::list<RndMat *> mats;
    draw->Mats(mats, false);
    RndMat *ret;
    if (mats.empty())
        ret = 0;
    else
        ret = mats.front();
    return ret;
}

bool SortDraws(RndDrawable *draw1, RndDrawable *draw2) {
    if (draw1->GetOrder() != draw2->GetOrder())
        return draw1->GetOrder() < draw2->GetOrder();
    else {
        RndMat *mat1 = GetMat(draw1);
        RndMat *mat2 = GetMat(draw2);
        if (mat1 != mat2) {
            return mat1 < mat2;
        } else
            return strcmp(draw1->Name(), draw2->Name()) < 0;
    }
}

bool SortPolls(const RndPollable *p1, const RndPollable *p2) {
    if (p1->PollEnabled() != p2->PollEnabled()) {
        return p1->PollEnabled();
    } else {
        return strcmp(p1->Name(), p2->Name()) < 0;
    }
}

bool LeftHanded(const Hmx::Matrix3 &m) {
    Vector3 cross;
    Cross(m.x, m.y, cross);
    float det = Dot(m.z, cross);
    return det < 0;
}

float AngleBetween(const Hmx::Quat &q1, const Hmx::Quat &q2) {
    Hmx::Quat qtmp;
    Negate(q1, qtmp);
    Multiply(q2, qtmp, qtmp);
    if (qtmp.w > 1.0f) {
        return 0;
    } else {
        return acosf(qtmp.w) * 2.0f;
    }
}

bool BadUV(Vector2 &v) {
    if (fabsf(v.x) > 1000.0f || fabsf(v.y) > 1000.0f) {
        return true;
    } else {
        if (NearlyZero(v.x)) {
            v.x = 0;
        }
        if (NearlyZero(v.y)) {
            v.y = 0;
        }
        return false;
    }
}

void SetLocalScale(RndTransformable *t, const Vector3 &vec) {
    Hmx::Matrix3 m;
    Normalize(t->LocalXfm().m, m);
    Scale(vec, m, m);
    t->SetLocalRot(m);
}

void CalcBox(RndMesh *m, Box &b) {
    FOREACH (it, m->Verts()) {
        Vector3 vec;
        Multiply(it->pos, m->WorldXfm(), vec);
        b.GrowToContain(vec, it == m->Verts().begin());
    }
}

void ClearAO(RndMesh *m) {
    if (m->HasAOCalc()) {
        for (uint i = 0; i < m->Verts().size(); i++) {
            m->Verts(i).color.Set(1, 1, 1, 1);
        }
        m->SetHasAOCalc(false);
        m->Sync(0x1F);
    }
}

void ListDrawGroups(RndDrawable *draw, ObjectDir *dir, std::list<RndGroup *> &gList) {
    for (ObjDirItr<RndGroup> it(dir, true); it != 0; ++it) {
        if (VectorFind(it->Draws(), draw)) {
            gList.push_back(it);
        }
    }
}

void ResetColors(std::vector<Hmx::Color> &colors, int newNumColors) {
    Hmx::Color reset(1, 1, 1, 1);
    colors.resize(newNumColors);
    for (int i = 0; i < newNumColors; i++) {
        colors[i] = reset;
    }
}

void UtilDrawString(const char *c, const Vector3 &v, const Hmx::Color &col) {
    Vector2 v2;
    if (RndCam::Current()->WorldToScreen(v, v2) > 0) {
        v2.x *= TheRnd.Width();
        v2.y *= TheRnd.Height();
        TheRnd.DrawString(c, v2, col, true);
    }
}

void UtilDrawBox(const Transform &tf, const Box &box, const Hmx::Color &col, bool b4) {
    Vector3 vecs[8] = { Vector3(box.mMin.x, box.mMin.y, box.mMin.z),
                        Vector3(box.mMin.x, box.mMax.y, box.mMin.z),
                        Vector3(box.mMax.x, box.mMax.y, box.mMin.z),
                        Vector3(box.mMax.x, box.mMin.y, box.mMin.z),
                        Vector3(box.mMin.x, box.mMin.y, box.mMax.z),
                        Vector3(box.mMin.x, box.mMax.y, box.mMax.z),
                        Vector3(box.mMax.x, box.mMax.y, box.mMax.z),
                        Vector3(box.mMax.x, box.mMin.y, box.mMax.z) };
    for (int i = 0; i < 8; i++) {
        Multiply(vecs[i], tf, vecs[i]);
    }
    TheRnd.DrawLine(vecs[0], vecs[1], col, b4);
    TheRnd.DrawLine(vecs[1], vecs[2], col, b4);
    TheRnd.DrawLine(vecs[2], vecs[3], col, b4);
    TheRnd.DrawLine(vecs[3], vecs[0], col, b4);

    TheRnd.DrawLine(vecs[0], vecs[4], col, b4);
    TheRnd.DrawLine(vecs[1], vecs[5], col, b4);
    TheRnd.DrawLine(vecs[2], vecs[6], col, b4);
    TheRnd.DrawLine(vecs[3], vecs[7], col, b4);

    TheRnd.DrawLine(vecs[4], vecs[5], col, b4);
    TheRnd.DrawLine(vecs[5], vecs[6], col, b4);
    TheRnd.DrawLine(vecs[6], vecs[7], col, b4);
    TheRnd.DrawLine(vecs[7], vecs[4], col, b4);
}

void UtilDrawAxes(const Transform &tf, float f, const Hmx::Color &c) {
    Vector3 vec38;
    Hmx::Color c48;
    ScaleAdd(tf.v, tf.m.x, f, vec38);
    Interp(c, Hmx::Color(1, 0, 0), 0.8f, c48);
    TheRnd.DrawLine(tf.v, vec38, c48, false);

    ScaleAdd(tf.v, tf.m.y, f, vec38);
    Interp(c, Hmx::Color(0, 1, 0), 0.8f, c48);
    TheRnd.DrawLine(tf.v, vec38, c48, false);

    ScaleAdd(tf.v, tf.m.z, f, vec38);
    Interp(c, Hmx::Color(0, 0, 1), 0.8f, c48);
    TheRnd.DrawLine(tf.v, vec38, c48, false);
}

void UtilDrawLine(const Vector2 &v1, const Vector2 &v2, const Hmx::Color &color) {
    RndCam *cam = RndCam::Current();
    float planeRatio = (cam->FarPlane() - cam->NearPlane()) / 10.0f + cam->NearPlane();
    Vector3 v3_1, v3_2;
    cam->ScreenToWorld(v1, planeRatio, v3_1);
    cam->ScreenToWorld(v2, planeRatio, v3_2);
    TheRnd.DrawLine(v3_1, v3_2, color, false);
}

void UtilDrawRect2D(const Vector2 &v1, const Vector2 &v2, const Hmx::Color &color) {
    Vector2 cross1(v2.x, v1.y);
    Vector2 cross2(v1.x, v2.y);
    UtilDrawLine(v1, cross1, color);
    UtilDrawLine(cross1, v2, color);
    UtilDrawLine(v2, cross2, color);
    UtilDrawLine(cross2, v1, color);
}

void CalcSphere(RndTransAnim *a, Sphere &s) {
    s.Zero();
    if (!a->TransKeys().empty()) {
        RndTransformable *trans = a->Trans() ? a->Trans()->TransParent() : nullptr;
        Box box;
        Vector3 vec;
        FOREACH (it, a->TransKeys()) {
            if (trans) {
                Multiply(it->value, trans->WorldXfm(), vec);
            } else
                vec = it->value;
            box.GrowToContain(vec, it == a->TransKeys().begin());
        }
        Vector3 vres;
        CalcBoxCenter(vres, box);
        Subtract(box.mMax, vres, vec);
        Vector3 vsphere;
        float fmax = Max(vec.x, vec.y, vec.z);
        CalcBoxCenter(vsphere, box);
        s.Set(vsphere, fmax);
    }
}

void SpliceKeys(
    RndTransAnim *anim1, RndTransAnim *anim2, float firstFrame, float lastFrame
) {
    float start = anim1->StartFrame();
    float end = anim1->EndFrame();
    if (start < 0.0f || end > lastFrame)
        MILO_NOTIFY("%s has keyframes outside (0, %f)", anim1->Name(), lastFrame);
    else {
        RndTransformable *trans = anim1->Trans();
        if (!anim1->TransKeys().empty()) {
            if (anim1->TransKeys().front().frame != 0.0f) {
                anim1->TransKeys().Add(anim1->TransKeys().front().value, 0.0f, false);
            }
            if (anim1->TransKeys().back().frame != lastFrame) {
                anim1->TransKeys().Add(anim1->TransKeys().back().value, lastFrame, false);
            }
        } else if (trans) {
            anim1->TransKeys().Add(trans->LocalXfm().v, 0.0f, false);
            anim1->TransKeys().Add(trans->LocalXfm().v, lastFrame, false);
        } else {
            anim1->TransKeys().Add(Vector3(0.0f, 0.0f, 0.0f), 0.0f, false);
            anim1->TransKeys().Add(Vector3(0.0f, 0.0f, 0.0f), lastFrame, false);
        }

        if (!anim1->RotKeys().empty()) {
            if (anim1->RotKeys().front().frame != 0.0f) {
                anim1->RotKeys().Add(anim1->RotKeys().front().value, 0.0f, false);
            }
            if (anim1->RotKeys().back().frame != lastFrame) {
                anim1->RotKeys().Add(anim1->RotKeys().back().value, lastFrame, false);
            }
        } else if (trans) {
            Hmx::Quat q(trans->LocalXfm().m);
            anim1->RotKeys().Add(q, 0.0f, false);
            anim1->RotKeys().Add(q, lastFrame, false);
        } else {
            anim1->RotKeys().Add(Hmx::Quat(0.0f, 0.0f, 0.0f, 1.0f), 0.0f, false);
            anim1->RotKeys().Add(Hmx::Quat(0.0f, 0.0f, 0.0f, 1.0f), lastFrame, false);
        }

        if (!anim1->ScaleKeys().empty()) {
            if (anim1->ScaleKeys().front().frame != 0.0f) {
                anim1->ScaleKeys().Add(anim1->ScaleKeys().front().value, 0.0f, false);
            }
            if (anim1->ScaleKeys().back().frame != lastFrame) {
                anim1->ScaleKeys().Add(anim1->ScaleKeys().back().value, lastFrame, false);
            }
        } else if (trans) {
            Vector3 v;
            MakeScale(trans->LocalXfm().m, v);
            anim1->ScaleKeys().Add(v, 0.0f, false);
            anim1->ScaleKeys().Add(v, lastFrame, false);
        } else {
            anim1->ScaleKeys().Add(Vector3(1.0f, 1.0f, 1.0f), 0.0f, false);
            anim1->ScaleKeys().Add(Vector3(1.0f, 1.0f, 1.0f), lastFrame, false);
        }

        for (Keys<Vector3, Vector3>::iterator it = anim1->TransKeys().begin();
             it != anim1->TransKeys().end();
             it++) {
            (*it).frame += firstFrame;
        }
        for (Keys<Hmx::Quat, Hmx::Quat>::iterator it = anim1->RotKeys().begin();
             it != anim1->RotKeys().end();
             it++) {
            (*it).frame += firstFrame;
        }
        for (Keys<Vector3, Vector3>::iterator it = anim1->ScaleKeys().begin();
             it != anim1->ScaleKeys().end();
             it++) {
            (*it).frame += firstFrame;
        }

        float fsum = firstFrame + lastFrame;
        int transRemoved = anim2->TransKeys().Remove(firstFrame, fsum);
        int rotRemoved = anim2->RotKeys().Remove(firstFrame, fsum);
        int scaleRemoved = anim2->ScaleKeys().Remove(firstFrame, fsum);

        anim2->TransKeys().insert(
            anim2->TransKeys().begin() + transRemoved,
            anim1->TransKeys().begin(),
            anim1->TransKeys().end()
        );
        anim2->RotKeys().insert(
            anim2->RotKeys().begin() + rotRemoved,
            anim1->RotKeys().begin(),
            anim1->RotKeys().end()
        );
        anim2->ScaleKeys().insert(
            anim2->ScaleKeys().begin() + scaleRemoved,
            anim1->ScaleKeys().begin(),
            anim1->ScaleKeys().end()
        );
    }
}

void LinearizeKeys(
    RndTransAnim *anim, float f2, float f3, float f4, float firstFrame, float lastFrame
) {
    int firstFrameIdx, lastFrameIdx;
    if (f2) {
        if (anim->TransKeys().size() > 2) {
            Keys<Vector3, Vector3> vecKeys;
            anim->TransKeys().FindBounds(
                firstFrame, lastFrame, firstFrameIdx, lastFrameIdx
            );
            for (int i = firstFrameIdx + 1; i < lastFrameIdx - vecKeys.size();) {
                vecKeys.push_back(anim->TransKeys()[i]);
                anim->TransKeys().Remove(i);
                for (int j = 0; j < vecKeys.size(); j++) {
                    Vector3 vec;
                    InterpVector(
                        anim->TransKeys(), anim->TransSpline(), vecKeys[j].frame, vec, 0
                    );
                    Subtract(vec, vecKeys[j].value, vec);
                    if (Length(vec) > f2) {
                        anim->TransKeys().insert(
                            anim->TransKeys().begin() + i, vecKeys.back()
                        );
                        vecKeys.pop_back();
                        i++;
                        break;
                    }
                }
            }
        }
    }
    if (f3) {
        if (anim->RotKeys().size() > 2) {
            Keys<Hmx::Quat, Hmx::Quat> quatKeys;
            anim->RotKeys().FindBounds(firstFrame, lastFrame, firstFrameIdx, lastFrameIdx);
            for (int i = firstFrameIdx + 1; i < lastFrameIdx - quatKeys.size();) {
                quatKeys.push_back(anim->RotKeys()[i]);
                anim->RotKeys().Remove(i);
                for (int j = 0; j < quatKeys.size(); j++) {
                    Hmx::Quat q;
                    anim->RotKeys().AtFrame(quatKeys[j].frame, q);
                    if (AngleBetween(q, quatKeys[j].value) > f3) {
                        anim->RotKeys().insert(
                            anim->RotKeys().begin() + i, quatKeys.back()
                        );
                        quatKeys.pop_back();
                        i++;
                        break;
                    }
                }
            }
        }
    }
    if (f4) {
        if (anim->ScaleKeys().size() > 2) {
            Keys<Vector3, Vector3> vecKeys;
            anim->ScaleKeys().FindBounds(
                firstFrame, lastFrame, firstFrameIdx, lastFrameIdx
            );
            for (int i = firstFrameIdx + 1; i < lastFrameIdx - vecKeys.size();) {
                vecKeys.push_back(anim->ScaleKeys()[i]);
                anim->ScaleKeys().Remove(i);
                for (int j = 0; j < vecKeys.size(); j++) {
                    Vector3 vec;
                    InterpVector(
                        anim->ScaleKeys(), anim->ScaleSpline(), vecKeys[j].frame, vec, 0
                    );
                    Subtract(vec, vecKeys[j].value, vec);
                    if (Length(vec) > f4) {
                        anim->ScaleKeys().insert(
                            anim->ScaleKeys().begin() + i, vecKeys.back()
                        );
                        vecKeys.pop_back();
                        i++;
                        break;
                    }
                }
            }
        }
    }
}

void TransformKeys(RndTransAnim *tanim, const Transform &tf) {
    Vector3 v48;
    MakeScale(tf.m, v48);
    Hmx::Matrix3 m3c;
    Scale(tf.m.x, 1.0f / v48.x, m3c.x);
    Scale(tf.m.y, 1.0f / v48.y, m3c.y);
    Scale(tf.m.z, 1.0f / v48.z, m3c.z);
    Hmx::Quat q58(m3c);
    FOREACH (it, tanim->TransKeys()) {
        Multiply(it->value, tf, it->value);
    }
    FOREACH (it, tanim->ScaleKeys()) {
        Scale(it->value, v48.x, it->value);
    }
    FOREACH (it, tanim->RotKeys()) {
        Multiply(q58, it->value, it->value);
    }
}

void EndianSwapBitmap(RndBitmap &bmap) {
    for (int i = 0; i < bmap.Height(); i++) {
        u8 *curRow = bmap.Pixels() + bmap.RowBytes() * i - 4;
        for (int j = 0; j < bmap.Width(); j++) {
            // EndianSwap()
        }
    }
}

void Clip(BuildPoly &bp, const Plane &plane, bool b) {
    Hmx::Ray ray;
    if (fabs(
            bp.mTransform.m.z.x * plane.a + bp.mTransform.m.z.z * plane.c
            + bp.mTransform.m.z.y * plane.b
        )
        <= 0.9999f) {
        Intersect(bp.mTransform, plane, ray);
        if (b) {
            ray.dir.x = -ray.dir.x;
            ray.dir.y = -ray.dir.y;
        }
        Clip(bp.mPoly, ray, bp.mPoly);
    }
}

void ScrambleXfms(RndMultiMesh *mesh) {
    double scrambleMax = 6.2829999923706055;
    double scrambleMin = 0.0;
    double max = 1.0;
    double min = -1.0;
    FOREACH (it, mesh->Instances()) {
        float randZ = RandomFloat(min, max);
        float randY = RandomFloat(min, max);
        float randX = RandomFloat(min, max);
        Vector3 vec(randX, randY, randZ);
        Normalize(vec, vec);
        float scrambler = RandomFloat(scrambleMin, scrambleMax);
        Hmx::Quat q;
        q.Set(vec, scrambler);
        MakeRotMatrix(q, it->mXfm.m);
    }
}

void SortXfms(RndMultiMesh *mesh, const Vector3 &vec) {
    gUtlXfms = vec;
    mesh->Instances().sort(XfmSort);
    mesh->InvalidateProxies();
}

bool XfmSort(RndMultiMesh::Instance &mesh1, RndMultiMesh::Instance &mesh2) {
    return (mesh1.mXfm.v.z - gUtlXfms.z) * (mesh1.mXfm.v.z - gUtlXfms.z)
        + (mesh1.mXfm.v.y - gUtlXfms.y) * (mesh1.mXfm.v.y - gUtlXfms.y)
        + (mesh1.mXfm.v.x - gUtlXfms.x) * (mesh1.mXfm.v.x - gUtlXfms.x)
        < (mesh2.mXfm.v.z - gUtlXfms.z) * (mesh2.mXfm.v.z - gUtlXfms.z)
        + (mesh2.mXfm.v.y - gUtlXfms.y) * (mesh2.mXfm.v.y - gUtlXfms.y)
        + (mesh2.mXfm.v.x - gUtlXfms.x) * (mesh2.mXfm.v.x - gUtlXfms.x);
}