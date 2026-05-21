#include "world/SpotlightDrawer.h"
#include "math/Color.h"
#include "math/Mtx.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/System.h"
#include "rndobj/BoxMap.h"
#include "rndobj/Draw.h"
#include "rndobj/Env.h"
#include "rndobj/MultiMesh.h"
#include "rndobj/Rnd.h"
#include "rndobj/Stats_NG.h"
#include "utl/BinStream.h"
#include "utl/Loader.h"
#include "world/Spotlight.h"

RndEnviron *SpotlightDrawer::sEnviron;
SpotlightDrawer *SpotlightDrawer::sDefault;
std::vector<SpotlightDrawer::SpotlightEntry> SpotlightDrawer::sLights;
std::vector<SpotlightDrawer::SpotMeshEntry> SpotlightDrawer::sCans;
std::vector<Spotlight *> SpotlightDrawer::sShadowSpots;
int SpotlightDrawer::sNeedBoxMap = -1;
bool sHaveAdditionals;
bool sHaveLenses;
bool sHaveFlares;

#pragma region SpotDrawParams

SpotDrawParams::SpotDrawParams(SpotlightDrawer *owner)
    : mIntensity(1), mColor(1, 1, 1), mBaseIntensity(0.1f), mSmokeIntensity(0.5f),
      mHalfDistance(250), mLightingInfluence(1), mTexture(owner), mProxy(owner),
      mOwner(owner) {
    MILO_ASSERT(owner, 0x351);
}

SpotDrawParams &SpotDrawParams::operator=(const SpotDrawParams &rhs) {
    mIntensity = rhs.mIntensity;
    mBaseIntensity = rhs.mBaseIntensity;
    mSmokeIntensity = rhs.mSmokeIntensity;
    mHalfDistance = rhs.mHalfDistance;
    mLightingInfluence = rhs.mLightingInfluence;
    mColor = rhs.mColor;
    mTexture = rhs.mTexture;
    mProxy = rhs.mProxy;
    return *this;
}

void SpotDrawParams::Save(BinStream &bs) {
    bs << mIntensity;
    bs << mBaseIntensity;
    bs << mSmokeIntensity;
    bs << mHalfDistance;
    bs << mColor;
    bs << mTexture;
    bs << mProxy;
    bs << mLightingInfluence;
}

void SpotDrawParams::Load(BinStreamRev &d) {
    d >> mIntensity;
    if (d.rev > 3) {
        d >> mBaseIntensity >> mSmokeIntensity >> mHalfDistance;
    } else {
        float x, y, z, w;
        d >> x >> y >> z >> w;
        if (z < 0.5f) {
            mSmokeIntensity = 0.5f;
            mBaseIntensity = 0.1f;
        } else {
            mBaseIntensity = 0.15f;
            mSmokeIntensity = 1.0f;
        }
    }
    d >> mColor;
    if (d.rev < 4) {
        float x;
        Vector2 vx, vy;
        d >> x >> vx >> vy;
    }
    d >> mTexture;
    d >> mProxy;
    if (d.rev < 3) {
        bool b;
        d >> b;
    }
    if (d.rev > 4) {
        d >> mLightingInfluence;
    }
}

#pragma endregion
#pragma region SpotlightDrawer

SpotlightDrawer::SpotlightDrawer() : mParams(this) { SetOrder(-100000); }

SpotlightDrawer::~SpotlightDrawer() {
    if (sCurrent == this) {
        DeSelect();
        ClearAndShrink(sLights);
        ClearAndShrink(sShadowSpots);
        ClearAndShrink(sCans);
    }
}

BEGIN_HANDLERS(SpotlightDrawer)
    HANDLE_SUPERCLASS(RndDrawable)
    HANDLE_SUPERCLASS(Hmx::Object)
    HANDLE_ACTION(select, Select())
    HANDLE_ACTION(deselect, DeSelect())
END_HANDLERS

