#include "hamobj/MoveMgr.h"
#include "SongUtl.h"
#include "flow/PropertyEventProvider.h"
#include "hamobj/HamDirector.h"
#include "hamobj/MoveGraph.h"
#include "hamobj/SongLayout.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "rndobj/PropAnim.h"
#include "stl/_algo.h"

MoveMgr::MoveMgr() : unk40(0), unka0(0) {
    unk9c = 0;
    for (int i = 0; i < 3; i++) {
        unk54[i].clear();
        unk48[i] = 0;
        unk2c[i] = 0;
    }
    unk3c = 0;
    unk168 = false;
    unk14c = "";
    //   pSVar1 = Hmx::Object::New<>();
    //   *(this + 0x1ac) = pSVar1;
    //   puVar2 = Symbol::Symbol(local_5c,"easeup_remixer");
    //   (**(**(this + 0x1ac) + 0x10))(*(this + 0x1ac),*puVar2);
    unk1a8 = nullptr;
    unk44 = dynamic_cast<SongLayout *>(Hmx::Object::NewObject("SongLayout"));
}

MoveMgr::~MoveMgr() {}

void MoveMgr::Clear() {
    unka0 = 0;
    unk9c = 0;
    for (int i = 0; i < 3; i++) {
        unk54[i].clear();
        unk48[i] = 0;
        unk2c[i] = 0;
    }
    unk3c = 0;
    unk40 = 0;
    unk104.clear();
    for (int i = 0; i < 2; i++) {
        unk150[i].clear();
        unk11c[i].clear();
        unk134[i].clear();
    }
    unk168 = false;
    unk16c.clear();
    unk178.clear();
    unk184.clear();
    unk190.clear();
    unk19c.clear();
    unk14c = "";
    mMoveGraph.Clear();
    unk1a8 = nullptr;
}

bool MoveVariantsWithHamMove(const MoveVariant *var, void *v) {
    int tag = var->HamMoveName();
    int *iv = (int *)v;
    return tag == *iv;
}

bool MoveMgr::HasRoutine() const {
    static Symbol gameplay_mode("gameplay_mode");
    static Symbol practice("practice");
    if (unk168) {
        if (TheHamProvider->Property(gameplay_mode, true)->Sym() != practice) {
            return true;
        }
    }
    return false;
}

void MoveMgr::InsertMoveInSong(const MoveVariant *var, int i2, int i3) {
    static Symbol clip("clip");
    static Symbol move("move");
    static Symbol practice("practice");
    if (var) {
        Symbol name = var->Name();
        float beat = i2 * 4;
        float f4 = BeatToFrame(beat);
        float f6 = BeatToFrame(i2 > 0 ? beat - 1 : 0);
        RndPropAnim *anim = TheHamDirector->SongAnim(i3);
        DataArrayPtr ptr90(clip);
        DataArrayPtr ptr88(move);
        anim->SetKeyVal(TheHamDirector, ptr90, f6, name, true);
        anim->SetKeyVal(TheHamDirector, ptr88, f4, var->HamMoveName(), true);
    }
}

void MoveMgr::SaveRoutine(DataArray *a) const {
    a->Resize(unk11c[0].size());
    int i = 0;
    for (std::vector<const MoveParent *>::const_iterator it = unk11c[0].begin();
         it != unk11c[0].end();
         ++it, ++i) {
        if (*it) {
            // a->Node(i) = (*it)->unk4;
        } else {
            a->Node(i) = 0;
        }
    }
}

void MoveMgr::GenerateMoveChoice(
    Symbol s1,
    std::vector<const MoveVariant *> &vec1,
    std::vector<const MoveVariant *> &vec2
) {
    vec1.clear();
    vec2.clear();
    MILO_LOG("MoveMgr::GenerateMoveChoice %s\n", s1.Str());
    const std::map<Symbol, MoveParent *> &parents = MoveParents();
    int invalidNum = 0;
    for (std::map<Symbol, MoveParent *>::const_iterator it = parents.begin();
         it != parents.end();
         ++it) {
        MoveParent *curParent = it->second;
        const MoveVariant *randomVar = curParent->PickRandomVariant();
        MILO_LOG("move=%s genre=%s", randomVar->Name(), randomVar->Genre().Str());
        if (!randomVar->IsValidForMinigame()) {
            MILO_LOG(" not valid for mini games\n");
            invalidNum++;
        } else {
            if (curParent->HasCategory(s1)) {
                MILO_LOG(" is %s\n", s1.Str());
                vec1.push_back(randomVar);
            } else {
                MILO_LOG(" NOT %s\n", s1.Str());
                vec2.push_back(randomVar);
            }
        }
    }
    MILO_LOG(
        "invalid=%d, wrong genre=%d right genre=%d\n", invalidNum, vec2.size(), vec1.size()
    );
    std::random_shuffle(vec1.begin(), vec1.end());
    std::random_shuffle(vec2.begin(), vec2.end());
}
