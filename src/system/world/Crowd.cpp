#include "char/Character.h"
#include "math/Color.h"
#include "math/Geo.h"
#include "math/Mtx.h"
#include "math/Rand.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/System.h"
#include "os/Timer.h"
#include "rndobj/BaseMaterial.h"
#include "rndobj/Cam.h"
#include "rndobj/Draw.h"
#include "rndobj/Env.h"
#include "rndobj/Mat.h"
#include "rndobj/MultiMesh.h"
#include "rndobj/Poll.h"
#include "rndobj/Rnd.h"
#include "rndobj/Tex.h"
#include "rndobj/Utl.h"
#include "utl/BinStream.h"
#include "utl/Loader.h"
#include "utl/Std.h"
#include "world/ColorPalette.h"
#include "world/Crowd.h"
#include "world/Crowd3DCharHandle.h"

static RndTex *gImpostorTex[kNumLods] = { nullptr };
static RndCam *gImpostorCamera = nullptr;
static RndMat *gImpostorMat = nullptr;
static int gNumCrowd = 0;
static WorldCrowd *gParent = nullptr;

namespace {
    void GetMeshShaderFlags(RndMat *mat, std::list<unsigned int> &flags) {
        FOREACH (it, mat->Refs()) {
            RndMesh *cur = dynamic_cast<RndMesh *>(it->RefOwner());
            if (cur) {
                // bit 1 = skinned
                // bit 2 = ao calc
                unsigned int mask = cur->IsSkinned() | (cur->HasAOCalc() ? 2 : 0);
                flags.push_back(mask);
            }
        }
        flags.sort();
        flags.unique();
    }
}

void SetMatColorFlags(
    ObjPtrList<RndMat> &mats,
    BaseMaterial::ColorModFlags flags,
    std::vector<Hmx::Color> *modulate
) {
    FOREACH (it, mats) {
        (*it)->SetColorModFlags(flags);
        if (modulate) {
            MILO_ASSERT(RndMat::kColorModNum == modulate->size(), 0x33B);
            for (int i = 0; i < modulate->size(); i++) {
                (*it)->SetColorMod(modulate->at(i), i);
            }
        }
    }
}

#pragma region CharDef

void WorldCrowd::CharDef::Save(BinStream &bs) const {
    bs << mChar;
    bs << mHeight;
    bs << mDensity;
    bs << mRadius;
    bs << mUseRandomColor;
}

void WorldCrowd::CharDef::Load(BinStreamRev &d) {
    d >> mChar;
    d >> mHeight;
    d >> mDensity;
    if (d.rev > 1) {
        d >> mRadius;
    }
    if (d.rev > 8) {
        d >> mUseRandomColor;
    }
}

#pragma endregion
#pragma region CharData

void WorldCrowd::CharData::Save(BinStream &bs) const { mDef.Save(bs); }

BinStream &operator<<(BinStream &bs, const WorldCrowd::CharData &cd) {
    cd.Save(bs);
    return bs;
}

BinStreamRev &operator>>(BinStreamRev &d, WorldCrowd::CharData &cd) {
    cd.mDef.Load(d);
    return d;
}

#pragma endregion
#pragma region WorldCrowd

