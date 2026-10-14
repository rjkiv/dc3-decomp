#pragma once
#include "gesture/Skeleton.h"
#include "gesture/ArchiveSkeleton.h"
#include <vector>

class SkeletonHistoryArchive {
public:
    SkeletonHistoryArchive();
    ~SkeletonHistoryArchive() {}

    const std::vector<ArchiveSkeleton> &GetArchive(int) const;
    void AddToHistory(int, const Skeleton &);
    void ClearHistory(int);

protected:
    std::vector<ArchiveSkeleton> mHistories[6]; // 0x0
};

class SkeletonHistory {
public:
    virtual bool PrevSkeleton(
        const Skeleton &current,
        int previous_ms,
        ArchiveSkeleton &previous,
        int &elapsed_ms
    ) const = 0;

protected:
    bool PrevFromArchive(
        const SkeletonHistoryArchive &history,
        const Skeleton &current,
        int previous_ms,
        ArchiveSkeleton &previous,
        int &elapsed_ms
    ) const;
};