BEGIN_PROPSYNCS(SpotlightDrawer)
    SYNC_PROP(total, mParams.mIntensity)
    SYNC_PROP(base_intensity, mParams.mBaseIntensity)
    SYNC_PROP(smoke_intensity, mParams.mSmokeIntensity)
    SYNC_PROP(color, mParams.mColor)
    SYNC_PROP(proxy, mParams.mProxy)
    SYNC_PROP(light_influence, mParams.mLightingInfluence)
    SYNC_SUPERCLASS(RndDrawable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(SpotlightDrawer)
    SAVE_REVS(6, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(RndDrawable)
    mParams.Save(bs);
END_SAVES

BEGIN_COPYS(SpotlightDrawer)
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(RndDrawable)
    CREATE_COPY_AS(SpotlightDrawer, c)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mParams)
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(6, 0)

BEGIN_LOADS(SpotlightDrawer)
    LOAD_REVS(bs)
    ASSERT_REVS(6, 0)
    if (d.rev > 0) {
        if (d.rev > 5) {
            LOAD_SUPERCLASS(Hmx::Object)
        }
        LOAD_SUPERCLASS(RndDrawable)
    } else {
        LOAD_SUPERCLASS(Hmx::Object)
    }
    SetOrder(-100000);
    mParams.Load(d);
END_LOADS

void SpotlightDrawer::DrawShowing() {
    if (sCurrent && sCurrent != sDefault && sCurrent != this) {
        MILO_NOTIFY_ONCE(
            "Drawing 2 spotlightdrawers in one frame, %s and %s",
            PathName(sCurrent),
            PathName(this)
        );
    } else
        Select();
}

void SpotlightDrawer::ListDrawChildren(std::list<RndDrawable *> &children) {
    children.push_back(mParams.mProxy);
}

void SpotlightDrawer::EndWorld() {
    UpdateBoxMap();
    if (sNeedDraw) {
        DrawWorld();
        ClearPostDraw();
    }
    if (TheRnd.DisablePP()) {
        ClearLights();
    }
    MILO_ASSERT(!sNeedDraw, 0x165);
}

void SpotlightDrawer::SetAmbientColor(const Hmx::Color &c) {
    sEnviron->SetAmbientColor(c);
    sEnviron->Select(nullptr);
}

void SpotlightDrawer::SortLights() {
    if (sLights.size() > 2) {
        std::sort(sLights.begin(), sLights.end(), ByColor());
    }
    if (sCans.size() > 2) {
        std::sort(sCans.begin(), sCans.end(), ByEnvMesh());
    }
}

template <class T>
void DrawAccessories(
    SpotlightDrawer::SpotlightEntry *const &spotBegin,
    SpotlightDrawer::SpotlightEntry *const &spotEnd
);

class LensExtract {};

void SpotlightDrawer::DrawWorld() {
    TheNgStats->mSpotlights = Max<int>(sLights.size(), TheNgStats->mSpotlights);
    if ((!sLights.empty() || !sCans.empty()) && Showing()) {
        SortLights();
        DrawMeshVec(sCans);
        sCans.resize(0);
        if (!sLights.empty()) {
            Vector3 *curPosSet = RndEnviron::CurrentPos();
            RndEnviron *env = RndEnviron::Current();
            MILO_ASSERT(sEnviron->GetUseApprox() == false, 0x1DC);
            sEnviron->Select(nullptr);
            if (GetGfxMode() == kOldGfx) {
                DrawShadow();
            }
            SpotlightEntry *spotIter = sLights.begin();
            SpotlightEntry *const spotEnd = sLights.end();
            while (spotIter != spotEnd) {
                Hmx::Color envColor = spotIter->mLight->IntensifiedColor();
                SpotlightEntry *const colorBegin = spotIter;
                SpotlightEntry *colorEnd = spotIter;
                for (; colorEnd != spotEnd
                     && colorEnd->mPackedColor == colorBegin->mPackedColor;
                     ++colorEnd)
                    ;
                SetAmbientColor(envColor);
                if (sHaveAdditionals) {
                    DrawAdditional(spotIter, colorEnd);
                }
                if (sHaveLenses) {
                    DrawAccessories<LensExtract>(colorBegin, colorEnd);
                }
                if (!DrawNGSpotlights() && !sNoBeams && TheRnd.DrawMode() != 5) {
                    DrawBeams(spotIter, colorEnd);
                }
                if (sHaveFlares) {
                    DrawFlares(spotIter, colorEnd);
                }
                spotIter = colorEnd;
            }
            if (env) {
                env->Select(curPosSet);
            }
        }
    }
}

void SpotlightDrawer::DrawShadow() {
    auto it = sShadowSpots.begin();
    auto itEnd = sShadowSpots.end();
    for (; it != itEnd; ++it) {
        Spotlight *shadowSpot = *it;
        MILO_ASSERT(shadowSpot->GetTarget() && shadowSpot->GetCastShadow(), 0x288);
        RndDrawable *draw = dynamic_cast<RndDrawable *>(shadowSpot->GetTarget());
        if (draw) {
            draw->DrawShadow(shadowSpot->WorldXfm(), 1.5f);
        }
    }
}

void SpotlightDrawer::DrawMeshVec(std::vector<SpotMeshEntry> &entries) {
    if (entries.size() != 0) {
        SpotMeshEntry *meshIter = entries.begin();
        RndMesh *currMesh = meshIter->mMesh;
        RndMultiMesh *multiMesh = currMesh->CreateMultiMesh();
        multiMesh->Instances().push_back(meshIter->mXfm);
        RndEnviron *currEnv = meshIter->mEnv;
        currEnv->Select(nullptr);
        SpotMeshEntry *const meshEnd = entries.end();
        for (; meshIter != meshEnd; ++meshIter) {
            bool changeEnv = meshIter->mEnv != currEnv;
            bool changeMesh = meshIter->mMesh != currMesh;
            if (changeEnv || changeMesh) {
                multiMesh->DrawShowing();
                if (changeEnv) {
                    currEnv = meshIter->mEnv;
                    currEnv->Select(nullptr);
                }
                if (changeMesh) {
                    currMesh = meshIter->mMesh;
                    multiMesh = meshIter->mMesh->CreateMultiMesh();
                }
            }
            multiMesh->Instances().push_back(meshIter->mXfm);
        }
        multiMesh->DrawShowing();
    }
}

void SpotlightDrawer::DrawAdditional(
    SpotlightDrawer::SpotlightEntry *spotIter,
    SpotlightDrawer::SpotlightEntry *const &spotEnd
) {
    MILO_ASSERT(spotIter != spotEnd, 0x298);
    for (; spotIter != spotEnd; ++spotIter) {
        Spotlight *sl = spotIter->mLight;
        FOREACH (it, sl->GetAdditionalObjects()) {
            RndDrawable *add = *it;
            MILO_ASSERT(add != sl, 0x2a3);
            if (add != sl)
                add->Draw();
        }
    }
}

void SpotlightDrawer::DrawLenses(
    SpotlightDrawer::SpotlightEntry *spotIter,
    SpotlightDrawer::SpotlightEntry *const &spotEnd
) {
    MILO_ASSERT(spotIter != spotEnd, 0x2B1);
    for (; spotIter != spotEnd; ++spotIter) {
        Spotlight *sl = spotIter->mLight;
        // if (sl->LensMesh()) {
        if (Spotlight::GetDiskMesh()) {
            MILO_ASSERT(sl->LensMesh(), 0x2B9);
            Spotlight::GetDiskMesh()->SetMat(sl->LensMesh());
            Spotlight::GetDiskMesh()->Draw();
        }
        // }
    }
}

void SpotlightDrawer::DrawBeams(
    SpotlightDrawer::SpotlightEntry *spotIter,
    SpotlightDrawer::SpotlightEntry *const &spotEnd
) {
    MILO_ASSERT(spotIter != spotEnd, 0x2c7);
    for (; spotIter != spotEnd; ++spotIter) {
        Spotlight *sl = spotIter->mLight;
        Spotlight::BeamDef &def = sl->GetBeam();
        if (def.mBeam) {
            MILO_ASSERT(def.mBeam->Showing(), 0x2e4);
            def.mBeam->DrawShowing();
        }
    }
}

void SpotlightDrawer::DrawFlares(
    SpotlightDrawer::SpotlightEntry *spotIter,
    SpotlightDrawer::SpotlightEntry *const &spotEnd
) {
    MILO_ASSERT(spotIter != spotEnd, 0x2f4);
    for (; spotIter != spotEnd; ++spotIter) {
        Spotlight *sl = spotIter->mLight;
        if (sl->GetFlare() && sl->GetFlare()->GetMat()) {
            sl->GetFlare()->Draw();
        }
    }
}

void SpotlightDrawer::ClearPostDraw() {
    ClearLights();
    sNeedDraw = false;
}

void SpotlightDrawer::Init() {
    sEnviron = Hmx::Object::New<RndEnviron>();
    sEnviron->SetUseApproxes(false);
    REGISTER_OBJ_FACTORY(SpotlightDrawer)
    sDefault = Hmx::Object::New<SpotlightDrawer>();
    sDefault->mParams.mLightingInfluence = 0.0f;
    sDefault->Select();
}

void SpotlightDrawer::Select() {
    if (sCurrent != this) {
        if (sCurrent) {
            TheRnd.UnregisterPostProcessor(sCurrent);
        }
        sCurrent = this;
        TheRnd.RegisterPostProcessor(this);
    }
    sNeedBoxMap = -1;
}

void SpotlightDrawer::DeSelect() {
    if (sCurrent == this) {
        if (sDefault != this) {
            sDefault->Select();
        } else {
            TheRnd.UnregisterPostProcessor(sCurrent);
            sCurrent = nullptr;
        }
    }
}

void SpotlightDrawer::ClearLights() {
    sLights.resize(0);
    sShadowSpots.resize(0);
    sCans.resize(0);
    sHaveAdditionals = false;
    sHaveLenses = false;
    sHaveFlares = false;
}

void SpotlightDrawer::UpdateBoxMap() {
    if (sNeedBoxMap != TheRnd.GetFrameID()) {
        RndEnviron::GetGlobalLighting().Clear();
        if (mParams.mLightingInfluence > 0) {
            ApplyLightingApprox(
                RndEnviron::GetGlobalLighting(), mParams.mLightingInfluence
            );
        }
        sNeedBoxMap = TheRnd.GetFrameID();
    }
}

void SpotlightDrawer::ApplyLightingApprox(BoxMapLighting &boxMap, float f2) const {
    MILO_ASSERT(boxMap.NumQueuedLights() == 0, 0x20B);
    auto it = sLights.begin();
    auto itEnd = sLights.end();
    for (; it != itEnd; ++it) {
        Spotlight *sl = it->mLight;
        const Transform &xfm = sl->WorldXfm();
        Hmx::Color color(sl->Color());
        Multiply(color, f2, color);
        Multiply(color, sl->Intensity(), color);
        BoxMapLighting::LightParams_Spot *params;
        if (!boxMap.ParamsAt(params)) {
            break;
        }
        params->unk40 = xfm.v;
        params->unk0 = xfm.m.y;
        params->mColor = color;
        auto &beam = sl->GetBeam();
        params->unk54 = beam.mTopRadius;
        params->unk58 = beam.mBottomRadius * 2;
        params->unk50 = beam.mLength * 2;
        boxMap.CacheData(*params);
    }
}

bool SpotlightDrawer::DrawNGSpotlights() {
    return GetGfxMode() == kNewGfx && TheLoadMgr.GetPlatform() != kPlatformPC;
}

void SpotlightDrawer::RemoveFromLists(Spotlight *spot) {
    for (auto it = sLights.begin(); it != sLights.end();) {
        if (it->mLight == spot) {
            it = sLights.erase(it);
        } else
            ++it;
    }
    for (auto it = sCans.begin(); it != sCans.end();) {
        if (it->mLight == spot) {
            it = sCans.erase(it);
        } else
            ++it;
    }
    for (auto it = sShadowSpots.begin(); it != sShadowSpots.end();) {
        if (*it == spot) {
            it = sShadowSpots.erase(it);
        } else
            ++it;
    }
}

#pragma endregion