WorldCrowd::WorldCrowd()
    : mPlacementMesh(this), mCharacters(this), mNum(0), mRotate(), mForce3DCrowd(0),
      mShow3DOnly(0), mCharFullness(1), mFlatFullness(1), mLod(0), mEnviron(this),
      mEnviron3D(this), mFocus(this), mCharForceLod(kLODPerFrame), unkd0(1),
      mModifyStamp(0) {
    if (gNumCrowd++ == 0) {
        int w, h, bpp;
        if (GetGfxMode() == kNewGfx) {
            w = 256;
            h = 512;
            bpp = 32;
        } else {
            w = 128;
            h = 256;
            bpp = 16;
        }
        for (int i = 0; i < kNumLods; i++, w >>= 1, h >>= 1) {
            RndTex *tex = Hmx::Object::New<RndTex>();
            tex->SetBitmap(w, h, bpp, RndTex::kRendered, true, nullptr);
            gImpostorTex[i] = tex;
        }
        RELEASE(gImpostorMat);
        RndMat *mat = Hmx::Object::New<RndMat>();
        gImpostorMat = mat;
        mat->SetPreLit(false);
        mat->SetUseEnv(true);
        mat->SetBlend(RndMat::kBlendSrc);
        mat->SetZMode(kZModeNormal);
        mat->SetAlphaCut(true);
        mat->SetAlphaThreshold(0x80);
        mat->SetTexWrap(kTexWrapClamp);
        mat->SetPerPixelLit(false);
        mat->SetPointLights(true);
        CreateAndSetMetaMat(mat);
        gImpostorCamera = Hmx::Object::New<RndCam>();
        SetMatAndCameraLod();
    }
}

WorldCrowd::~WorldCrowd() {
    Delete3DCrowdHandles();
    FOREACH (it, mCharacters) {
        if (it->mMMesh) {
            delete it->mMMesh->Mesh();
            RELEASE(it->mMMesh);
        }
    }
    if (--gNumCrowd == 0) {
        for (int i = 0; i < kNumLods; i++) {
            RELEASE(gImpostorTex[i]);
        }
        RELEASE(gImpostorCamera);
        RELEASE(gImpostorMat);
    }
}

DataNode WorldCrowd::OnRebuild(DataArray *) { return 0; }

