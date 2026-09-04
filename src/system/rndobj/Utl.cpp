#include "Rnd.h"
#include "math/Color.h"
#include "math/Geo.h"
#include "math/Key.h"
#include "math/Mtx.h"
#include "math/Utl.h"
#include "math/Trig.h"
#include "math/Vec.h"
#include "obj/DataFunc.h"
#include "obj/Object.h"
#include "obj/Utl.h"
#include "os/Debug.h"
#include "os/Endian.h"
#include "os/File.h"
#include "os/FileCache.h"
#include "os/HolmesClient.h"
#include "os/Platform.h"
#include "os/System.h"
#include "rndobj/AmbientOcclusion.h"
#include "rndobj/Bitmap.h"
#include "rndobj/Cam.h"
#include "rndobj/CamAnim.h"
#include "rndobj/Dir.h"
#include "rndobj/Draw.h"
#include "rndobj/Env.h"
#include "rndobj/Flare.h"
#include "rndobj/Gen.h"
#include "rndobj/Group.h"
#include "rndobj/Line.h"
#include "rndobj/Lit.h"
#include "rndobj/LitAnim.h"
#include "rndobj/Mat.h"
#include "rndobj/MatAnim.h"
#include "rndobj/Mesh.h"
#include "rndobj/MeshAnim.h"
#include "rndobj/MetaMaterial.h"
#include "rndobj/Morph.h"
#include "rndobj/MultiMesh.h"
#include "rndobj/Part.h"
#include "rndobj/PartAnim.h"
#include "rndobj/ShaderMgr.h"
#include "rndobj/Trans.h"
#include "rndobj/TransAnim.h"
#include "utl/Loader.h"
#include "utl/Std.h"
#include "utl/Cache.h"
#include "rndobj/Utl.h"
#include "math/Rand.h"
#include "utl/Str.h"
#include <set>

class ResourceFileCacheHelper : public FileCacheHelper {
public:
    virtual const char *CacheFile(const char *);
};

const char *ResourceFileCacheHelper::CacheFile(const char *c) {
    return CacheResource(c, nullptr);
}

static ResourceFileCacheHelper gResourceFileCacheHelper;
static float gLimitUVRange = 0;
static ObjectDir *sSphereDir = nullptr;
static RndMesh *sSphereMesh = nullptr;
static ObjectDir *sCylinderDir = nullptr;
static RndMesh *sCylinderMesh = nullptr;
SplashFunc gSplashPoll;
SplashFunc gSplashSuspend;
SplashFunc gSplashResume;
std::list<BuildPoly> gChildPolys;
std::list<BuildPoly> gParentPolys;
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

