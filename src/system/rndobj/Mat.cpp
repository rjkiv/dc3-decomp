#include "rndobj/Mat.h"
#include "Rnd.h"
#include "Utl.h"
#include "math/Color.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/DirLoader.h"
#include "obj/Object.h"
#include "obj/Utl.h"
#include "os/Debug.h"
#include "os/File.h"
#include "os/System.h"
#include "rndobj/BaseMaterial.h"
#include "rndobj/Fur.h"
#include "rndobj/MetaMaterial.h"
#include "rndobj/Tex.h"
#include "utl/BinStream.h"
#include "utl/FilePath.h"
#include "utl/Loader.h"
#include "utl/Symbol.h"

const char *kAnonMetaMatPrefix = "{anon}";
const char *kMiloMetaMatPrefix = "{milo}";

MatShaderOptions::MatShaderOptions() : pack(0x12), mTempMat(0) {}

namespace {
    void MergeMetaMaterials(ObjectDir *o1, ObjectDir *o2) {
        if (o2 && o1) {
            for (ObjDirItr<MetaMaterial> it(o2, true); it != nullptr; ++it) {
                MetaMaterial *mat = o1->Find<MetaMaterial>(it->Name(), false);
                if (!mat) {
                    mat = Hmx::Object::New<MetaMaterial>();
                    mat->SetName(it->Name(), o1);
                }
                CopyObject(it, mat, Hmx::Object::kCopyDeep, true);
            }
        }
    }

    void AddOverridePropName(String &str, Symbol &sym) {
        if (!str.empty()) {
            str += ", ";
        }
        str += sym.Str();
    }
}

RndMat::RndMat()
    : mMetaMaterial(this), unk20c(0), mToggleDisplayAllProps(0), unk225(0), unk226(0),
      mDirty(3) {
    ResetColors(mColorMod, 3);
}

RndMat::~RndMat() {
    if (unk225 && mMetaMaterial) {
        if (mMetaMaterial->RefCount() == 1) {
            RELEASE(mMetaMaterial);
        }
    }
}

BEGIN_HANDLERS(RndMat)
    HANDLE(get_metamats_dir, OnGetMetaMaterialsDir)
    HANDLE(get_metamats, OnGetMetaMaterials)
    HANDLE_EXPR(prop_is_hidden, OnGetPropertyDisplay(kPropDisplayHidden, _msg->Sym(2)))
    HANDLE_EXPR(
        prop_is_read_only, OnGetPropertyDisplay(kPropDisplayReadOnly, _msg->Sym(2))
    )
    HANDLE_ACTION(
        toggle_display_all_props, mToggleDisplayAllProps = !mToggleDisplayAllProps
    )
    HANDLE_ACTION(create_metamat, CreateMetaMaterial(true))
    HANDLE_SUPERCLASS(BaseMaterial)
END_HANDLERS