BEGIN_HANDLERS(WorldCrowd)
    HANDLE(rebuild, OnRebuild)
    HANDLE_ACTION(assign_random_colors, AssignRandomColors(true))
    HANDLE(iterate_frac, OnIterateFrac)
    HANDLE_ACTION(set_fullness, SetFullness(_msg->Float(2), _msg->Float(3)))
    HANDLE_SUPERCLASS(RndDrawable)
    HANDLE_SUPERCLASS(RndPollable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_CUSTOM_PROPSYNC(WorldCrowd::CharData)
    SYNC_PROP(character, o.mDef.mChar)
    SYNC_PROP(height, o.mDef.mHeight)
    SYNC_PROP(density, o.mDef.mDensity)
    SYNC_PROP(radius, o.mDef.mRadius)
END_CUSTOM_PROPSYNC

BEGIN_PROPSYNCS(WorldCrowd)
    gParent = this;
    SYNC_PROP(num, mNum)
    SYNC_PROP(placement_mesh, mPlacementMesh)
    SYNC_PROP(characters, mCharacters)
    SYNC_PROP(show_3d_only, mShow3DOnly)
    SYNC_PROP(environ, mEnviron)
    SYNC_PROP(environ_3d, mEnviron3D)
    SYNC_PROP_SET(lod, mLod, SetLod(_val.Int()))
    SYNC_PROP_SET(force_3D_crowd, mForce3DCrowd, Force3DCrowd(_val.Int()))
    SYNC_PROP(focus, mFocus)
    SYNC_PROP(char_force_lod, (int &)mCharForceLod)
    SYNC_SUPERCLASS(RndDrawable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

void WorldCrowd::SetLod(int lod) { mLod = Clamp(0, 2, lod); }

BEGIN_SAVES(WorldCrowd)
    SAVE_REVS(0x10, 0)
    SAVE_SUPERCLASS(RndDrawable)
    bool force = mForce3DCrowd;
    Force3DCrowd(false);
    bs << mPlacementMesh << mNum << mCharacters << mEnviron;
    bs << mEnviron3D;
    FOREACH (it, mCharacters) {
        std::list<Transform> transforms;
        RndMultiMesh *mesh = it->mMMesh;
        if (mesh) {
            FOREACH (t, mesh->Instances()) {
                transforms.push_back(t->mXfm);
            }
        }
        bs << transforms;
    }
    bs << mModifyStamp;
    bs << force;
    bs << mShow3DOnly;
    bs << mFocus;
    bs << mCharForceLod;
    bs << unkd0;
    Force3DCrowd(force);
    SAVE_SUPERCLASS(RndPollable)
END_SAVES

BEGIN_COPYS(WorldCrowd)
    COPY_SUPERCLASS(RndDrawable)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(WorldCrowd)
    BEGIN_COPYING_MEMBERS
        Delete3DCrowdHandles();
        COPY_MEMBER(mPlacementMesh)
        COPY_MEMBER(mNum)
        COPY_MEMBER(unk70)
        COPY_MEMBER(mCharFullness)
        COPY_MEMBER(mFlatFullness)
        COPY_MEMBER(mLod)
        COPY_MEMBER(mEnviron)
        COPY_MEMBER(mEnviron3D)
        COPY_MEMBER(mForce3DCrowd)
        COPY_MEMBER(mShow3DOnly)
        COPY_MEMBER(mFocus)
        COPY_MEMBER(mCharForceLod)
        COPY_MEMBER(unkd0)

        mCharacters.clear();
        mCharacters.resize(c->mCharacters.size());
        ObjList<CharData>::const_iterator j = c->mCharacters.begin();
        ObjList<CharData>::iterator i = mCharacters.begin();
        for (; i != mCharacters.end(); ++i, ++j) {
            i->mDef = j->mDef;
            i->mBackup = j->mBackup;
            i->m3DChars = j->m3DChars;
            i->m3DCharsCreated = j->m3DCharsCreated;
        }
        CreateMeshes();
        j = c->mCharacters.begin();
        for (ObjList<CharData>::iterator i = mCharacters.begin(); i != mCharacters.end();
             ++i, ++j) {
            if (i->mMMesh) {
                MILO_ASSERT(j->mMMesh, 0x1DD);
                i->mMMesh->Instances() = j->mMMesh->Instances();
            }
        }
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(0x10, 0)

BEGIN_LOADS(WorldCrowd)
    LOAD_REVS(bs)
    ASSERT_REVS(0x10, 0)
    LOAD_SUPERCLASS(RndDrawable)
    Reset3DCrowd();
    d >> mPlacementMesh;
    if (d.rev < 3) {
        int x;
        d >> x;
    }
    d >> mNum;
    if (d.rev < 8) {
        bool b;
        d >> b;
    }
    d >> mCharacters;
    if (d.rev > 6) {
        d >> mEnviron;
    }
    if (d.rev > 9) {
        d >> mEnviron3D;
    } else {
        mEnviron3D = mEnviron;
    }
    if (d.rev > 1) {
        CreateMeshes();
        FOREACH (it, mCharacters) {
            if (d.rev < 0xE) {
                std::list<Transform> xfmList;
                std::list<RndMultiMesh::Instance> instancesList;
                std::list<OldMMInst> oldmmiList;
                if (it->mMMesh) {
                    if (d.rev < 9) {
                        d >> xfmList;
                        it->mMMesh->Instances().clear();
                        FOREACH (transIt, xfmList) {
                            it->mMMesh->Instances().push_back(*transIt);
                        }
                    } else if (d.rev < 0xB) {
                        d >> oldmmiList;
                        FOREACH (mmiIt, oldmmiList) {
                            OldMMInst &old = *mmiIt;
                            it->mMMesh->Instances().push_back(old.mOldXfm);
                        }
                    } else {
                        InstanceList &instances = it->mMMesh->Instances();
                        unsigned int count;
                        d >> count;
                        instances.resize(count);
                        FOREACH (instIt, instances) {
                            instIt->LoadRev(d.stream, 3);
                        }
                    }
                } else if (d.rev > 3) {
                    if (d.rev < 9)
                        d >> xfmList;
                    else if (d.rev < 0xB)
                        d >> oldmmiList;
                    else
                        d >> instancesList;
                }
            } else {
                std::list<Transform> xfms;
                d >> xfms;
                if (it->mMMesh) {
                    it->mMMesh->Instances().clear();
                    FOREACH (xfmIt, xfms) {
                        it->mMMesh->Instances().push_back(*xfmIt);
                    }
                }
            }
            AssignRandomColors(false);
        }
    } else {
        OnRebuild(nullptr);
    }
    if (d.rev > 4) {
        d >> mModifyStamp;
    }
    if (d.rev > 0xC) {
        bool force = false;
        d >> force;
        Force3DCrowd(force);
    }
    if (d.rev > 5) {
        d >> mShow3DOnly;
    }
    if (d.rev > 0xB) {
        d >> mFocus;
    }
    if (d.rev > 0xE) {
        d >> (int &)mCharForceLod;
    }
    if (d.rev > 0xF) {
        d >> unkd0;
    }
    if (d.rev > 0) {
        LOAD_SUPERCLASS(RndPollable);
    }
END_LOADS

void WorldCrowd::UpdateSphere() {
    Sphere s;
    MakeWorldSphere(s, true);
    SetSphere(s);
}

float WorldCrowd::GetDistanceToPlane(const Plane &p, Vector3 &vout) {
    if (mCharacters.empty())
        return 0;
    else {
        float dist = 0;
        bool b1 = true;
        FOREACH (it, mCharacters) {
            RndMultiMesh *multimesh = it->mMMesh;
            if (multimesh) {
                Vector3 v4c;
                float f5 = multimesh->GetDistanceToPlane(p, v4c);
                if (b1 || (std::fabs(f5) < std::fabs(dist))) {
                    b1 = false;
                    vout = v4c;
                    dist = f5;
                }
            }
        }
        return dist;
    }
}

bool WorldCrowd::MakeWorldSphere(Sphere &s, bool b) {
    if (b) {
        s.Zero();
        FOREACH (it, mCharacters) {
            RndMultiMesh *multimesh = it->mMMesh;
            if (multimesh) {
                Sphere local;
                multimesh->MakeWorldSphere(local, true);
                s.GrowToContain(local);
            }
        }
        return true;
    } else if (GetSphere().radius) {
        s = GetSphere();
        return true;
    } else
        return false;
}

void WorldCrowd::Mats(std::list<RndMat *> &mats, bool b2) {
    if (b2) {
        MatShaderOptions opts;
        opts.pack |= 0x20;
        int masks[2] = { 0xD, 0x13 };
        for (int i = 0; i < 2; i++) {
            opts.SetLast5(masks[i]);
            for (int j = 0; j < 2; j++) {
                for (int k = 0; k < 2; k++) {
                    RndMat *mat = Hmx::Object::New<RndMat>();
                    mat->Copy(gImpostorMat, kCopyDeep);
                    mat->SetUseEnv(j);
                    opts.mTempMat = true;
                    opts.SetHasAOCalc(k);
                    mat->SetShaderOpts(opts);
                    mats.push_back(mat);
                }
            }
        }
        std::vector<Hmx::Color> colors;
        for (int i = 0; i < 3; i++) {
            colors.push_back(Hmx::Color(1, 1, 1));
        }
        for (int i = 0; i <= 3; i++) {
            if (i != 2) {
                FOREACH (it, mCharacters) {
                    if (it->mDef.mUseRandomColor) {
                        SetMatColorFlags(
                            it->mDef.mMats, (BaseMaterial::ColorModFlags)i, &colors
                        );
                        FOREACH (mat, it->mDef.mMats) {
                            std::list<unsigned int> flags;
                            GetMeshShaderFlags(*mat, flags);
                            FOREACH (flag, flags) {
                                unsigned int curFlag = *flag;
                                opts.SetLast5(0x12);
                                opts.SetHasBones(curFlag & 1);
                                opts.SetHasAOCalc(curFlag >> 1 & 1);
                                RndMat *curMat = Hmx::Object::New<RndMat>();
                                curMat->Copy(*mat, kCopyDeep);
                                opts.mTempMat = true;
                                curMat->SetShaderOpts(opts);
                                mats.push_back(curMat);
                            }
                        }
                    }
                }
            }
        }
    }
}

void WorldCrowd::DrawShowing() {
    START_AUTO_TIMER("crowd_draw");
    if (!mPlacementMesh) {
        return;
    }
    Draw3DChars();
    if (TheRnd.DrawMode() == 5) {
        return;
    }
    MILO_ASSERT(!gImpostorMat->NextPass(), 0x3A0);
    std::vector<Hmx::Rect> rects;
    rects.reserve(12);
    FOREACH (it, mCharacters) {
    }
}

void WorldCrowd::ListDrawChildren(std::list<RndDrawable *> &draws) {
    FOREACH (it, mCharacters) {
        Character *curChar = it->mDef.mChar;
        if (curChar)
            draws.push_back(curChar);
    }
}

void WorldCrowd::CollideList(const Segment &seg, std::list<Collision> &colls) {
    if (TheLoadMgr.EditMode() && CollideSphere(seg)) {
        FOREACH (it, mCharacters) {
            RndMultiMesh *curMM = it->mMMesh;
            if (curMM) {
                curMM->CollideList(seg, colls);
            }
            Character *curChar = it->mDef.mChar;
            for (int i = 0; i != it->m3DChars.size(); i++) {
                Apply3DCharXfm(it, i, nullptr);
                float fl;
                Plane pl;
                if (curChar->CollideShowing(seg, fl, pl)) {
                    if (!it->m3DChars[i].m3DCrowdHandle) {
                        it->m3DChars[i].m3DCrowdHandle =
                            Hmx::Object::New<WorldCrowd3DCharHandle>();
                        it->m3DChars[i].m3DCrowdHandle->Set3DChar(
                            this, it, i, it->m3DChars[i].mXfm
                        );
                    }
                    colls.push_back(Collision(it->m3DChars[i].m3DCrowdHandle, fl, pl));
                }
            }
        }
    }
}

void WorldCrowd::Poll() {
    if (Showing()) {
        FOREACH (it, mCharacters) {
            Character *curChar = it->mDef.mChar;
            if (curChar && curChar->GetPollState() != 3) {
                curChar->Poll();
            }
        }
    }
}

void WorldCrowd::Enter() {
    RndPollable::Enter();
    FOREACH (it, mCharacters) {
        it->mDef.mMats.clear();
        Character *curChar = it->mDef.mChar;
        if (curChar) {
            if (curChar->GetPollState() != 2)
                curChar->Enter();
            ColorPalette *randPal = curChar->Find<ColorPalette>("random1.pal", false);
            if (randPal && randPal->NumColors() != 0) {
                for (ObjDirItr<RndMat> objIt(curChar, true); objIt; ++objIt) {
                    it->mDef.mMats.push_back(objIt);
                }
            }
        }
    }
}

void WorldCrowd::Exit() {
    RndPollable::Exit();
    FOREACH (it, mCharacters) {
        Character *curChar = it->mDef.mChar;
        if (curChar)
            curChar->Exit();
    }
}

void WorldCrowd::ListPollChildren(std::list<RndPollable *> &polls) const {
    FOREACH (it, mCharacters) {
        Character *curChar = it->mDef.mChar;
        if (curChar)
            polls.push_back(curChar);
    }
}

void WorldCrowd::Delete3DCrowdHandles() {
    if (TheLoadMgr.EditMode()) {
        FOREACH (it, mCharacters) {
            for (int i = 0; i != it->m3DChars.size(); i++) {
                RELEASE(it->m3DChars[i].m3DCrowdHandle);
            }
        }
    }
}

bool WorldCrowd::Crowd3DExists() {
    FOREACH (it, mCharacters) {
        if (it->mDef.mChar && it->mMMesh && !it->m3DChars.empty()) {
            return true;
        }
    }
    return false;
}

void WorldCrowd::SetMatAndCameraLod() {
    gImpostorCamera->SetTargetTex(gImpostorTex[mLod]);
    gImpostorMat->SetDiffuseTex(gImpostorTex[mLod]);
}

void WorldCrowd::CreateMeshes() {
    mCharFullness = 1.0f;
    mFlatFullness = 1.0f;
    mLod = 0;
    FOREACH (it, mCharacters) {
        if (it->mMMesh) {
            delete it->mMMesh->Mesh();
            RELEASE(it->mMMesh);
        }
        it->mBackup.clear();
        if (it->mDef.mChar) {
            RndMesh *built = BuildBillboard(it->mDef.mChar, it->mDef.mHeight);
            it->mMMesh = Hmx::Object::New<RndMultiMesh>();
            it->mMMesh->SetMesh(built);
        }
    }
}

struct Sort3DChars {
    bool operator()(
        const WorldCrowd::CharData::Char3D &char1,
        const WorldCrowd::CharData::Char3D &char2
    ) const {
        return char1.mIndex < char2.mIndex;
    }
};

void WorldCrowd::Sort3DCharList() {
    FOREACH (it, mCharacters) {
        std::sort(it->m3DChars.begin(), it->m3DChars.end(), Sort3DChars());
        it->m3DCharsCreated = it->m3DChars;
    }
}

void WorldCrowd::Force3DCrowd(bool force) {
    mForce3DCrowd = force;
    if (mForce3DCrowd) {
        Set3DCharAll();
    } else {
        SetFullness(1, 1);
        std::vector<std::pair<int, int> > vec;
        Set3DCharList(vec, this);
    }
}

void WorldCrowd::Reset3DCrowd() {
    SetFullness(1, mCharFullness);
    FOREACH (it, mCharacters) {
        if (it->mMMesh) {
            auto &insts = it->mMMesh->Instances();
            int i6 = 0;
            auto inst = insts.begin();
            for (int i = 0; i != it->m3DCharsCreated.size(); i++) {
                int cap = it->m3DCharsCreated[i].mIndex;
                for (; i6 != cap; i6++) {
                    ++inst;
                }
                inst = insts.insert(inst, it->m3DCharsCreated[i].mXfm);
            }
        }
        it->m3DCharsCreated.clear();
        it->m3DChars.clear();
    }
}

void WorldCrowd::Draw3DChars() {
    if (Crowd3DExists()) {
        RndEnviron *env = mEnviron3D ? mEnviron3D : mEnviron;
        bool global = true;
        if (env) {
            global = env->UsesApproxGlobal();
            env->SetUseApproxGlobal(false);
        }
        RndEnvironTracker tracker(env, nullptr);
        FOREACH (it, mCharacters) {
            Character *curChar = it->mDef.mChar;
            RndMultiMesh *curMMesh = it->mMMesh;
            if (curChar && curMMesh) {
                auto &chars = it->m3DChars;
                for (int i = 0; i != chars.size(); i++) {
                    Apply3DCharXfm(it, i, RndCam::Current());
                    if (it->mDef.mUseRandomColor) {
                        SetMatColorFlags(
                            it->mDef.mMats,
                            RndMat::kColorModModulate,
                            &chars[i].mRandColors
                        );
                    }
                    bool selfShadow = curChar->SelfShadow();
                    bool floorShadow = curChar->FloorShadow();
                    bool spotCutout = curChar->SpotCutout();
                    if (TheRnd.InGame()) {
                        curChar->SetSelfShadow(false);
                        curChar->SetFloorShadow(false);
                        curChar->SetSpotCutout(false);
                    }
                    if (mCharForceLod != -1) {
                        curChar->SetLodType(mCharForceLod);
                    }
                    curChar->Draw();
                    if (mCharForceLod != -1) {
                        curChar->SetLodType(kLODPerFrame);
                    }
                    curChar->SetSelfShadow(selfShadow);
                    curChar->SetFloorShadow(floorShadow);
                    curChar->SetSpotCutout(spotCutout);
                }
            }
        }
        if (env) {
            env->SetUseApproxGlobal(global);
        }
    }
}

void WorldCrowd::Set3DCharAll() {
    START_AUTO_TIMER("crowd_set3d");
    float oldFlat = mFlatFullness;
    Reset3DCrowd();
    FOREACH (it, mCharacters) {
        RndMultiMesh *multiMesh = it->mMMesh;
        if (multiMesh) {
            auto instIt = multiMesh->Instances().begin();
            int idx = 0;
            for (; instIt != multiMesh->Instances().end(); ++instIt, ++idx) {
                CharData::Char3D char3D(instIt->mXfm, idx);
                it->m3DChars.push_back(char3D);
            }
            multiMesh->Instances().clear();
            multiMesh->InvalidateProxies();
        }
    }
    Sort3DCharList();
    SetFullness(oldFlat, mCharFullness);
    AssignRandomColors(false);
}

void WorldCrowd::Set3DCharXfm(
    const std::list<CharData>::iterator &charItr, int char3DIdx, const Transform &xfm
) {
    MILO_ASSERT_RANGE(char3DIdx, 0, charItr->m3DChars.size(), 0x289);
    charItr->m3DChars[char3DIdx].mXfm = xfm;
    bool foundCreated = false;
    for (int i = 0; i < charItr->m3DCharsCreated.size(); i++) {
        if (charItr->m3DChars[char3DIdx].mIndex == charItr->m3DCharsCreated[i].mIndex) {
            charItr->m3DCharsCreated[i].mXfm = xfm;
            foundCreated = true;
            break;
        }
    }
    MILO_ASSERT(foundCreated, 0x297);
}

void WorldCrowd::Apply3DCharXfm(
    const std::list<CharData>::iterator &charItr, int char3DIdx, RndCam *cam
) {
    MILO_ASSERT_RANGE(char3DIdx, 0, charItr->m3DChars.size(), 0x29D);
    Character *itrChar = charItr->mDef.mChar;
    if (itrChar && mPlacementMesh) {
        Transform xfm;
        xfm.v = charItr->m3DChars[char3DIdx].mXfm.v;
        xfm.v.z -= charItr->mDef.mHeight / 2;
        bool b8 = mRotate != kCrowdRotateNone && cam;
        if (!b8 && !mFocus) {
            xfm.m = mPlacementMesh->WorldXfm().m;
            itrChar->SetWorldXfm(xfm);
            return;
        }
        // else...
        xfm.m.z = mPlacementMesh->WorldXfm().m.z;
        if (mRotate == kCrowdRotateFace) {
            Cross(xfm.m.z, cam->WorldXfm().m.y, xfm.m.x);
        } else if (mRotate == kCrowdRotateAway) {
            Cross(cam->WorldXfm().m.y, xfm.m.z, xfm.m.x);
        } else {
            const Vector3 &v = mFocus->WorldXfm().v;
            Vector3 diff(v.x - xfm.v.x, v.y - xfm.v.y, 0);
            Cross(diff, xfm.m.z, xfm.m.x);
        }
        Normalize(xfm.m.x, xfm.m.x);
        Cross(xfm.m.z, xfm.m.x, xfm.m.y);
        itrChar->SetWorldXfm(xfm);
    }
}

void WorldCrowd::SetFullness(float f1, float f2) {
    START_AUTO_TIMER("crowd_set");
    mCharFullness = f2;
    mFlatFullness = f1;
    Delete3DCrowdHandles();
    FOREACH (it, mCharacters) {
        if (it->mMMesh) {
            int bigsize = (float)(it->mMMesh->Instances().size() + it->mBackup.size())
                * mFlatFullness;
            int instsize = it->mMMesh->Instances().size();
            if (instsize < bigsize) {
                auto backupIt = it->mBackup.begin();
                for (; instsize < bigsize; instsize++) {
                    ++backupIt;
                }
                auto backupBegin = it->mBackup.begin();
                auto instsBegin = it->mMMesh->Instances().begin();
                if (backupBegin != backupIt && instsBegin != backupIt) {
                    // something
                }
            } else if (bigsize < instsize) {
                auto backupIt = it->mBackup.begin();
                for (; bigsize < instsize; instsize--) {
                    ++backupIt;
                }
                auto backupBegin = it->mBackup.begin();
                auto instsBegin = it->mMMesh->Instances().begin();
                if (instsBegin != backupIt && backupBegin != backupIt) {
                    // something
                }
                it->mMMesh->InvalidateProxies();
            }
        }
    }
    AssignRandomColors(false);
}

void WorldCrowd::AssignRandomColors(bool b1) {
    if (b1) {
        unkd0++;
    }
    FOREACH (it, mCharacters) {
        if (it->mDef.mChar && it->mMMesh && !it->m3DChars.empty()) {
            std::vector<ColorPalette *> colorPalettes;
            it->mDef.mUseRandomColor = false;
            for (int i = 0; i < 3; i++) {
                const char *str = MakeString("random%d.pal", i + 1);
                ColorPalette *p = it->mDef.mChar->Find<ColorPalette>(str, false);
                if (p) {
                    colorPalettes.push_back(p);
                }
            }
            if (colorPalettes.size() == 3) {
                for (int i = 0; i != it->m3DChars.size(); i++) {
                    CharData::Char3D &curChar3D = it->m3DChars[i];
                    curChar3D.mRandColors.clear();
                    Rand rand(curChar3D.mIndex + unkd0);
                    it->mDef.mUseRandomColor = true;
                    for (int j = 0; j < 3; j++) {
                        ColorPalette *curPalette = colorPalettes[j];
                        Hmx::Color c =
                            curPalette->GetColor(rand.Int(0, curPalette->NumColors()));
                        curChar3D.mRandColors.push_back(c);
                    }
                }
            }
        }
    }
}

RndMesh *WorldCrowd::BuildBillboard(Character *c, float f) {
    RndMesh *mesh = Hmx::Object::New<RndMesh>();
    mesh->SetMutable(0x1F);
    RndMesh::VertVector &verts = mesh->Verts();
    std::vector<RndMesh::Face> &faces = mesh->Faces();
    float f1 = f / 2;
    float f2 = f1 / 2;
    verts.resize(4);
    verts[0].pos.Set(-f2, 0, f1);
    verts[1].pos.Set(-f2, 0, -f1);
    verts[2].pos.Set(f2, 0, f1);
    verts[3].pos.Set(f2, 0, -f1);
    verts[0].tex.Set(0, 0);
    verts[1].tex.Set(0, 1);
    verts[2].tex.Set(1, 0);
    verts[3].tex.Set(1, 1);
    faces.resize(2);
    faces[0].Set(0, 1, 2);
    faces[1].Set(1, 3, 2);
    mesh->Sync(0x3F);
    mesh->SetMat(gImpostorMat);
    mesh->SetTransConstraint(
        RndTransformable::kConstraintFastBillboardXYZ, gImpostorCamera, false
    );
    return mesh;
}

DataNode WorldCrowd::OnIterateFrac(DataArray *a) {
    START_AUTO_TIMER("crowd_iter");
    if (mCharacters.empty()) {
        return 0;
    } else {
        Character *chars[64];
        int num = 0;
        FOREACH (it, mCharacters) {
            Character *cur = it->mDef.mChar;
            if (cur) {
                chars[num] = cur;
                num++;
            }
        }
        int max = num - 1;
        for (; max > 0; max--) {
            std::swap(chars[max], chars[RandomInt() % (max + 1)]);
        }
        float f16 = 0;
        for (int i = 2; i < a->Size(); i++) {
            float f19 = a->Array(i)->Float(0);
            if (f19 > 0) {
                f16 += f19;
            }
        }
        int j = 0;
        float f18 = -0.5f;
        f16 = (float)num / f16;
        for (int i = 2; i < a->Size(); i++) {
            DataArray *arr = a->Array(i);
            float f19 = arr->Float(0);
            f18 += f19 * f16;
            for (; j < f18; j++) {
                arr->ExecuteScript(1, chars[j], nullptr, 1);
            }
        }
        return 0;
    }
}