void SetRndSplasherCallback(
    SplashFunc pollFunc, SplashFunc suspendFunc, SplashFunc resumeFunc
) {
    gSplashPoll = pollFunc;
    gSplashSuspend = suspendFunc;
    gSplashResume = resumeFunc;
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

const char *MovieExtension(const char *name, Platform p) {
    const char *ext;
    if (strieq(name, "xbv")) {
        // xbox, pc, ps3, or wii only
        if (p >= kPlatformXBox && p <= kPlatformWii) {
            return "xbv";
        }
        return name;
    } else
        return nullptr;
}

const char *CacheResource(const char *file, CacheResourceResult &res) {
    Platform thisPlatform = TheLoadMgr.GetPlatform();
    res = kCacheUnnecessary;
    char locBuffer[256];
    const char *localized = FileLocalize(file, locBuffer);
    bool islocal = FileIsLocal(localized);
    const char *ext = FileGetExt(localized);

    if (strieq(ext, "bmp") || strieq(ext, "png")) {
        const char *finalFileName = localized;
        char ps3File[256];
        if (TheLoadMgr.GetPlatform() == kPlatformPS3) {
            const char *xboxStr = &finalFileName[strlen(finalFileName) - 5];
            if (xboxStr >= finalFileName && streq("_xbox", xboxStr)) {
                strcpy(ps3File, finalFileName);
                int ps3Idx = xboxStr - finalFileName;
                strcpy(ps3File + ps3Idx, "_ps3");
                strcpy(ps3File + ps3Idx + 4, xboxStr + 5);
                finalFileName = ps3File;
            }
        }

        static char finalFileBuffer[0x100];
        strcpy(
            finalFileBuffer,
            MakeString(
                "%s/gen/%s.%s_%s",
                FileGetPath(finalFileName),
                FileGetBase(finalFileName),
                FileGetExt(finalFileName),
                PlatformSymbol(thisPlatform)
            )
        );
        if (!UsingCD() && !islocal) {
            String str;
            FileQualifiedFilename(str, finalFileName);
            res = HolmesClientCacheResource(str.c_str(), finalFileBuffer);
            if (res > 0) {
                return nullptr;
            } else {
                return finalFileBuffer;
            }
        }
        return finalFileBuffer;
    } else {
        const char *movieExt = MovieExtension(ext, thisPlatform);
        if (movieExt) {
            return MakeString(
                "%s/%s.%s", FileGetPath(localized), FileGetBase(localized), movieExt
            );
        } else {
            res = kCacheUnknownExtension;
            return nullptr;
        }
    }
}

const char *CacheResource(const char *file, const Hmx::Object *obj) {
    if (file && *file) {
        strstr(file, "icons_buttons_xbox_nomip");
        CacheResourceResult res;
        const char *ret = CacheResource(file, res);
        if (res > kCacheUnnecessary) {
            switch (res) {
            case kCacheUnknownExtension:
                if (obj) {
                    MILO_NOTIFY(
                        "%s: \"%s\" has unrecognized extension \"%s\"",
                        PathName(obj),
                        file,
                        FileGetExt(file)
                    );
                } else {
                    MILO_NOTIFY(
                        "Unrecognized extension \"%s\" to \"%s\"", FileGetExt(file), file
                    );
                }
                break;
            case kCacheMissingFile:
                if (obj) {
                    MILO_NOTIFY("%s: couldn't find %s", PathName(obj), file);
                } else {
                    MILO_NOTIFY("Couldn't find %s", file);
                }
                break;
            default:
                if (obj) {
                    MILO_NOTIFY("%s: unknown CacheResource error %s", PathName(obj), file);
                } else {
                    MILO_NOTIFY("Unknown CacheResource error %s", file);
                }
                break;
            }
        }
        return ret;
    } else {
        return nullptr;
    }
}

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
        sCylinderMesh = sCylinderDir->Find<RndMesh>("Cylinder.mesh", true);
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
            opts.shader_struct.mHasBones = mesh->NumBones() != 0;
            opts.shader_struct.mHasAOCalc = mesh->HasAOCalc();
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
    if (!trans) {
        return false;
    }
    const char *name = trans->Name();
    if (!name) {
        return false;
    }
    return strnicmp("bone_", name, 5) == 0 || strnicmp("exo_", name, 4) == 0
        || strncmp("spot_", name, 5) == 0;
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
    if (IsNaN(v.x) || IsNaN(v.y) || fabsf(v.x) > 1000.0f || fabsf(v.y) > 1000.0f) {
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

void UtilDrawCigar(
    const Transform &t1,
    const float *const scales,
    const float *const lengths,
    const Hmx::Color &c,
    int
) {
    float lens[2];
    for (int i = 0; i < 2; i++) {
        lens[i] = lengths[i] * Length(t1.m.x);
    }
    Transform xfm(t1);
    Normalize(xfm.m, xfm.m);
    Vector3 mults[2];
    Multiply(Vector3(lens[0] - scales[0], 0, 0), xfm, mults[0]);
    Multiply(Vector3(lens[1] + scales[1], 0, 0), xfm, mults[1]);

    Vector3 f1c0[3][6];
    Vector3 f2e0[3][6];

    for (int i = 0; i < 3; i++) {
        float f15 = (float)i * (PI / 6);
        float f18 = FastCos(f15) * scales[0];
        float f13 = FastSin(f15) * scales[0];
        float f16 = FastCos(f15) * scales[1];
        float f19 = FastSin(f15) * scales[1];
        for (int j = 0; j < 6; j++) {
            float f10 = (float)j * (PI / 3);
            float s10 = FastSin(f10);
            float c10 = FastCos(f10);
            Multiply(Vector3(lens[0] - f13, c10 * f18, s10 * f18), xfm, f1c0[i][j]);
            Multiply(Vector3(lens[1] + f19, c10 * f16, s10 * f16), xfm, f2e0[i][j]);
        }
    }
    for (int i = 0; i < 6; i++) {
        TheRnd.DrawLine(f2e0[0][i], f1c0[0][i], c, false);
    }
    for (int i = 0; i < 3; i++) {
        int jOffset = 5;
        for (int j = 0; j < 6; j++) {
            TheRnd.DrawLine(f2e0[i][j], f2e0[i][jOffset], c, false);
            TheRnd.DrawLine(f2e0[i][j], i == 2 ? mults[1] : f2e0[i + 1][j], c, false);
            TheRnd.DrawLine(f1c0[i][j], f1c0[i][jOffset], c, false);
            TheRnd.DrawLine(f1c0[i][j], i == 2 ? mults[0] : f1c0[i + 1][j], c, false);
            jOffset = j;
        }
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

void UtilDrawBox(const Transform &tf, const Box &box, const Hmx::Color &col, bool no_z) {
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
    TheRnd.DrawLine(vecs[0], vecs[1], col, no_z);
    TheRnd.DrawLine(vecs[1], vecs[2], col, no_z);
    TheRnd.DrawLine(vecs[2], vecs[3], col, no_z);
    TheRnd.DrawLine(vecs[3], vecs[0], col, no_z);

    TheRnd.DrawLine(vecs[0], vecs[4], col, no_z);
    TheRnd.DrawLine(vecs[1], vecs[5], col, no_z);
    TheRnd.DrawLine(vecs[2], vecs[6], col, no_z);
    TheRnd.DrawLine(vecs[3], vecs[7], col, no_z);

    TheRnd.DrawLine(vecs[4], vecs[5], col, no_z);
    TheRnd.DrawLine(vecs[5], vecs[6], col, no_z);
    TheRnd.DrawLine(vecs[6], vecs[7], col, no_z);
    TheRnd.DrawLine(vecs[7], vecs[4], col, no_z);
}

void UtilDrawAxes(const Transform &tf, float scale, const Hmx::Color &c) {
    Vector3 vec38;
    Hmx::Color c48;
    ScaleAdd(tf.v, tf.m.x, scale, vec38);
    Interp(c, Hmx::Color(1, 0, 0), 0.8f, c48);
    TheRnd.DrawLine(tf.v, vec38, c48, false);

    ScaleAdd(tf.v, tf.m.y, scale, vec38);
    Interp(c, Hmx::Color(0, 1, 0), 0.8f, c48);
    TheRnd.DrawLine(tf.v, vec38, c48, false);

    ScaleAdd(tf.v, tf.m.z, scale, vec38);
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

void UtilDrawPlane(
    const Plane &p,
    const Vector3 &v,
    const Hmx::Color &c,
    int ringct,
    float ringscl,
    bool no_z
) {
    Transform xfm;
    ScaleAdd(v, reinterpret_cast<const Vector3 &>(p), -p.Dot(v), xfm.v);
    xfm.m.y = reinterpret_cast<const Vector3 &>(p);
    Hmx::Matrix3 mb0;
    mb0.Identity();
    int idx = 0;
    float threshold = 10000;
    for (int i = 0; i < 3; i++) {
        if (MinEq(threshold, Dot(mb0[i], xfm.m.y))) {
            idx = i;
        }
    }
    Cross(xfm.m.y, mb0[idx], xfm.m.z);
    Normalize(xfm.m.z, xfm.m.z);
    Cross(xfm.m.y, xfm.m.z, xfm.m.x);
    for (int i = 0; i < ringct; i++) {
        Vector3 v[4];
        float scalar = (float)(i + 1) * ringscl;
        ScaleAdd(xfm.v, xfm.m.x, scalar, v[0]);
        ScaleAdd(xfm.v, xfm.m.z, scalar, v[1]);
        ScaleAdd(xfm.v, xfm.m.x, -scalar, v[2]);
        ScaleAdd(xfm.v, xfm.m.z, -scalar, v[3]);
        TheRnd.DrawLine(v[0], v[1], c, no_z);
        TheRnd.DrawLine(v[1], v[2], c, no_z);
        TheRnd.DrawLine(v[2], v[3], c, no_z);
        TheRnd.DrawLine(v[3], v[0], c, no_z);
    }
}

void UtilDrawCircle2D(const Vector2 &v2, float scale, const Hmx::Color &c, int vertct) {
    std::vector<Vector2> vec(vertct + 1);
    float y = TheRnd.YRatio();
    for (int i = 0; i <= vertct; i++) {
        Vector2 &cur = vec[i];
        float fi = (float)i * (2 * PI) / (float)vertct;
        float cos = FastCos(fi);
        float sin = FastSin(fi);
        cur.x = (cos * y) * scale + v2.x;
        cur.y = sin * scale + v2.y;
    }
    for (int i = 0; i < vertct; i++) {
        UtilDrawLine(vec[i], vec[i + 1], c);
    }
}

void UtilDrawSphere(const Vector3 &v1, float scale, const Hmx::Color &c, RndMat *mat) {
    if (!sSphereMesh) {
        MILO_NOTIFY_ONCE("Sphere mesh is not loaded");
    } else {
        RndMat *oldMat = sSphereMesh->Mat();
        Transform tf;
        tf.Reset();
        tf.v = v1;
        Scale(Vector3(scale, scale, scale), tf.m, tf.m);
        if (mat) {
            sSphereMesh->SetMat(mat);
        } else {
            sSphereMesh->Mat()->SetColor(c.red, c.green, c.blue);
            sSphereMesh->Mat()->SetAlpha(0.2f);
            sSphereMesh->Mat()->SetCull(kCullNone);
        }
        sSphereMesh->SetLocalXfm(tf);
        sSphereMesh->SetSphere(Sphere(Vector3(0, 0, 0), scale));
        sSphereMesh->Draw();
        if (mat) {
            sSphereMesh->SetMat(oldMat);
        }
    }
}

void UtilDrawCylinder(
    const Transform &xfm, float base, float height, const Hmx::Color &c, int
) {
    if (!sCylinderMesh) {
        MILO_NOTIFY_ONCE("Sphere mesh is not loaded");
    } else {
        Transform tfb0 = xfm;
        sCylinderMesh->Mat()->SetColor(c.red, c.green, c.blue);
        sCylinderMesh->Mat()->SetAlpha(0.2f);
        Scale(Vector3(height, base, base), tfb0.m, tfb0.m);
        sCylinderMesh->Mat()->SetCull(kCullNone);
        sCylinderMesh->SetLocalXfm(tfb0);
        sCylinderMesh->SetSphere(Sphere(Vector3(0, 0, 0), base));
        sCylinderMesh->Draw();
    }
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

void SpliceKeys(RndTransAnim *in, RndTransAnim *exist, float offset, float length) {
    float start = in->StartFrame();
    float end = in->EndFrame();
    if (start < 0.0f || end > length)
        MILO_NOTIFY("%s has keyframes outside (0, %f)", in->Name(), length);
    else {
        RndTransformable *trans = in->Trans();
        if (!in->TransKeys().empty()) {
            if (in->TransKeys().front().frame != 0.0f) {
                in->TransKeys().Add(in->TransKeys().front().value, 0.0f, false);
            }
            if (in->TransKeys().back().frame != length) {
                in->TransKeys().Add(in->TransKeys().back().value, length, false);
            }
        } else if (trans) {
            in->TransKeys().Add(trans->LocalXfm().v, 0.0f, false);
            in->TransKeys().Add(trans->LocalXfm().v, length, false);
        } else {
            in->TransKeys().Add(Vector3(0.0f, 0.0f, 0.0f), 0.0f, false);
            in->TransKeys().Add(Vector3(0.0f, 0.0f, 0.0f), length, false);
        }

        if (!in->RotKeys().empty()) {
            if (in->RotKeys().front().frame != 0.0f) {
                in->RotKeys().Add(in->RotKeys().front().value, 0.0f, false);
            }
            if (in->RotKeys().back().frame != length) {
                in->RotKeys().Add(in->RotKeys().back().value, length, false);
            }
        } else if (trans) {
            Hmx::Quat q(trans->LocalXfm().m);
            in->RotKeys().Add(q, 0.0f, false);
            in->RotKeys().Add(q, length, false);
        } else {
            in->RotKeys().Add(Hmx::Quat(0.0f, 0.0f, 0.0f, 1.0f), 0.0f, false);
            in->RotKeys().Add(Hmx::Quat(0.0f, 0.0f, 0.0f, 1.0f), length, false);
        }

        if (!in->ScaleKeys().empty()) {
            if (in->ScaleKeys().front().frame != 0.0f) {
                in->ScaleKeys().Add(in->ScaleKeys().front().value, 0.0f, false);
            }
            if (in->ScaleKeys().back().frame != length) {
                in->ScaleKeys().Add(in->ScaleKeys().back().value, length, false);
            }
        } else if (trans) {
            Vector3 v;
            MakeScale(trans->LocalXfm().m, v);
            in->ScaleKeys().Add(v, 0.0f, false);
            in->ScaleKeys().Add(v, length, false);
        } else {
            in->ScaleKeys().Add(Vector3(1.0f, 1.0f, 1.0f), 0.0f, false);
            in->ScaleKeys().Add(Vector3(1.0f, 1.0f, 1.0f), length, false);
        }

        for (Keys<Vector3, Vector3>::iterator it = in->TransKeys().begin();
             it != in->TransKeys().end();
             it++) {
            (*it).frame += offset;
        }
        for (Keys<Hmx::Quat, Hmx::Quat>::iterator it = in->RotKeys().begin();
             it != in->RotKeys().end();
             it++) {
            (*it).frame += offset;
        }
        for (Keys<Vector3, Vector3>::iterator it = in->ScaleKeys().begin();
             it != in->ScaleKeys().end();
             it++) {
            (*it).frame += offset;
        }

        float fsum = offset + length;
        int transRemoved = exist->TransKeys().Remove(offset, fsum);
        int rotRemoved = exist->RotKeys().Remove(offset, fsum);
        int scaleRemoved = exist->ScaleKeys().Remove(offset, fsum);

        exist->TransKeys().insert(
            exist->TransKeys().begin() + transRemoved,
            in->TransKeys().begin(),
            in->TransKeys().end()
        );
        exist->RotKeys().insert(
            exist->RotKeys().begin() + rotRemoved,
            in->RotKeys().begin(),
            in->RotKeys().end()
        );
        exist->ScaleKeys().insert(
            exist->ScaleKeys().begin() + scaleRemoved,
            in->ScaleKeys().begin(),
            in->ScaleKeys().end()
        );
    }
}

void LinearizeKeys(
    RndTransAnim *anim, float transTol, float rotTol, float scaleTol, float start, float end
) {
    int firstFrameIdx, lastFrameIdx;
    if (transTol) {
        if (anim->TransKeys().size() > 2) {
            Keys<Vector3, Vector3> vecKeys;
            anim->TransKeys().FindBounds(start, end, firstFrameIdx, lastFrameIdx);
            for (int i = firstFrameIdx + 1; i < lastFrameIdx - vecKeys.size();) {
                vecKeys.push_back(anim->TransKeys()[i]);
                anim->TransKeys().Remove(i);
                for (int j = 0; j < vecKeys.size(); j++) {
                    Vector3 vec;
                    InterpVector(
                        anim->TransKeys(), anim->TransSpline(), vecKeys[j].frame, vec, 0
                    );
                    Subtract(vec, vecKeys[j].value, vec);
                    if (Length(vec) > transTol) {
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
    if (rotTol) {
        if (anim->RotKeys().size() > 2) {
            Keys<Hmx::Quat, Hmx::Quat> quatKeys;
            anim->RotKeys().FindBounds(start, end, firstFrameIdx, lastFrameIdx);
            for (int i = firstFrameIdx + 1; i < lastFrameIdx - quatKeys.size();) {
                quatKeys.push_back(anim->RotKeys()[i]);
                anim->RotKeys().Remove(i);
                for (int j = 0; j < quatKeys.size(); j++) {
                    Hmx::Quat q;
                    anim->RotKeys().AtFrame(quatKeys[j].frame, q);
                    if (AngleBetween(q, quatKeys[j].value) > rotTol) {
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
    if (scaleTol) {
        if (anim->ScaleKeys().size() > 2) {
            Keys<Vector3, Vector3> vecKeys;
            anim->ScaleKeys().FindBounds(start, end, firstFrameIdx, lastFrameIdx);
            for (int i = firstFrameIdx + 1; i < lastFrameIdx - vecKeys.size();) {
                vecKeys.push_back(anim->ScaleKeys()[i]);
                anim->ScaleKeys().Remove(i);
                for (int j = 0; j < vecKeys.size(); j++) {
                    Vector3 vec;
                    InterpVector(
                        anim->ScaleKeys(), anim->ScaleSpline(), vecKeys[j].frame, vec, 0
                    );
                    Subtract(vec, vecKeys[j].value, vec);
                    if (Length(vec) > scaleTol) {
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
        Scale(it->value, v48, it->value);
    }
    FOREACH (it, tanim->RotKeys()) {
        Multiply(it->value, q58, it->value);
    }
}

void EndianSwapBitmap(RndBitmap &bmap) {
    for (int i = 0; i < bmap.Height(); i++) {
        unsigned int *curRow = (unsigned int *)(bmap.Pixels() + bmap.RowBytes() * i);
        for (int j = 0; j < bmap.Width(); j++) {
            EndianSwapEq(curRow[j]);
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
    Vector3 diff1;
    Subtract(mesh1.mXfm.v, gUtlXfms, diff1);
    Vector3 diff2;
    Subtract(mesh2.mXfm.v, gUtlXfms, diff2);
    return LengthSquared(diff1) < LengthSquared(diff2);
}

void DistributeXfms(RndMultiMesh *mesh, int i1, float f1) {
    int index = 0;
    FOREACH (it, mesh->Instances()) {
        int row = index / i1;
        int col = index % i1;
        Vector3 temp(col * f1, row * f1, 0.0f);
        Add(it->mXfm.v, temp, it->mXfm.v);
        index++;
    }
}

void MoveXfms(RndMultiMesh *mesh, const Vector3 &v3) {
    FOREACH (it, mesh->Instances()) {
        Add(it->mXfm.v, v3, it->mXfm.v);
    }
}

void ScaleXfms(RndMultiMesh *mesh, const Vector3 &v3) {
    FOREACH (it, mesh->Instances()) {
        Scale(v3, it->mXfm.m, it->mXfm.m);
    }
}

void ComputeFaceTangentBasis(RndMesh *m, int faceIdx, Hmx::Matrix3 &mtx) {
    MILO_ASSERT(m, 592);
    auto &face = m->Faces(faceIdx);
    mtx.Identity();
    unsigned short face1 = face.v1;
    unsigned short face2 = face.v2;
    unsigned short face3 = face.v3;
    if (face1 != face2 && face2 != face3 && face3 != face1) {
        auto &verts = m->Verts();
        auto &vert1 = verts[face1];
        auto &vert2 = verts[face2];
        auto &vert3 = verts[face3];
        Vector2 tex1 = vert1.tex;
        Vector2 tex2 = vert2.tex;
        Vector2 tex3 = vert3.tex;
        if (!BadUV(tex1) && !BadUV(tex2) && !BadUV(tex3)) {
            Vector3 diff21;
            Subtract(vert2.pos, vert1.pos, diff21);
            Vector3 diff31;
            Subtract(vert3.pos, vert1.pos, diff31);
            Vector2 diff21tex(tex2.x - tex1.x, tex2.y - tex1.y);
            Vector2 diff31tex(tex3.x - tex1.x, tex3.y - tex1.y);
            if (!(diff21 == Vector3(0, 0, 0)) && !(diff31 == Vector3(0, 0, 0))
                && !(diff21tex == Vector2(0, 0)) && !(diff31tex == Vector2(0, 0))) {
                Vector3 diffz;
                Cross(diff21, diff31, diffz);
                Hmx::Matrix3 diffMtx(diff21, diff31, diffz);
                Invert(diffMtx, diffMtx);
                Hmx::Matrix3 mb0(
                    diff21tex.x, diff31tex.x, 0, diff21tex.y, diff31tex.y, 0, 0, 0, 1
                );
                Transpose(diffMtx, diffMtx);
                Multiply(mb0, diffMtx, mtx);
            }
        } else {
            MILO_LOG("NOTIFY: %s has bad UVs, should reexport from Max\n", PathName(m));
        }
    }
}

void FixVertOrder(const RndMesh *src, RndMesh *dst) {
    int mismatches = 0;
    auto &src_verts = const_cast<RndMesh *>(src)->Verts();
    auto &dst_verts = dst->Verts();
    auto &dst_faces = dst->Faces();
    int num_verts = src_verts.size();
    for (int i = 0; i < num_verts; i++) {
        int idx = 0;
        Vector2 tex = src_verts[i].tex;
        for (int j = 0; j < dst_verts.size(); j++) {
            const Vector2 &vertTex = dst_verts[j].tex;
            if (std::fabs(tex.x - vertTex.x) < 1e-5f
                && std::fabs(tex.y - vertTex.y) < 1e-5f) {
                idx = j;
                goto out;
            }
        }
        idx = -1;
    out:
        if (idx != -1) {
            unsigned short ui = i;
            unsigned short uj = idx;
            if (uj != ui) {
                auto tmp = dst_verts[uj];
                dst_verts[uj] = dst_verts[ui];
                dst_verts[ui] = tmp;
            }
            if (uj != ui) {
                int num_faces = dst_faces.size();
                for (int k = 0; k < num_faces; k++) {
                    auto &face = dst_faces[k];
                    if (face.v1 == uj) {
                        face.v1 = ui;
                    } else if (face.v1 == ui) {
                        face.v1 = uj;
                    }
                    if (face.v2 == uj) {
                        face.v2 = ui;
                    } else if (face.v2 == ui) {
                        face.v2 = uj;
                    }
                    if (face.v3 == uj) {
                        face.v3 = ui;
                    } else if (face.v3 == ui) {
                        face.v3 = uj;
                    }
                }
            }
        } else {
            mismatches++;
        }
    }
    if (mismatches != 0) {
        MILO_LOG("%s has %d mismatched verts\n", dst->Name(), mismatches);
    }
}

void BurnXfm(RndMesh *mesh, bool zero) {
    Transform xfm(mesh->LocalXfm());
    if (zero) {
        xfm.v.Zero();
    }
    Hmx::Matrix3 inv;
    Invert(xfm.m, inv);
    Transpose(inv, inv);
    FOREACH (it, mesh->Verts()) {
        Multiply(it->pos, xfm, it->pos);
        Multiply(inv, it->norm, it->norm);
        Normalize(it->norm, it->norm);
        Vector3 &tangent = reinterpret_cast<Vector3 &>(it->tangent);
        Multiply(inv, tangent, tangent);
        Normalize(tangent, tangent);
    }
    mesh->Sync(0x1F);
    MultiplyEq(mesh->GetBSPTree(), xfm);
    Sphere s;
    Multiply(mesh->GetSphere(), xfm, s);
    mesh->SetSphere(s);
    xfm.Reset();
    if (zero) {
        xfm.v = mesh->LocalXfm().v;
    }
    mesh->SetLocalXfm(xfm);
}

void SetBloomBlurWeights(bool b1, float texWidth, float texHeight) {
    static float sFloats1[] = { 0.015928393f, 0.027077837f, 0.042423189f, 0.061254792f,
                                0.081512496f, 0.099966787f, 0.11298861f,  0.11769579f,
                                0.11298861f,  0.099966787f, 0.081512496f, 0.061254792f,
                                0.042423189f, 0.027077837f, 0.015928393f };
    static float sFloats2[] = { -6.5f, -5.5f, -4.5f, -3.5f, -2.5f, -1.5f, -0.5f, 0.5f,
                                1.5f,  2.5f,  3.5f,  4.5f,  5.5f,  6.5f,  7.5f };
    float div2 = 1 / texWidth;
    float div3 = 1 / texHeight;
    TheShaderMgr.SetNumTaps(DIM(sFloats2));
    for (int i = 0; i < DIM(sFloats2); i++) {
        float x, y;
        if (b1) {
            x = sFloats2[i] * div2;
            y = 0;
        } else {
            x = 0;
            y = sFloats2[i] * div3;
        }
        TheShaderMgr.SetPConstant((PShaderConstant)(0x8A + i), Vector4(x, y, 1, 1));
        TheShaderMgr.SetPConstant(
            (PShaderConstant)(0x9A + i),
            Vector4(sFloats1[i], sFloats1[i], sFloats1[i], sFloats1[i])
        );
    }
}

#define kNumBloomTaps 7U

void SetBloomBlurWeightsStreak(bool b1, float f2, float f3, float f4, int pass, float f6) {
    MILO_ASSERT(pass >= 0 && pass < 3, 0x11AA);

    float f14 = powf(f4, powf(4, pass));
    float f17 = powf(4, pass);

    float fc0[kNumBloomTaps];
    float fe0[kNumBloomTaps];

    float f11 = f14;
    float f10 = f17;

    int middle = kNumBloomTaps / 2;
    for (int i = 0; i <= 2; i++) {
        MILO_ASSERT((middle - i) >= 0 && (middle + i) < kNumBloomTaps, 0x11C5);
        fc0[middle - i] = f11 / 3;
        fe0[middle - i] = 0.5f - f10;
        fc0[middle + i] = f11 / 3;
        fe0[middle + i] = f10 + 0.5f;
        f11 *= f14;
        f10 += f17;
    }

    TheShaderMgr.SetNumTaps(kNumBloomTaps);

    float div2 = 1 / f2;
    float div3 = 1 / f3;
    float rnddiv = (float)TheRnd.Height() / (float)TheRnd.Width();

    float sinRad = sin(f6 * DEG2RAD);
    float cosRad = cos(f6 * DEG2RAD);

    for (int i = 0; i < kNumBloomTaps; i++) {
        float x, y;
        if (b1) {
            float cur = fe0[i] * div2;
            x = cur * cosRad * rnddiv;
            y = cur * sinRad;
        } else {
            float cur = fe0[i] * div3;
            x = -cur * sinRad * rnddiv;
            y = cur * cosRad;
        }
        TheShaderMgr.SetPConstant((PShaderConstant)(0x8A + i), Vector4(x, y, 1, 1));
        TheShaderMgr.SetPConstant(
            (PShaderConstant)(0x9A + i), Vector4(fc0[i], fc0[i], fc0[i], fc0[i])
        );
    }
}

void RandomXfms(RndMultiMesh *multiMesh) {
    InstanceList randomized;
    FOREACH (it, multiMesh->Instances()) {
        int idx = RandomInt(0, multiMesh->Instances().size());
        randomized.splice(
            randomized.begin(),
            multiMesh->Instances(),
            NextItr(multiMesh->Instances().begin(), idx)
        );
    }
    multiMesh->Instances().splice(multiMesh->Instances().begin(), randomized);
    multiMesh->InvalidateProxies();
}

void TestTextureSize(ObjectDir *dir, int iType, int i3, int i4, int i5, int maxBpp) {
    bool rendered = iType == RndTex::kRendered || iType == RndTex::kRenderedNoZ;
    bool b2 = GetGfxMode() == kOldGfx || rendered;
    int ivar4 = b2 ? i5 : 1;
    int inputProduct = i3 * i4 * ivar4;
    for (ObjDirItr<RndTex> it(dir, true); it != nullptr; ++it) {
        if (it->GetType() == iType) {
            int local_bpp = b2 ? it->Bpp() : 1;
            if (rendered && GetGfxMode() == 1 && local_bpp == 0x10)
                local_bpp = 0x20;
            int product = it->Width() * it->Height() * local_bpp;
            if (product > inputProduct) {
                MILO_NOTIFY(
                    "%s is too big w:%d h:%d bpp:%d",
                    PathName(it),
                    it->Width(),
                    it->Height(),
                    local_bpp
                );
            }
            if (product != 0 && b2 && local_bpp > maxBpp) {
                MILO_NOTIFY("%s is %d bpp > %d, too big", PathName(it), local_bpp, maxBpp);
            }
        }
    }
}

void TestTexturePaths(ObjectDir *dir) {
    String path(FileRoot());
    FileNormalizePath(path.c_str());
    for (ObjDirItr<RndTex> it(dir, true); it != nullptr; ++it) {
        FilePath fp(it->File());
        if (fp.empty()) {
            continue;
        }
        String relative(FileRelativePath(FileRoot(), fp.c_str()));
        FileNormalizePath(path.c_str());
        if (strstr(relative.c_str(), "..") == relative.c_str()
            && strstr(relative.c_str(), "../../system/run") != relative.c_str()) {
            MILO_NOTIFY("%s: %s is outside project path", PathName(it), relative);
        }
        if (relative.length() > 2 && relative.c_str()[1] == ':') {
            MILO_NOTIFY("%s: %s is outside project path", PathName(it), relative);
        }
    }
    if (dir->Loader()) {
        const char *dirFile = dir->Loader()->LoaderFile().c_str();
        bool isNG = strstr(dirFile, "/ng/");
        for (ObjDirItr<RndTex> it(dir, false); it != nullptr; ++it) {
            const char *texFile = it->File().c_str();
            if (!isNG && strstr(texFile, "/ng/")) {
                MILO_NOTIFY("og %s has ng texture %s", dirFile, texFile);
            } else if (isNG && strstr(texFile, "/og/")) {
                MILO_NOTIFY("ng %s has og texture %s", dirFile, texFile);
            }
        }
    }
}

void TestMaterialTextures(ObjectDir *dir) {
    for (ObjDirItr<RndMat> it(dir, false); it != nullptr; ++it) {
        RndTex *normal = it->NormalMap();
        if (normal) {
            FilePath file = normal->File();
            if (!normal->IsRenderTarget() && !strstr(file.c_str(), "_norm")) {
                MILO_NOTIFY(
                    "normal map %s used by %s must have _norm in the filename",
                    PathName(normal),
                    PathName(it)
                );
            }
        }
    }
}

DataNode GetNormalMapTextures(ObjectDir *dir) {
    DataArrayPtr ptr(new DataArray(0x100));
    int idx = 0;
    ptr->Node(idx++) = NULL_OBJ;
    for (ObjDirItr<RndTex> it(dir, true); it != nullptr; ++it) {
        bool b1 = false;
        FilePath fp(it->File());
        if (strstr(FileGetBase(fp.c_str()), "_norm")) {
            b1 = true;
        } else {
            if (fp.empty()) {
                if (it->IsRenderTarget())
                    b1 = true;
            }
        }
        if (b1) {
            ptr->Node(idx++) = &*it;
        }
    }
    ptr->Resize(idx);
    return ptr;
}

DataNode GetTexturesOfType(ObjectDir *dir, RndTex::Type texType) {
    int num = 0;
    for (ObjDirItr<RndTex> it(dir, true); it != 0; ++it) {
        if ((texType & it->GetType()) == texType) {
            num++;
        }
    }
    DataArrayPtr ptr(new DataArray(num + 1));
    num = 0;
    for (ObjDirItr<RndTex> it(dir, true); it != 0; ++it) {
        if ((texType & it->GetType()) == texType) {
            ptr->Node(num++) = &*it;
        }
    }
    ptr->Node(num) = NULL_OBJ;
    return ptr;
}

DataNode GetRenderTextures(ObjectDir *dir) {
    return GetTexturesOfType(dir, RndTex::kRendered);
}

DataNode GetRenderTexturesNoZ(ObjectDir *dir) {
    return GetTexturesOfType(dir, RndTex::kRenderedNoZ);
}

DataNode OnTestDrawGroups(DataArray *da) {
    DataArray *arr = nullptr;
    ObjectDir *dir = da->Obj<class ObjectDir>(2);
    if (da->Size() > 3)
        arr = da->Array(3);
    for (ObjDirItr<RndDrawable> it(dir, true); it; ++it) {
        std::list<RndGroup *> gList;
        ListDrawGroups(it, dir, gList);
        if (arr) {
            for (std::list<RndGroup *>::iterator gListIt = gList.begin();
                 gListIt != gList.end();
                 gListIt) {
                bool canerase = false;
                for (int i = 0; i < arr->Size(); i++) {
                    if (streq((*gListIt)->Name(), arr->Str(i))) {
                        canerase = true;
                        break;
                    }
                }
                if (canerase)
                    gListIt = gList.erase(gListIt);
                else
                    ++gListIt;
            }
        }
        if (gList.size() > 1) {
            class String str(
                MakeString("%s is in %d groups:", PathName(it), gList.size())
            );
            for (std::list<RndGroup *>::iterator gListIt = gList.begin();
                 gListIt != gList.end();
                 ++gListIt) {
                str << " " << PathName(*gListIt);
            }
            MILO_NOTIFY(str.c_str());
        }
    }
    return 0;
}

void ConvertBonesToTranses(ObjectDir *dir, bool b2) {
    std::list<RndMesh *> meshes;
    for (ObjDirItr<RndMesh> it(dir, false); it != nullptr; ++it) {
        RndTransformable *t = it;
        if (ShouldStrip(t)) {
            meshes.push_back(it);
        } else if (b2) {
            bool b1 = false;
            for (auto rit = it->Refs().begin(); !b1 && rit != it->Refs().end(); ++rit) {
                RndMesh *curRefOwner = dynamic_cast<RndMesh *>(rit->RefOwner());
                if (curRefOwner) {
                    for (int i = 0; i < curRefOwner->NumBones(); i++) {
                        if (curRefOwner->BoneTransAt(i) == t) {
                            meshes.push_back(it);
                            b1 = true;
                            break;
                        }
                    }
                }
            }
        }
    }
    while (!meshes.empty()) {
        ReplaceObject(
            meshes.front(), Hmx::Object::New<RndTransformable>(), true, true, true
        );
        meshes.pop_front();
    }
    for (ObjDirItr<RndTransformable> it(dir, true); it != nullptr; ++it) {
        if (strneq("spot_", it->Name(), 5)) {
            Normalize(it->LocalXfm().m, it->DirtyLocalXfm().m);
        }
    }
}

void BuildSphereStratified(unsigned int ui, std::vector<Vector3> &vectors) {
    Rand rand(0x29A);
    unsigned int root = sqrtf((float)ui) + 0.5f;
    vectors.clear();
    vectors.reserve(root * root);
    float f12 = -1;
    float f11 = 0;
    float f7 = (1.0f / (float)root) * 2.0f;
    float f8 = (1.0f / (float)root) * (PI * 2);
    for (int i = 0; i < root; i++) {
        for (int j = 0; j < root; j++) {
            float f6 = rand.Float() * f7 + f12;
            float _x = rand.Float() * f8 + f11;
            float f5 = sqrt(-(f6 * f6 - 1));
            Vector3 v490;
            float x = cosf(_x) * f5;
            float y = sinf(_x) * f5;
            Normalize(Vector3(x, y, f6), v490);
            vectors.push_back(v490);
            f11 += f8;
        }
        f12 += f7;
    }
}

void AttachMesh(RndMesh *main, RndMesh *attach) {
    MILO_ASSERT(main && attach, 0x525);
    int numMainFaces = main->Faces().size();
    int numAttachFaces = attach->Faces().size();
    main->Faces().resize(numMainFaces + numAttachFaces);
    int numMainVerts = main->Verts().size();
    for (int i = 0; i < numAttachFaces; i++) {
        auto &attachFace = attach->Faces(i);
        auto &mainFace = main->Faces(i + numMainFaces);
        mainFace.Set(
            attachFace.v1 + numMainVerts,
            attachFace.v2 + numMainVerts,
            attachFace.v3 + numMainVerts
        );
    }
    Transform tf80;
    FastInvert(main->WorldXfm(), tf80);
    Multiply(attach->WorldXfm(), tf80, tf80);
    int numAttachVerts = attach->Verts().size();
    main->Verts().resize(numMainVerts + numAttachVerts);
    for (int i = 0; i < numAttachVerts; i++) {
        RndMesh::Vert &mainVert = main->Verts(i + numMainVerts);
        RndMesh::Vert &attachVert = attach->Verts(i);
        Multiply(attachVert.pos, tf80, mainVert.pos);
        mainVert.color = attachVert.color;
        mainVert.boneWeights = attachVert.boneWeights;
        mainVert.norm = attachVert.norm;
        mainVert.tex = attachVert.tex;
    }
    main->Sync(0x3F);
}

void TessellateMesh(RndMesh *mesh) {
    std::set<RndAmbientOcclusion::Edge> edges;
    std::vector<RndMesh::Face> faces;
    std::vector<RndMesh::Vert> verts;
    faces.reserve(mesh->Faces().size() * 4);
    verts.reserve(mesh->Verts().size() * 3);
    unsigned int originalNumVerts = mesh->Verts().size();
    unsigned int newNumVerts = originalNumVerts;
    for (int i = 0; i < mesh->Faces().size(); i++) {
        auto &face = mesh->Faces(i);
        RndAmbientOcclusion::Edge edge1(face.v1, face.v2);
        RndAmbientOcclusion::Edge edge2(face.v2, face.v3);
        RndAmbientOcclusion::Edge edge3(face.v3, face.v1);
        auto &vert1 = mesh->Verts(face.v1);
        auto &vert2 = mesh->Verts(face.v2);
        auto &vert3 = mesh->Verts(face.v3);
        RndMesh::Vert v190;
        RndMesh::Vert v130;
        RndMesh::Vert vd0;
        RndAmbientOcclusion::BlendVert(vert1, vert2, v190);
        RndAmbientOcclusion::BlendVert(vert2, vert3, v130);
        RndAmbientOcclusion::BlendVert(vert3, vert1, vd0);
        auto edge1it = edges.find(edge1);
        if (edge1it == edges.end()) {
            edge1.split = newNumVerts++;
            edges.insert(edge1);
            verts.push_back(v190);
        } else {
            edge1 = *edge1it;
        }
        auto edge2it = edges.find(edge2);
        if (edge2it == edges.end()) {
            edge2.split = newNumVerts++;
            edges.insert(edge2);
            verts.push_back(v130);
        } else {
            edge2 = *edge2it;
        }
        auto edge3it = edges.find(edge3);
        if (edge3it == edges.end()) {
            edge3.split = newNumVerts++;
            edges.insert(edge3);
            verts.push_back(vd0);
        } else {
            edge3 = *edge3it;
        }

        RndMesh::Face face1(face.v1, edge1.split, edge3.split);
        RndMesh::Face face2(edge3.split, edge1.split, edge2.split);
        RndMesh::Face face3(edge1.split, face.v2, edge2.split);
        RndMesh::Face face4(edge2.split, face.v3, edge3.split);
        faces.push_back(face1);
        faces.push_back(face2);
        faces.push_back(face3);
        faces.push_back(face4);
    }
    mesh->Faces().assign(faces.begin(), faces.end());
    mesh->Verts().resize(mesh->Verts().size() + verts.size());
    for (int i = originalNumVerts; i < newNumVerts; i++) {
        mesh->Verts(i) = verts[i - originalNumVerts];
    }
    mesh->Sync(0x3F);
}

void RandomPointOnMesh(RndMesh *m, Vector3 &v1, Vector3 &v2) {
    RndMesh::Face &face = m->Faces()[RandomInt(0, m->Faces().size())];
    int numverts = m->Verts().size();
    if (face.v1 >= numverts || face.v2 >= numverts || face.v3 >= numverts) {
        MILO_NOTIFY_ONCE(
            "%s: %s random face contains unknown vert indices!", PathName(m), m->Name()
        );
        v1.Zero();
        v2.Zero();
    } else {
        Vector3 v58, v64, v70;
        Vector3 v7c, v88, v94;
        if (m->NumBones() > 0) {
            v58 = m->SkinVertex(m->Verts()[face.v1], &v7c);
            v64 = m->SkinVertex(m->Verts()[face.v2], &v88);
            v70 = m->SkinVertex(m->Verts()[face.v3], &v94);
        } else {
            v58 = m->Verts()[face.v1].pos;
            v64 = m->Verts()[face.v2].pos;
            v70 = m->Verts()[face.v3].pos;
            v7c = m->Verts()[face.v1].norm;
            v88 = m->Verts()[face.v2].norm;
            v94 = m->Verts()[face.v3].norm;
        }
        float f8 = RandomFloat();
        float f9 = RandomFloat();
        if (f8 + f9 > 1.0f) {
            f8 = 1.0f - f8;
            f9 = 1.0f - f9;
        }
        float f1 = (1.0f - f8) - f9;
        v58 *= f8;
        v64 *= f9;
        v70 *= f1;
        Add(v58, v64, v1);
        Add(v1, v70, v1);
        v7c *= f8;
        v88 *= f9;
        v94 *= f1;
        Add(v7c, v88, v2);
        Add(v2, v94, v2);
        Normalize(v2, v2);
    }
}

void MakeTangentsLate(RndMesh *mesh) {
    if (mesh && mesh->GetGeomOwner() == mesh && !mesh->Verts().empty()
        && GetGfxMode() != kOldGfx) {
        std::vector<Vector4> vectors(mesh->Faces().size());

        for (int i = 0; i < mesh->Faces().size(); i++) {
            Hmx::Matrix3 mtx;
            ComputeFaceTangentBasis(mesh, i, mtx);
            // determinant?
            Vector3 tmp;
            Cross(mtx.z, mtx.x, tmp);
            float w = Dot(tmp, mtx.y) < 0 ? -1.0f : 1.0f;
            Vector3 inv;
            Normalize(mtx.x, inv);
            // the asm said they did this don't get mad at me
            vectors[i] = reinterpret_cast<Vector4 &>(inv);
            vectors[i].w = w;
        }

        for (int i = 0; i < mesh->Verts().size(); i++) {
            bool first = true;
            auto &vert = mesh->Verts(i);
            Vector4 &tangent = vert.tangent;
            Vector3 &tangent3 = reinterpret_cast<Vector3 &>(tangent);
            for (int j = 0; j < mesh->Faces().size(); j++) {
                auto &curFace = mesh->Faces(j);
                int faceIdx;
                for (faceIdx = 0; faceIdx < 3; faceIdx++) {
                    if (curFace[faceIdx] == i) {
                        break;
                    }
                }
                if (faceIdx != 3) {
                    if (first) {
                        first = false;
                        vert.tangent = vectors[j];
                    } else {
                        if (vectors[j].w * tangent.w < 0) {
                            MILO_LOG(
                                "NOTIFY: %s has previously welded vertex tangents with opposite handedness; re-export from Max for more accurate normal mapping.\n",
                                PathName(mesh)
                            );
                        } else {
                            Add(tangent3,
                                reinterpret_cast<Vector3 &>(vectors[j]),
                                tangent3);
                        }
                    }
                }
            }
            Normalize(tangent3, tangent3);
            Vector3 &norm = vert.norm;
            Vector3 tangentCopy = tangent3;
            Vector3 vd0;
            Scale(norm, Dot(tangentCopy, norm), vd0);
            Subtract(tangentCopy, vd0, vd0);
            Normalize(vd0, tangent3);
        }

        MILO_LOG("NOTIFY: %s MakingTangentsLate, resave this file!", PathName(mesh));
    }
}

void RndScaleObject(Hmx::Object *obj, float scale, float frameScale) {
    RndDrawable *draw = dynamic_cast<RndDrawable *>(obj);
    if (draw) {
        Sphere s = draw->GetSphere();
        s.center *= scale;
        s.radius *= scale;
        draw->SetSphere(s);
    }
    RndTransformable *trans = dynamic_cast<RndTransformable *>(obj);
    if (trans) {
        Vector3 pos;
        Scale(trans->LocalXfm().v, scale, pos);
        trans->SetLocalPos(pos);
    }
    RndCam *cam = dynamic_cast<RndCam *>(obj);
    if (cam) {
        cam->SetFrustum(cam->NearPlane() * scale, cam->FarPlane() * scale, cam->YFov(), 1);
        return;
    }
    RndCamAnim *camAnim = dynamic_cast<RndCamAnim *>(obj);
    if (camAnim) {
        if (camAnim->KeysOwner() == camAnim) {
            ScaleFrame(camAnim->FovKeys(), frameScale);
        }
        return;
    }
    RndEnviron *env = dynamic_cast<RndEnviron *>(obj);
    if (env) {
        env->SetFogRange(env->GetFogStart() * scale, env->GetFogEnd() * scale);
        return;
    }
    RndGenerator *gen = dynamic_cast<RndGenerator *>(obj);
    if (gen) {
        float lo, hi;
        gen->GetRateVar(lo, hi);
        gen->SetRateVar(lo * frameScale, hi * frameScale);
        return;
    }
    RndLight *lit = dynamic_cast<RndLight *>(obj);
    if (lit) {
        lit->SetRange(lit->Range() * scale);
        return;
    }
    RndLightAnim *litAnim = dynamic_cast<RndLightAnim *>(obj);
    if (litAnim) {
        if (litAnim->KeysOwner() == litAnim) {
            ScaleFrame(litAnim->ColorKeys(), frameScale);
        }
        return;
    }
    RndLine *line = dynamic_cast<RndLine *>(obj);
    if (line) {
        line->SetWidth(line->GetWidth() * scale);
        for (int i = 0; i < line->NumPoints(); i++) {
            Vector3 pos;
            Scale(line->PointAt(i).point, scale, pos);
            line->SetPointPos(i, pos);
        }
        return;
    }
    RndMatAnim *matAnim = dynamic_cast<RndMatAnim *>(obj);
    if (matAnim) {
        if (matAnim->KeysOwner() == matAnim) {
            ScaleFrame(matAnim->ColorKeys(), frameScale);
            ScaleFrame(matAnim->AlphaKeys(), frameScale);
            ScaleFrame(matAnim->TransKeys(), frameScale);
            ScaleFrame(matAnim->ScaleKeys(), frameScale);
            ScaleFrame(matAnim->RotKeys(), frameScale);
        }
        return;
    }
    RndMesh *mesh = dynamic_cast<RndMesh *>(obj);
    if (mesh) {
        if (mesh->GetGeomOwner() == mesh) {
            FOREACH (vert, mesh->Verts()) {
                vert->pos *= scale;
            }
            mesh->Sync(0x1F);
            Transform xfm;
            xfm.m.Set(scale, 0, 0, 0, scale, 0, 0, 0, scale);
            xfm.v.Zero();
            MultiplyEq(mesh->GetBSPTree(), xfm);
        }
        mesh->ScaleBones(scale);
        return;
    }
    RndMeshAnim *meshAnim = dynamic_cast<RndMeshAnim *>(obj);
    if (meshAnim) {
        if (meshAnim->KeysOwner() == meshAnim) {
            FOREACH (it, meshAnim->VertPointsKeys()) {
                FOREACH (vit, it->value) {
                    *vit *= scale;
                }
            }
            ScaleFrame(meshAnim->VertNormalsKeys(), frameScale);
            ScaleFrame(meshAnim->VertPointsKeys(), frameScale);
            ScaleFrame(meshAnim->VertTexsKeys(), frameScale);
            ScaleFrame(meshAnim->VertColorsKeys(), frameScale);
        }
        return;
    }
    RndMorph *morph = dynamic_cast<RndMorph *>(obj);
    if (morph) {
        for (int i = 0; i < morph->NumPoses(); i++) {
            ScaleFrame(morph->PoseAt(i).weights, frameScale);
        }
        return;
    }
    RndMultiMesh *multiMesh = dynamic_cast<RndMultiMesh *>(obj);
    if (multiMesh) {
        FOREACH (it, multiMesh->Instances()) {
            it->mXfm.v *= scale;
        }
        return;
    }
    RndParticleSys *partSys = dynamic_cast<RndParticleSys *>(obj);
    if (partSys) {
        // this whole block needs fixing
        Vector3 forceDir = partSys->ForceDir();
        forceDir *= (scale / frameScale) / frameScale;
        partSys->SetForceDir(forceDir);
        partSys->SetBubbleSize(
            partSys->BubbleSize().x * scale, partSys->BubbleSize().y * scale
        );
        partSys->SetBubblePeriod(
            partSys->BubblePeriod().x * frameScale, partSys->BubblePeriod().y * frameScale
        );
        partSys->SetLife(partSys->Life().x * frameScale, partSys->Life().y * frameScale);
        partSys->SetEmitRate(
            partSys->EmitRate().x / frameScale, partSys->EmitRate().y / frameScale
        );
        Vector3 box1, box2;
        Scale(partSys->BoxExtent1(), scale, box1);
        Scale(partSys->BoxExtent2(), scale, box2);
        partSys->SetStartSize(
            partSys->StartSize().x * scale, partSys->StartSize().y * scale
        );
        partSys->SetSpeed(
            (partSys->Speed().x * scale) / frameScale,
            (partSys->Speed().y * scale) / frameScale
        );
        partSys->SetDeltaSize(
            partSys->DeltaSize().x * scale, partSys->DeltaSize().y * scale
        );
        partSys->SetBoxExtent(box1, box2);
        return;

        // from ghidra
        // uVar5 = mForceDir.w
        // mBubbleSize.x = mBubbleSize.x * scale;
        // fVar1 = (float)(1.0 / frameScale);
        // mBubbleSize.y = mBubbleSize.y * scale;
        // mLife.x = mLife.x * frameScale;
        // mBubblePeriod.x = mBubblePeriod.x * frameScale;
        // mBubblePeriod.y = mBubblePeriod.y * frameScale;
        // mEmitRate.y = mEmitRate.y * fVar1;
        // mLife.y = mLife.y * frameScale;
        // mEmitRate.x = mEmitRate.x * fVar1;
        // fVar2 = (float)((double)(fVar1 * fVar1) * scale);
        // mForceDir.y = mForceDir.y * fVar2;
        // mForceDir.z = mForceDir.z * fVar2;
        // mForceDir.w = uVar5;
        // mForceDir.x = mForceDir.x * fVar2;
        // mStartSize.x = mStartSize.x * scale;
        // mStartSize.y = mStartSize.y * scale;
        // mSpeed.x = (mSpeed.x * fVar1) * scale;
        // mSpeed.y = (mSpeed.y * fVar1) * scale;
        // mDeltaSize.x = mDeltaSize.x * scale;
        // mBoxExtent1.x = mBoxExtent1.x * scale;
        // mDeltaSize.y = mDeltaSize.y * scale;
        // mBoxExtent1.y = mBoxExtent1.y * scale;
        // mBoxExtent1.z = mBoxExtent1.z * scale;
        // mBoxExtent1.w = unknown - padding
        // mBoxExtent2.x = mBoxExtent2.x * scale;
        // mBoxExtent2.y = mBoxExtent2.y * scale;
        // mBoxExtent2.z = mBoxExtent2.z * scale;
        // mBoxExtent2.z = unknown - padding
    }
    RndParticleSysAnim *partSysAnim = dynamic_cast<RndParticleSysAnim *>(obj);
    if (partSysAnim) {
        if (partSysAnim->KeysOwner() == partSysAnim) {
            ScaleFrame(partSysAnim->StartColorKeys(), frameScale);
            ScaleFrame(partSysAnim->EndColorKeys(), frameScale);
            ScaleFrame(partSysAnim->EmitRateKeys(), frameScale);
            ScaleFrame(partSysAnim->SpeedKeys(), frameScale);
            ScaleFrame(partSysAnim->LifeKeys(), frameScale);
            ScaleFrame(partSysAnim->StartSizeKeys(), frameScale);
        }
        return;
    }
    RndTransAnim *transAnim = dynamic_cast<RndTransAnim *>(obj);
    if (transAnim) {
        if (transAnim->KeysOwner() == transAnim) {
            FOREACH (it, transAnim->TransKeys()) {
                it->value *= scale;
            }
            ScaleFrame(transAnim->TransKeys(), frameScale);
            ScaleFrame(transAnim->RotKeys(), frameScale);
            ScaleFrame(transAnim->ScaleKeys(), frameScale);
        }
        return;
    }
}

void MakeNormals(RndMesh *);

void BuildVisit(BSPNode *);

void BuildFromBSP(RndMesh *mesh) {
    BuildVisit(mesh->GetBSPTree());
    int numVerts = 0;
    int numFaces = 0;
    for (auto it = gChildPolys.begin(); it != gChildPolys.end();) {
        auto &pts = it->mPoly.points;
        if (pts.size() < 3) {
            it = gChildPolys.erase(it);
        } else {
            ++it;
            numVerts += pts.size();
            numFaces += pts.size() - 2;
        }
    }
    mesh->Verts().resize(numVerts);
    mesh->Faces().resize(numFaces);
    int i13 = 0;
    FOREACH (it, gChildPolys) {
        FOREACH (pt, it->mPoly.points) {
            Multiply(Vector3(pt->x, pt->y, 0), it->mTransform, mesh->Verts(i13).pos);
            i13++;
        }
        int i8 = i13 - it->mPoly.points.size();
        int i11 = i8 + 2;
        for (int i = 0; i < i13 - i11; i++) {
            mesh->Faces(i).Set(i8, i11 - 1, i11);
            i11++;
        }
    }
    gParentPolys.clear();
    gChildPolys.clear();
    MakeNormals(mesh);
}
