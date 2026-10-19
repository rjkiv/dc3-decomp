#include "obj/Object.h"
#include "os/Debug.h"
#include "rndobj/Font.h"
#include "rndobj/FontBase.h"

bool RndFont3d::HasChar(unsigned short us) const {
    return mCharInfos.find(us) != mCharInfos.end();
}

RndFont3d::RndFont3d()
    : mMat(this), mCharGeomOwner(this, this), mCellSize(0, 0, 0), mNormalize(0, 0, 0),
      mCharOriginOffset(0, 0, 0) {}

BEGIN_HANDLERS(RndFont3d)
    HANDLE_SUPERCLASS(RndFontBase)
END_HANDLERS

BEGIN_PROPSYNCS(RndFont3d)
    SYNC_SUPERCLASS(RndFontBase)
END_PROPSYNCS

BEGIN_SAVES(RndFont3d)
    SAVE_REVS(0, 0)
    SAVE_SUPERCLASS(RndFontBase)
    bs << mMat;
    bs << mCharGeomOwner;
    bs << mCellSize;
    bs << mNormalize;
    bs << mCharOriginOffset;
    bs << mCharInfos.size();
    FOREACH (it, mCharInfos) {
        bs << it->first;
        CharInfo *value = it->second;
        bs << value->bbox;
        bs << value->charAdvance;
        bs << value->mesh;
        bs << value->handmade;
    }
END_SAVES

BEGIN_COPYS(RndFont3d)
    COPY_SUPERCLASS(RndFontBase)
    CREATE_COPY_AS(RndFont3d, f)
    MILO_ASSERT(f, 0x91);
    COPY_MEMBER_FROM(f, mMat)
    COPY_MEMBER_FROM(f, mCellSize)
    COPY_MEMBER_FROM(f, mNormalize)
    COPY_MEMBER_FROM(f, mCharOriginOffset)
    FOREACH (it, mCharInfos) {
        delete it->second;
    }
    mCharInfos.clear();
    FOREACH (it, f->mCharInfos) {
        CharInfo *c = new CharInfo(this);
        *c = *(it->second);
        mCharInfos[it->first] = c;
    }
    if (ty == kCopyShallow || (ty == kCopyFromMax && f->mCharGeomOwner != f)) {
        mCharGeomOwner = f->mCharGeomOwner.Ptr();
    } else {
        mCharGeomOwner = this;
    }
END_COPYS

INIT_REVS(0, 0)

BEGIN_LOADS(RndFont3d)
    LOAD_REVS(bs)
    ASSERT_REVS(0, 0)
    LOAD_SUPERCLASS(RndFontBase)
    d >> mMat;
    d >> mCharGeomOwner;
    d >> mCellSize;
    d >> mNormalize;
    d >> mCharOriginOffset;
    size_t numMapEntries;
    d >> numMapEntries;
    for (int i = 0; i < numMapEntries; i++) {
        unsigned short key;
        d >> key;
        CharInfo *c = new CharInfo(this);
        d >> c->bbox;
        d >> c->charAdvance;
        d >> c->mesh;
        d >> c->handmade;
        mCharInfos[key] = c;
    }
END_LOADS

float RndFont3d::CharWidth(unsigned short c) const {
    if (mCharGeomOwner != this) {
        return mCharGeomOwner->CharWidth(c);
    } else {
        MILO_ASSERT(HasChar(c), 0xCA);
        auto it = mCharGeomOwner->mCharInfos.find(c);
        float w = Max(it->second->bbox.mMax.x, 0.0f);
        MILO_ASSERT(w >= 0.f, 0xCC);
        return w * FontUnitInverse();
    }
}

bool RndFont3d::CharAdvance(unsigned short us1, unsigned short us2, float &fref) const {
    if (mCharGeomOwner != this) {
        return mCharGeomOwner->CharAdvance(us1, us2, fref);
    } else {
        auto it = mCharInfos.find(us2);
        if (it != mCharInfos.end()) {
            CharInfo *c = it->second;
            float vol = c->bbox.Volume();
            if (vol > 0 || c->charAdvance > 0) {
                if (mMonospace) {
                    fref = 1;
                } else {
                    fref = FontUnitInverse() * c->charAdvance;
                }
                fref += Kerning(us1, us2);
                return true;
            }
        }
        return false;
    }
}

float RndFont3d::CharAdvance(unsigned short c) const {
    if (mCharGeomOwner != this) {
        return mCharGeomOwner->CharAdvance(c);
    } else {
        MILO_ASSERT(HasChar(c), 0xD5);
        if (mMonospace) {
            return 1;
        } else {
            float a = mCharInfos.find(c)->second->charAdvance;
            MILO_ASSERT(a >= 0.f, 0xDB);
            return a * FontUnitInverse();
        }
    }
}

float RndFont3d::Kerning(unsigned short us1, unsigned short us2) const {
    if (mCharGeomOwner != this) {
        return mCharGeomOwner->Kerning(us1, us2);
    } else {
        return mNormalize.x * RndFontBase::Kerning(us1, us2);
    }
}

float RndFont3d::AspectRatio() const {
    return mCharGeomOwner->mCellSize.z / mCharGeomOwner->mCellSize.x;
}

void RndFont3d::Clear() {
    FOREACH (it, mCharInfos) {
        delete it->second;
    }
    mCharInfos.clear();
    mChars.clear();
    RELEASE(mKerning);
}

RndFont3d::CharInfo *RndFont3d::GetCharInfo(unsigned short us1) const {
    if (mCharGeomOwner != this) {
        // aintnoway
        while (true)
            ;
    }
    auto it = mCharInfos.find(us1);
    if (it != mCharInfos.end()) {
        return it->second;
    } else {
        return nullptr;
    }
}

Vector3 RndFont3d::CharOriginOffset() const {
    if (mCharGeomOwner != this) {
        return mCharGeomOwner->CharOriginOffset();
    } else {
        Vector3 out;
        Scale(mCharOriginOffset, FontUnitInverse(), out);
        return out;
    }
}

bool RndFont3d::CharWidthAdvanceMesh(
    unsigned short us1, float &f2, float &f3, RndMesh **meshPtr
) const {
    if (mCharGeomOwner != this) {
        return mCharGeomOwner->CharWidthAdvanceMesh(us1, f2, f3, meshPtr);
    }
    auto it = mCharInfos.find(us1);
    if (it != mCharInfos.end()) {
        CharInfo *c = it->second;
        float f7 = c->bbox.Volume();
        if (f7 > 0 || c->charAdvance > 0) {
            f2 = FontUnitInverse() * Max(c->bbox.mMax.x, 0.0f);
            f3 = mMonospace ? 1 : FontUnitInverse() * c->charAdvance;
            *meshPtr = c->mesh;
            return true;
        }
    }
    return false;
}
