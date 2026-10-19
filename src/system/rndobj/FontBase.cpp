#include "rndobj/FontBase.h"
#include "os/Debug.h"
#include "rndobj/Font.h"
#include "obj/Object.h"
#include "utl/BinStream.h"
#include "utl/UTF8.h"

RndFontBase::RndFontBase() : mMonospace(0), mBaseKerning(0), mKerning(nullptr) {}

BEGIN_HANDLERS(RndFontBase)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(RndFontBase)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(RndFontBase)
    SAVE_REVS(0, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    bs << mChars;
    bs << mMonospace;
    bs << mBaseKerning;
    bs << (mKerning != nullptr);
    if (mKerning) {
        mKerning->Save(bs);
    }
END_SAVES

BEGIN_COPYS(RndFontBase)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY_AS(RndFontBase, f)
    MILO_ASSERT(f, 0x42);
    COPY_MEMBER_FROM(f, mChars)
    COPY_MEMBER_FROM(f, mMonospace)
    if (ty != kCopyShallow) {
        if (ty != kCopyFromMax || f->DataOwner() == f) {
            mBaseKerning = f->DataOwner()->mBaseKerning;
            std::vector<KernInfo> kerns;
            f->DataOwner()->GetKerning(kerns);
            SetKerning(kerns);
        }
    }
END_COPYS

INIT_REVS(0, 0)

BEGIN_LOADS(RndFontBase)
    LOAD_REVS(bs)
    ASSERT_REVS(0, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    d >> mChars;
    d >> mMonospace;
    d >> mBaseKerning;
    bool newKerning;
    d >> newKerning;
    if (newKerning) {
        mKerning = new KerningTable();
        mKerning->Load(d, this);
    }
END_LOADS

float RndFontBase::Kerning(unsigned short us1, unsigned short us2) const {
    if (DataOwner() != this) {
        return DataOwner()->Kerning(us1, us2);
    } else if (us1 == 0 || us2 == 0)
        return 0;
    else if (!mMonospace && mKerning) {
        return mBaseKerning + mKerning->Kerning(us1, us2);
    } else
        return mBaseKerning;
}

bool RndFontBase::CharDefined(unsigned short us) const {
    if (DataOwner() != this) {
        return DataOwner()->CharDefined(us);
    } else
        return HasChar(us);
}

String RndFontBase::GetASCIIChars() const {
    if (DataOwner() != this) {
        return DataOwner()->GetASCIIChars();
    } else
        return WideVectorToASCII(mChars);
}

void RndFontBase::SetBaseKerning(float f1) {
    MILO_ASSERT(DataOwner() == this, 0x65);
    mBaseKerning = f1;
}

void RndFontBase::SetKerning(const std::vector<KernInfo> &kernInfo) {
    MILO_ASSERT(DataOwner() == this, 0x7C);
    if (kernInfo.empty()) {
        RELEASE(mKerning);
    } else {
        if (!mKerning) {
            mKerning = new KerningTable();
        }
        mKerning->SetKerning(kernInfo, this);
    }
}

void RndFontBase::SetASCIIChars(String str) {
    if (DataOwner() != this) {
        MILO_ASSERT(0, 0x167);
    } else {
        ASCIItoWideVector(mChars, str.c_str());
    }
}

bool RndFontBase::HasChar(unsigned short us) const {
    if (DataOwner() != this) {
        return DataOwner()->HasChar(us);
    } else {
        for (int i = 0; i < mChars.size(); i++) {
            if (mChars[i] == us)
                return true;
        }
        return false;
    }
}

void RndFontBase::GetKerning(std::vector<KernInfo> &kernInfo) const {
    const RndFontBase *owner;
    for (owner = this; owner->DataOwner() != owner; owner = owner->DataOwner())
        ;
    if (owner->mKerning) {
        owner->mKerning->GetKerning(kernInfo);
    } else {
        kernInfo.clear();
    }
}
