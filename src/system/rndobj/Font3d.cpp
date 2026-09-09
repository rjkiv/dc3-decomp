#include "obj/Object.h"
#include "os/Debug.h"
#include "rndobj/Font.h"
#include "rndobj/FontBase.h"

bool RndFont3d::HasChar(unsigned short us) const {
    return mCharInfoMap.find(us) != mCharInfoMap.end();
}

RndFont3d::RndFont3d()
    : unk44(this), mTextureOwner(this, this), unk6c(0, 0, 0), unk7c(0, 0, 0),
      unk8c(0, 0, 0) {}

BEGIN_HANDLERS(RndFont3d)
    HANDLE_SUPERCLASS(RndFontBase)
END_HANDLERS

BEGIN_PROPSYNCS(RndFont3d)
    SYNC_SUPERCLASS(RndFontBase)
END_PROPSYNCS

BEGIN_SAVES(RndFont3d)
    SAVE_REVS(0, 0)
    SAVE_SUPERCLASS(RndFontBase)
    bs << unk44;
    bs << mTextureOwner;
    bs << unk6c;
    bs << unk7c;
    bs << unk8c;
    bs << mCharInfoMap.size();
    FOREACH (it, mCharInfoMap) {
        bs << it->first;
        CharInfo *value = it->second;
        bs << value->unk0;
        bs << value->unk20;
        bs << value->unk24;
        bs << value->unk38;
    }
END_SAVES

BEGIN_COPYS(RndFont3d)
    COPY_SUPERCLASS(RndFontBase)
    CREATE_COPY_AS(RndFont3d, f)
    MILO_ASSERT(f, 0x91);
    COPY_MEMBER_FROM(f, unk44)
    COPY_MEMBER_FROM(f, unk6c)
    COPY_MEMBER_FROM(f, unk7c)
    COPY_MEMBER_FROM(f, unk8c)
    FOREACH (it, mCharInfoMap) {
        delete it->second;
    }
    mCharInfoMap.clear();
    FOREACH (it, f->mCharInfoMap) {
        CharInfo *c = new CharInfo(this);
        *c = *(it->second);
        mCharInfoMap[it->first] = c;
    }
    switch (ty) {
    case Hmx::Object::kCopyShallow:
        mTextureOwner = f->mTextureOwner.Ptr();
        break;
    case Hmx::Object::kCopyFromMax:
        mTextureOwner = f->mTextureOwner != f ? f->mTextureOwner.Ptr() : this;
        break;
    default:
        mTextureOwner = this;
        break;
    }
END_COPYS

INIT_REVS(0, 0)

BEGIN_LOADS(RndFont3d)
    LOAD_REVS(bs)
    ASSERT_REVS(0, 0)
    LOAD_SUPERCLASS(RndFontBase)
    d >> unk44;
    d >> mTextureOwner;
    d >> unk6c;
    d >> unk7c;
    d >> unk8c;
    size_t numMapEntries;
    d >> numMapEntries;
    for (int i = 0; i < numMapEntries; i++) {
        unsigned short key;
        d >> key;
        CharInfo *c = new CharInfo(this);
        d >> c->unk0;
        d >> c->unk20;
        d >> c->unk24;
        d >> c->unk38;
        mCharInfoMap[key] = c;
    }
END_LOADS

float RndFont3d::CharWidth(unsigned short c) const {
    if (mTextureOwner != this) {
        return mTextureOwner->CharWidth(c);
    } else {
        MILO_ASSERT(HasChar(c), 0xCA);
        auto it = mTextureOwner->mCharInfoMap.find(c);
        float w = Max(it->second->unk0.mMax.x, 0.0f);
        MILO_ASSERT(w >= 0.f, 0xCC);
        return w * FontUnitInverse();
    }
}

bool RndFont3d::CharAdvance(unsigned short us1, unsigned short us2, float &fref) const {
    if (mTextureOwner != this) {
        return mTextureOwner->CharAdvance(us1, us2, fref);
    } else {
        auto it = mCharInfoMap.find(us2);
        if (it != mCharInfoMap.end()) {
            CharInfo *c = it->second;
            float vol = c->unk0.Volume();
            if (vol > 0 || c->unk20 > 0) {
                if (mMonospace) {
                    fref = 1;
                } else {
                    fref = FontUnitInverse() * c->unk20;
                }
                fref += Kerning(us1, us2);
                return true;
            }
        }
        return false;
    }
}

float RndFont3d::CharAdvance(unsigned short c) const {
    if (mTextureOwner != this) {
        return mTextureOwner->CharAdvance(c);
    } else {
        MILO_ASSERT(HasChar(c), 0xD5);
        if (mMonospace) {
            return 1;
        } else {
            float a = mCharInfoMap.find(c)->second->unk20;
            MILO_ASSERT(a >= 0.f, 0xDB);
            return a * FontUnitInverse();
        }
    }
}

float RndFont3d::Kerning(unsigned short us1, unsigned short us2) const {
    if (mTextureOwner != this) {
        return mTextureOwner->Kerning(us1, us2);
    } else {
        return unk7c.x * RndFontBase::Kerning(us1, us2);
    }
}

float RndFont3d::AspectRatio() const {
    return mTextureOwner->unk6c.z / mTextureOwner->unk6c.x;
}

void RndFont3d::Clear() {
    FOREACH (it, mCharInfoMap) {
        delete it->second;
    }
    mCharInfoMap.clear();
    mChars.clear();
    RELEASE(mKerningTable);
}

RndFont3d::CharInfo *RndFont3d::GetCharInfo(unsigned short us1) const {
    if (mTextureOwner != this) {
        // aintnoway
        while (true)
            ;
    }
    auto it = mCharInfoMap.find(us1);
    if (it != mCharInfoMap.end()) {
        return it->second;
    } else {
        return nullptr;
    }
}

Vector3 RndFont3d::CharOriginOffset() const {
    if (mTextureOwner != this) {
        return mTextureOwner->CharOriginOffset();
    } else {
        Vector3 out;
        Scale(unk8c, FontUnitInverse(), out);
        return out;
    }
}

bool RndFont3d::CharWidthAdvanceMesh(
    unsigned short us1, float &f2, float &f3, RndMesh **meshPtr
) const {
    if (mTextureOwner != this) {
        return mTextureOwner->CharWidthAdvanceMesh(us1, f2, f3, meshPtr);
    }
    auto it = mCharInfoMap.find(us1);
    if (it != mCharInfoMap.end()) {
        CharInfo *c = it->second;
        float f7 = c->unk0.Volume();
        if (f7 > 0 || c->unk20 > 0) {
            f2 = FontUnitInverse() * Max(c->unk0.mMax.x, 0.0f);
            f3 = mMonospace ? 1 : FontUnitInverse() * c->unk20;
            *meshPtr = c->unk24;
            return true;
        }
    }
    return false;
}