#define SYNC_MAT_PROP(s, member, dirty_flag)                                             \
    {                                                                                    \
        _NEW_STATIC_SYMBOL(s)                                                            \
        if (sym == _s) {                                                                 \
            Symbol action(#s "_edit_action");                                            \
            if (!(_op & (kPropSize | kPropGet)) && !IsEditable(action)) {                \
                return true;                                                             \
            }                                                                            \
            if (!PropSync(member, _val, _prop, _i + 1, _op))                             \
                return false;                                                            \
            if (!(_op & (kPropSize | kPropGet)))                                         \
                mDirty |= dirty_flag;                                                    \
            return true;                                                                 \
        }                                                                                \
    }

BEGIN_PROPSYNCS(RndMat)
    // clang-format off
    SYNC_PROP_SET(metamaterial, mMetaMaterial.Ptr(),
        mMetaMaterial = _val.Obj<MetaMaterial>();
        UpdatePropertiesFromMetaMat();
        unk225 = false;
    )
    // clang-format on
    SYNC_MAT_PROP(intensify, mIntensify, 2)
    SYNC_MAT_PROP(blend, (int &)mBlend, 2)
    SYNC_MAT_PROP(color, mColor, 1)
    SYNC_MAT_PROP(alpha, mColor.alpha, 1)
    SYNC_MAT_PROP(use_environ, mUseEnviron, 2)
    SYNC_MAT_PROP(z_mode, (int &)mZMode, 2)
    SYNC_MAT_PROP(stencil_mode, (int &)mStencilMode, 2)
    SYNC_MAT_PROP(tex_gen, (int &)mTexGen, 2)
    SYNC_MAT_PROP(tex_wrap, (int &)mTexWrap, 2)
    SYNC_MAT_PROP(tex_xfm, mTexXfm, 2)
    SYNC_MAT_PROP(diffuse_tex, mDiffuseTex, 2)
    SYNC_MAT_PROP(diffuse_tex2, mDiffuseTex2, 2)
    SYNC_MAT_PROP(prelit, mPrelit, 2)
    SYNC_MAT_PROP(alpha_cut, mAlphaCut, 2)
    SYNC_PROP(alpha_threshold, mAlphaThreshold)
    SYNC_MAT_PROP(alpha_write, mAlphaWrite, 2)
    SYNC_PROP(force_alpha_write, mForceAlphaWrite)
    SYNC_MAT_PROP(next_pass, mNextPass, 2)
    SYNC_MAT_PROP(cull, (int &)mCull, 2)
    SYNC_MAT_PROP(per_pixel_lit, mPerPixelLit, 2)
    SYNC_MAT_PROP(emissive_multiplier, mEmissiveMultiplier, 2)
    SYNC_MAT_PROP(specular_rgb, mSpecularRGB, 2)
    SYNC_MAT_PROP(specular_power, mSpecularRGB.alpha, 2)
    SYNC_MAT_PROP(specular2_rgb, mSpecular2RGB, 2)
    SYNC_MAT_PROP(specular2_power, mSpecular2RGB.alpha, 2)
    SYNC_MAT_PROP(normal_map, mNormalMap, 2)
    SYNC_MAT_PROP(emissive_map, mEmissiveMap, 2)
    SYNC_PROP_SET(
        specular_map,
        mSpecularMap.Ptr(),
        if (IsEditable("specular_map_edit_action")) SetSpecularMap(_val.Obj<RndTex>())
    )
    SYNC_MAT_PROP(environ_map, mEnvironMap, 2)
    SYNC_MAT_PROP(environ_map_falloff, mEnvironMapFalloff, 2)
    SYNC_MAT_PROP(environ_map_specmask, mEnvironMapSpecMask, 2)
    SYNC_MAT_PROP(de_normal, mDeNormal, 2)
    SYNC_MAT_PROP(anisotropy, mAnisotropy, 2)
    SYNC_MAT_PROP(norm_detail_tiling, mNormDetailTiling, 2)
    SYNC_MAT_PROP(norm_detail_strength, mNormDetailStrength, 2)
    SYNC_MAT_PROP(norm_detail_map, mNormDetailMap, 2)
    SYNC_MAT_PROP(rim_rgb, mRimRGB, 2)
    SYNC_MAT_PROP(rim_power, mRimRGB.alpha, 2)
    SYNC_MAT_PROP(rim_map, mRimMap, 2)
    SYNC_MAT_PROP(rim_light_under, mRimLightUnder, 2)
    SYNC_MAT_PROP(refract_enabled, mRefractEnabled, 2)
    SYNC_MAT_PROP(refract_strength, mRefractStrength, 2)
    SYNC_MAT_PROP(refract_normal_map, mRefractNormalMap, 2)
    SYNC_MAT_PROP(screen_aligned, mScreenAligned, 2)
    SYNC_MAT_PROP(shader_variation, (int &)mShaderVariation, 2)
    SYNC_MAT_PROP(point_lights, mPointLights, 2)
    SYNC_MAT_PROP(fog, mFog, 2)
    SYNC_MAT_PROP(fade_out, mFadeout, 2)
    SYNC_MAT_PROP(color_adjust, mColorAdjust, 2)
    SYNC_MAT_PROP(fur, mFur, 2)
    // clang-format off
    SYNC_PROP_SET(recv_proj_lights, mPerfSettings.mRecvProjLights,
        if (IsEditable("recv_proj_lights_edit_action"))
            mPerfSettings.mRecvProjLights = _val.Int() > 0;
    )
    SYNC_PROP_SET(recv_point_cube_tex, mPerfSettings.mRecvPointCubeTex,
        if (IsEditable("recv_point_cube_tex_edit_action"))
            mPerfSettings.mRecvPointCubeTex = _val.Int() > 0;
    )
    SYNC_PROP_SET(ps3_force_trilinear, mPerfSettings.mPS3ForceTrilinear,
        if (IsEditable("ps3_force_trilinear_edit_action"))
            mPerfSettings.mPS3ForceTrilinear = _val.Int() > 0;
    )
    // clang-format on
    SYNC_MAT_PROP(bloom_multiplier, mBloomMultiplier, 2)
    SYNC_MAT_PROP(never_fit_to_spline, mNeverFitToSpline, 2)
    SYNC_MAT_PROP(allow_distortion_effects, mAllowDistortionEffects, 2)
    SYNC_MAT_PROP(shockwave_mult, mShockwaveMult, 2)
    SYNC_MAT_PROP(world_projection_tiling, mWorldProjectionTiling, 2)
    SYNC_MAT_PROP(world_projection_start_blend, mWorldProjectionStartBlend, 2)
    SYNC_MAT_PROP(world_projection_end_blend, mWorldProjectionEndBlend, 2)
    SYNC_SUPERCLASS(BaseMaterial)
END_PROPSYNCS

BEGIN_SAVES(RndMat)
    SAVE_REVS(0x46, 0)
    SAVE_SUPERCLASS(BaseMaterial)
    bs << mMetaMaterial;
END_SAVES

BEGIN_COPYS(RndMat)
    COPY_SUPERCLASS(BaseMaterial)
    CREATE_COPY(RndMat)
    BEGIN_COPYING_MEMBERS
        if (ty != kCopyFromMax) {
            COPY_MEMBER(mShaderOptions)
            COPY_MEMBER(unk20c)
            COPY_MEMBER(mColorMod)
            COPY_MEMBER(mMetaMaterial)
        }
        mDirty = 3;
        UpdatePropertiesFromMetaMat();
    END_COPYING_MEMBERS
END_COPYS

BEGIN_LOADS(RndMat)
    LOAD_REVS(bs)
    ASSERT_REVS(0x46, 0)
    MILO_ASSERT_FMT(
        d.rev >= 0x19,
        "%s can't load old %s version %d < %d.  Use RB2 Milo to load.",
        PathName(this),
        ClassName(),
        d.rev,
        0x19
    );
    mDirty = 3;
    ResetColors(mColorMod, 3);
    if (d.rev < 0x45) {
        LoadOld(d);
    } else {
        BaseMaterial::Load(d.stream);
    }
    if (d.rev > 0x45) {
        mMetaMaterial.Load(d.stream, true, sMetaMaterials);
    }
    UpdatePropertiesFromMetaMat();
END_LOADS

void RndMat::Init() {
    REGISTER_OBJ_FACTORY(RndMat);
    RndMat *mat = Hmx::Object::New<RndMat>();
    BaseMaterial::SetDefaultMat(mat);
    RELEASE(sMetaMaterials);
    sMetaMaterials = LoadMetaMaterials();
    int hashsize = (sMetaMaterials->HashTableUsedSize() + 200) * 2;
    sMetaMaterials->Reserve(hashsize, sMetaMaterials->StrTableUsedSize() + 4400);
    CreateAndSetMetaMat(mat);
}

void RndMat::Terminate() { RELEASE(sMetaMaterials); }

void RndMat::ReloadMetaMaterials() {
    ObjectDir *metaDir = LoadMetaMaterials();
    if (metaDir != nullptr && sMetaMaterials != nullptr) {
        MergeMetaMaterials(sMetaMaterials, metaDir);
        for (ObjDirItr<MetaMaterial> it(sMetaMaterials, true); it != 0; ++it) {
            MetaMaterial *mat = metaDir->Find<MetaMaterial>(it->Name(), false);
            if (mat == 0) {
                delete &*it;
                ObjDirItr<MetaMaterial> tmp(0, true);
                it = tmp;
            }
        }
    }
}

float RndMat::GetRefractStrength() { return mRefractStrength; }
RndTex *RndMat::GetRefractNormalMap() {
    return mRefractNormalMap ? mRefractNormalMap : mNormalMap;
}

bool RndMat::GetRefractEnabled(bool b1) {
    if (mRefractEnabled == 1 && mRefractStrength > 0.0f) {
        RndTex *tex = mRefractNormalMap ? mRefractNormalMap : mNormalMap;
        if (tex && (b1 || TheRnd.GetCurrentFrameTex(false))) {
            return true;
        }
    }
    return false;
}

MatPropEditAction RndMat::GetMetaMatPropAction(Symbol s) {
    String str(s);
    str += "_edit_action";
    const DataNode *node = mMetaMaterial->Property(str.c_str(), true);
    MILO_ASSERT(node, 0x2BE);
    return (MatPropEditAction)node->Int();
}

bool RndMat::OnGetPropertyDisplay(PropDisplay display, Symbol s) {
    MILO_ASSERT(display == kPropDisplayHidden || display == kPropDisplayReadOnly, 0x357);
    if (mMetaMaterial) {
        if (mToggleDisplayAllProps)
            return display == kPropDisplayHidden;
        else
            return true;
    } else {
        MatPropEditAction a = GetMetaMatPropAction(s);
        return a == 2 || (a != 0 && a != 1);
    }
    return false;
}

void RndMat::SetColorMod(const Hmx::Color &color, int index) {
    MILO_ASSERT(index >= 0 && index < kColorModNum, 0x230);
    mColorMod[index] = color;
    mDirty |= 2;
}

DataNode RndMat::OnGetMetaMaterialsDir(const DataArray *) { return sMetaMaterials; }

RndMat *LookupOrCreateMat(const char *shader, ObjectDir *dir) {
    const char *fileStr = MakeString("%s.mat", FileGetBase(shader));
    RndMat *mat = dir->Find<RndMat>(fileStr, false);
    if (!mat) {
        mat = dir->Find<RndMat>(FileGetBase(shader), false);
        if (!mat) {
            bool old = TheLoadMgr.EditMode();
            TheLoadMgr.SetEditMode(true);
            mat = dir->New<RndMat>(fileStr);
            TheLoadMgr.SetEditMode(old);
        }
    }
    return mat;
}

void RndMat::SetSpecularMap(RndTex *tex) {
    if (tex && !mSpecularMap) {
        if (mSpecularRGB.Pack() == 0) {
            mSpecularRGB.Set(1, 1, 1, mSpecularRGB.alpha);
        }
    }
    mSpecularMap = tex;
    mDirty |= 2;
}

void RndMat::SetMetaMat(MetaMaterial *mat, bool b) {
    mMetaMaterial = mat;
    UpdatePropertiesFromMetaMat();
    unk225 = b;
}

void RndMat::UpdateAllMatPropertiesFromMetaMat(ObjectDir *dir) {
    for (ObjDirItr<RndMat> it(dir, true); it != nullptr; ++it) {
        it->UpdatePropertiesFromMetaMat();
    }
}

ObjectDir *RndMat::LoadMetaMaterials() {
    const char *path = "";
    ObjectDir *dir = nullptr;
    DataArray *cfg = SystemConfig("objects", "Mat");
    if (cfg->FindData("metamaterial_path", path, false) && *path != '\0') {
        {
            FilePathTracker tracker(path);
            dir = DirLoader::LoadObjects("metamaterials.milo", nullptr, nullptr);
            MILO_ASSERT(dir, 0x99);
        }
        if (!strstr("system/run", FilePath::Root().c_str()) && TheLoadMgr.EditMode()) {
            ObjectDir *loadedDir = DirLoader::LoadObjects(
                "../../system/run/config/metamaterials.milo", nullptr, nullptr
            );
            MergeMetaMaterials(dir, loadedDir);
            delete loadedDir;
        }
    }
    return dir;
}

DataNode RndMat::OnGetMetaMaterials(const DataArray *a) {
    bool i2 = a->Int(2);
    int numMetaMats = 0;
    if (sMetaMaterials) {
        for (ObjDirItr<MetaMaterial> it(sMetaMaterials, true); it != nullptr; ++it) {
            numMetaMats++;
        }
    }
    DataArrayPtr ptr;
    ptr->Resize(numMetaMats + 1);
    ptr->Node(0) = NULL_OBJ;
    if (sMetaMaterials) {
        int idx = 1;
        for (ObjDirItr<MetaMaterial> it(sMetaMaterials, true); it != nullptr; ++it) {
            const char *name = it->Name();
            if (!strstr(name, kAnonMetaMatPrefix)
                && (i2 || !strstr(name, kMiloMetaMatPrefix))) {
                ptr->Node(idx++) = &*it;
            }
        }
        ptr->Resize(idx);
        ptr->SortNodes(0);
    }
    return ptr;
}

MetaMaterial *RndMat::CreateMetaMaterial(bool notify) {
    bool hasStr = false;
    String str(Name());
    if (str.empty()) {
        hasStr = true;
        str = kAnonMetaMatPrefix;
        str += ".";
        str += ClassExt("Mat");
    }
    int extlen = strlen(ClassExt("Mat"));
    if (strlen(str.c_str()) > extlen) {
        str.erase(str.length() - extlen);
    }
    str += ClassExt("MetaMaterial");
    MILO_ASSERT(sMetaMaterials, 0x30A);

    const char *nextname = NextName(str.c_str(), sMetaMaterials);
    MetaMaterial *mat = Hmx::Object::New<MetaMaterial>();
    mat->SetName(nextname, sMetaMaterials);
    mat->Copy(this, kCopyDeep);
    String strc0;
    std::list<Symbol> symList;
    ListProperties(symList, "Mat", 0, nullptr, false);
    for (std::list<Symbol>::iterator it = symList.begin(); it != symList.end(); ++it) {
        Symbol cur = *it;
        static Symbol metamaterial("metamaterial");
        if (cur != metamaterial) {
            bool b10 = false;
            const DataNode *node = Property(cur, true);
            if (node && node->Type() == kDataObject && node->GetObj()) {
                if (!strc0.empty()) {
                    strc0 += ", ";
                }
                strc0 += cur.Str();
                mat->SetProperty(cur, NULL_OBJ);
                b10 = true;
            }
            bool b9 = PropValDifferent(cur, nullptr);
            String stre0(cur);
            stre0 += "_edit_action";
            mat->SetProperty(stre0.c_str(), b10 ? 2 : b9 != 0);
        }
    }
    if (hasStr) {
        for (ObjDirItr<MetaMaterial> it(sMetaMaterials, true); it != nullptr; ++it) {
            if (mat != it && mat->IsEquivalent(it)) {
                delete mat;
                mat = it;
                break;
            }
        }
    }
    if (notify) {
        if (!strc0.empty()) {
            MILO_NOTIFY(
                "Some object properties were not copied to the MetaMaterial:%s",
                strc0.c_str()
            );
        }
    }
    return mat;
}

bool RndMat::IsEditable(Symbol s) {
    if (mMetaMaterial && !unk226) {
        bool ret = mMetaMaterial->Property(s, true)->Int() == 2;
        if (!ret) {
            String str(s);
            int len = str.length();
            if (len > 12) {
                str = str.substr(0, len - 12);
            }
            MILO_NOTIFY_ONCE(
                "Unable to set property %s in Mat %s.  Not allowed by MetaMaterial %s.\n",
                str.c_str(),
                PathName(this),
                PathName(mMetaMaterial)
            );
        }
        return ret;
    }
    return true;
}

void RndMat::UpdatePropertiesFromMetaMat() {
    if (mMetaMaterial) {
        unk226 = true;
        String str70;
        std::list<Symbol> syms78;
        ListProperties(syms78, 0, "Mat", nullptr, false);
        for (std::list<Symbol>::iterator it = syms78.begin(); it != syms78.end(); ++it) {
            static Symbol metamaterial("metamaterial");
            Symbol cur = *it;
            if (cur != metamaterial) {
                MatPropEditAction a =
                    GetMetaMatPropAction(cur == "alpha_threshold" ? "alpha_cut" : cur);
                if ((a == kPropDefault || a == kPropForce)
                    && PropValDifferent(cur, mMetaMaterial)) {
                    if (cur == "tex_xfm") {
                        mTexXfm = mMetaMaterial->TexXfm();
                        mDirty |= 2;
                    } else {
                        SetProperty(cur, *mMetaMaterial->Property(cur, true));
                    }
                    AddOverridePropName(str70, cur);
                }
            }
        }
        if (!str70.empty())
            str70 += ".";
        unk226 = false;
    }
    mDirty |= 2;
}

void RndMat::LoadOld(BinStreamRev &bs) {
    Hmx::Object::Load(bs.stream);
    bs >> (int &)mBlend;
    mBlend = CheckBlendMode(mBlend, this);
    bs >> mColor;
    bs >> mUseEnviron >> mPrelit;
    bs >> (int &)mZMode;
    bs >> mAlphaCut;
    if (bs.rev > 0x25) {
        bs >> mAlphaThreshold;
    }
    bs >> mAlphaWrite;
    bs >> (int &)mTexGen;
    bs >> (int &)mTexWrap;
    bs >> mTexXfm;
    bs >> mDiffuseTex;
    bs >> mNextPass;
    bs.stream >> mIntensify;
    bool bCull;
    bs >> bCull;
    mCull = (Cull)bCull;
    bs >> mEmissiveMultiplier;
    bs >> mSpecularRGB;
    bs >> mNormalMap;
    bs >> mEmissiveMap;
    bs >> mSpecularMap;
    if (bs.rev < 0x33) {
        ObjPtr<RndTex> tex(this);
        bs >> tex;
    }
    bs >> mEnvironMap;
    if (bs.rev > 0x3C) {
        bs >> mEnvironMapFalloff;
        if (bs.rev > 0x42) {
            bs >> mEnvironMapSpecMask;
        }
    }
    if (bs.rev < 0x25) {
        if (mSpecularMap) {
            mSpecularRGB.Set(1, 1, 1);
        }
    }
    if (bs.rev > 0x19) {
        bs >> mPerPixelLit;
    }
    if (bs.rev > 0x1A && bs.rev < 0x32) {
        bool b150;
        bs >> b150;
    }
    if (bs.rev > 0x1B) {
        bs >> (int &)mStencilMode;
    }
    if (bs.rev < 0x29 && bs.rev > 0x1C) {
        Symbol s;
        bs >> s;
    }
    if (bs.rev > 0x20) {
        bs >> mFur;
    } else if (bs.rev > 0x1D) {
        bool old = TheLoadMgr.EditMode();
        TheLoadMgr.SetEditMode(true);
        const char *name = MakeString("%s.fur", FileGetBase(Name()));
        ObjectDir *dir = Dir();
        RndFur *fur = Hmx::Object::New<RndFur>();
        if (name) {
            fur->SetName(name, dir);
        }
        TheLoadMgr.SetEditMode(old);
        if (fur->LoadOld(bs)) {
            mFur = fur;
        } else {
            delete fur;
            mFur = nullptr;
        }
    }
    if (bs.rev > 0x21 && bs.rev < 0x31) {
        bool b150;
        Hmx::Color c80;
        bs >> b150 >> c80;
        if (bs.rev > 0x22) {
            ObjPtr<RndTex> tex(this);
            bs >> tex;
        }
    }
    if (bs.rev > 0x23) {
        bs >> mDeNormal;
        bs >> mAnisotropy;
    }
    if (bs.rev > 0x26) {
        if (bs.rev < 0x2A) {
            bool b150;
            bs >> b150;
        }
        bs >> mNormDetailTiling;
        bs >> mNormDetailStrength;
        if (bs.rev < 0x2A) {
            int x;
            Hmx::Color c;
            bs >> x;
            bs >> c;
        }
        bs >> mNormDetailMap;
        if (bs.rev < 0x2A) {
            ObjPtr<RndTex> tex(this);
            bs >> tex;
        }
        if (bs.rev < 0x28) {
            mNormDetailStrength = 0;
        }
    }
    if (bs.rev > 0x2A) {
        if (bs.rev > 0x2C) {
            bs >> mPointLights;
        } else {
            int x;
            bs >> x;
            mPointLights = x == 1;
        }
        if (bs.rev < 0x3F) {
            bool b150;
            bs >> b150;
        }
        bs >> mFog >> mFadeout;
        if (bs.rev > 0x2B && bs.rev < 0x2E) {
            bool b150;
            bs >> b150;
        }
        if (bs.rev > 0x2E) {
            bs >> mColorAdjust;
        }
    }
    if (bs.rev > 0x2F) {
        bs >> mRimRGB;
        bs >> mRimMap;
        if (bs.rev >= 0x3A) {
            bs >> mRimLightUnder;
        } else {
            bool b150;
            bs >> b150;
            float red = mRimRGB.red * 2.857143f;
            float green = mRimRGB.green * 2.857143f;
            float blue = mRimRGB.blue * 2.857143f;
            mRimRGB.red = Min(red, 1.0f);
            mRimRGB.green = Min(green, 1.0f);
            mRimRGB.blue = Min(blue, 1.0f);
        }
        if (bs.rev < 0x3B) {
            mRimRGB.Set(0, 0, 0);
        }
    }
    if (bs.rev > 0x30) {
        bs >> mScreenAligned;
    }
    if (bs.rev > 0x31 && bs.rev < 0x33) {
        bool b150;
        bs >> b150;
        if (b150) {
            mShaderVariation = kShaderVariationSkin;
        }
    }
    if (bs.rev > 0x32) {
        bs >> (int &)mShaderVariation;
        bs >> mSpecular2RGB;
    }
    if (bs.rev > 0x33 && bs.rev < 0x44) {
        std::vector<Hmx::Color> colors;
        if (bs.rev < 0x35) {
            bool x;
            bs >> x;
        } else {
            int x;
            bs >> x;
        }
        if (bs.rev > 0x34 && bs.rev < 0x3C) {
            Hmx::Color c;
            bs >> c;
        }
        if (bs.rev >= 0x3C) {
            bs >> colors;
        }
    }
    if (bs.rev > 0x35 && bs.rev < 0x3E) {
        ObjPtr<Hmx::Object> obj(this);
        bs >> obj;
    }
    if (bs.rev > 0x36 && bs.rev < 0x3F) {
        bool b150;
        bs >> b150;
        mPerfSettings.mPS3ForceTrilinear = b150;
    }
    if (bs.rev > 0x37 && bs.rev < 0x39) {
        int x, y;
        bs >> x >> y;
    }
    if (bs.rev > 0x3E) {
        mPerfSettings.LoadOld(bs);
    }
    if (bs.rev > 0x3F) {
        bs >> mRefractEnabled;
        bs >> mRefractStrength;
        bs >> mRefractNormalMap;
        if (bs.rev < 0x41) {
            if (mRefractEnabled) {
                mRefractStrength *= 0.15f;
            } else {
                mRefractStrength = 0;
            }
        }
    }
}
