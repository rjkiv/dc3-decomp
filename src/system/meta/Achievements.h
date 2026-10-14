#pragma once
#include "obj/Object.h"
#include "xdk/XAPILIB.h"
#include <vector>

/**
 * @brief Handles Achievements.
 */
// size 0x40
class Achievements : public Hmx::Object {
private:
    Achievements();

    int mProfileSaveSize; // 0x2c
    bool mAllowAchievements; // 0x30
    std::vector<XUSER_ACHIEVEMENT> gAchievements; // 0x34

    XUSER_ACHIEVEMENT GetAchievementData(int, int);

    static std::vector<XUSER_ACHIEVEMENT> gThreadAchievements;
    static int SubmitAchievementsFunc();
    static void SubmitAchievementsCallback(int);

public:
    virtual DataNode Handle(DataArray *, bool);

    void Poll();
    void Submit(int, Symbol, int);
    void SetAllowAchievements(bool allow) { mAllowAchievements = allow; }

    static void Init();
    static void PlatformInit();
    static void Terminate();
};

extern Achievements *TheAchievements;
