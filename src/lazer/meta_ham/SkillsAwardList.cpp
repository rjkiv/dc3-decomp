#include "SkillsAwardList.h"

#include "hamobj/HamGameData.h"

bool SkillsAwardList::HasFailure() const {
    FOREACH(it, unk8) {
        if (!it->second) {
            return true;
        }
    }
    return false;
}

int SkillsAwardList::SaveSize(int i1) {
    return 0x60004;
}

SkillsAward SkillsAwardList::GetAward(DataArray *da) {
    return GetAward(TheGameData->GetSong(), da);
}

int SkillsAwardList::AwardCount(SkillsAward sa) const {
    int count = 0;
    FOREACH(it, unk8) {
        if (it->second == sa) {
            count++;
        }
    }
    return count;
}

void SkillsAwardList::Clear() {
    unk8.clear();
    unk20 = false;
}