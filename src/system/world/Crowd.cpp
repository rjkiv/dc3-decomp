#include "char/Character.h"
#include "obj/Object.h"
#include "os/System.h"
#include "rndobj/Cam.h"
#include "rndobj/Draw.h"
#include "rndobj/Mat.h"
#include "rndobj/MultiMesh.h"
#include "rndobj/Poll.h"
#include "rndobj/Tex.h"
#include "rndobj/Utl.h"
#include "utl/BinStream.h"
#include "utl/Loader.h"
#include "world/ColorPalette.h"
#include "world/Crowd.h"
#include "world/Crowd3DCharHandle.h"

RndCam *gImpostorCamera;
RndMat *gImpostorMat;
int gNumCrowd;
WorldCrowd *gParent;
RndTex *gImpostorTex[kNumLods];

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
    : mPlacementMesh(this), mCharacters(this), mNum(0), unk6c(0), mForce3DCrowd(0),
      mShow3DOnly(0), mCharFullness(1), mFlatFullness(1), mLod(0), mEnviron(this),
      mEnviron3D(this), mFocus(this), mCharForceLod(kLODPerFrame), unkd0(0),
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
        for (int i = 0; i < kNumLods; i++) {
            gImpostorTex[i] = Hmx::Object::New<RndTex>();
            gImpostorTex[i]->SetBitmap(w, h, bpp, RndTex::kRendered, true, nullptr);
        }
        RELEASE(gImpostorMat);
        RndMat *mat = Hmx::Object::New<RndMat>();
        gImpostorMat = mat;
        mat->SetUseEnv(true);
        mat->SetPreLit(false);
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
    for (ObjList<CharData>::iterator it = mCharacters.begin(); it != mCharacters.end();
         ++it) {
        if (it->mMMesh) {
            delete it->mMMesh->Mesh();
            RELEASE(it->mMMesh);
        }
    }
    gNumCrowd--;
    if (gNumCrowd == 0) {
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
                            it->mMMesh->Instances().push_back(
                                RndMultiMesh::Instance(*transIt)
                            );
                        }
                    } else if (d.rev < 0xB) {
                        d >> oldmmiList;
                        FOREACH (mmiIt, oldmmiList) {
                            OldMMInst &old = *mmiIt;
                            it->mMMesh->Instances().push_back(
                                RndMultiMesh::Instance(old.mOldXfm)
                            );
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
                        it->mMMesh->Instances().push_back(RndMultiMesh::Instance(*xfmIt));
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
    } else if (mSphere.GetRadius()) {
        s = mSphere;
        return true;
    } else
        return false;
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
                RELEASE(it->m3DChars[i].unk50);
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
        return char1.unk40 < char2.unk40;
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
    if (mForce3DCrowd)
        Set3DCharAll();
    else {
        SetFullness(1, 1);
        std::vector<std::pair<int, int> > vec;
        Set3DCharList(vec, this);
    }
}
