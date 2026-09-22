#include "gesture/SkeletonHistory.h"
#include "ArchiveSkeleton.h"
#include "os/Debug.h"
#include "utl/Std.h"

SkeletonHistoryArchive::SkeletonHistoryArchive() {
    for (int i = 0; i < 6; i++) {
        mHistories[i].reserve(0xA0);
    }
}

const std::vector<ArchiveSkeleton> &
SkeletonHistoryArchive::GetArchive(int skel_idx) const {
    MILO_ASSERT((0) <= (skel_idx) && (skel_idx) < (6), 0x36);
    return mHistories[skel_idx];
}

bool SkeletonHistory::PrevFromArchive(
    const SkeletonHistoryArchive &archives,
    const Skeleton &skeleton,
    int i3,
    ArchiveSkeleton &archiveSkeleton,
    int &elapsedMs
) const {
    int skel_idx = skeleton.SkeletonIndex();
    MILO_ASSERT_RANGE(skel_idx, 0, 6, 0x13);
    const std::vector<ArchiveSkeleton> &archive = archives.GetArchive(skel_idx);
    std::vector<ArchiveSkeleton>::const_iterator it = archive.begin();
    elapsedMs = skeleton.ElapsedMs();
    for (; it != archive.end(); ++it) {
        if (elapsedMs >= i3)
            break;
        elapsedMs += it->ElapsedMs();
    }

    if (it != archive.end()) {
        archiveSkeleton = *it;
        return true;
    } else
        return false;
}

void SkeletonHistoryArchive::ClearHistory(int skel_idx) {
    MILO_ASSERT((0) <= (skel_idx) && (skel_idx) < (6), 0x4C);
    mHistories[skel_idx].clear();
}

void SkeletonHistoryArchive::AddToHistory(int skel_idx, const Skeleton &skeleton) {
    MILO_ASSERT((0) <= (skel_idx) && (skel_idx) < (6), 0x3F);

    auto &histories = mHistories[skel_idx];
    while (histories.size() >= 160) {
        histories.pop_back();
    }

    ArchiveSkeleton aSkeleton;
    aSkeleton.Set(skeleton);
    histories.insert(histories.begin(), aSkeleton);
}
