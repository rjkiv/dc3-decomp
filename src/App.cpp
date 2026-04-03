#include "App.h"
#include "ChecksumData_xbox.h"
#include "char/Char.h"
#include "flow/Flow.h"
#include "flow/FlowManager.h"
#include "flow/PropertyEventProvider.h"
#include "game/Game.h"
#include "game/GameMode.h"
#include "game/HamUserMgr.h"
#include "game/PartyModeMgr.h"
#include "game/PresenceMgr.h"
#include "gesture/GestureMgr.h"
#include "gesture/LiveCameraInput.h"
#include "gesture/SkeletonUpdate.h"
#include "hamobj/Ham.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamNavList.h"
#include "hamobj/HamPlayerData.h"
#include "hamobj/HamWardrobe.h"
#include "hamobj/MiniGameMgr.h"
#include "hamobj/MoveMgr.h"
#include "meta/Achievements.h"
#include "meta/FixedSizeSaveable.h"
#include "meta_ham/AccomplishmentManager.h"
#include "meta_ham/Challenges.h"
#include "meta_ham/ContextChecker.h"
#include "meta_ham/HamSongMgr.h"
#include "meta_ham/Leaderboards.h"
#include "meta_ham/MetaPanel.h"
#include "meta_ham/MetagameRank.h"
#include "meta_ham/SaveLoadManager.h"
#include "midi/MidiParser.h"
#include "movie/Movie.h"
#include "movie/Splash.h"
#include "net/DingoSvr.h"
#include "net_ham/RockCentral.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/DirLoader.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "os/Archive.h"
#include "os/Debug.h"
#include "os/File.h"
#include "os/FileCache.h"
#include "os/PlatformMgr.h"
#include "os/System.h"
#include "os/Timer.h"
#include "rndobj/HiResScreen.h"
#include "rndobj/Rnd.h"
#include "stl/_algobase.h"
#include "synth/Synth.h"
#include "synth/SynthSample.h"
#include "ui/PanelDir.h"
#include "ui/UI.h"
#include "utl/Cheats.h"
#include "utl/Loader.h"
#include "utl/Magnu.h"
#include "utl/MemTracker.h"
#include "utl/Option.h"
#include "utl/Symbol.h"
#include "world/World.h"
#include "xdk/nui/nuiskeleton.h"
#include "xdk/win_types.h"
#include "xdk/xapilibi/handleapi.h"
#include "xdk/xapilibi/processthreadsapi.h"
#include "xdk/xapilibi/synchapi.h"
#include "xdk/xapilibi/xbox.h"
#include <cctype>
#include <cstring>

App *gApp;
ModalCallbackFunc *gRealCallback;
namespace {
    FileCache *gPersistentCache;
    bool gListenForKinectGuide;
}

bool EndsWith(const char *c1, const char *c2) {
    int len1 = strlen(c1);
    int len2 = strlen(c2);
    return strstr(c1, c2) == c1 + len1 - len2;
}

void DebugModal(Debug::ModalType &ty, FixedString &str, bool b3) {
    if (ty == Debug::kModalFail) {
        gRealCallback(ty, str, b3);
    } else {
        if (ty != Debug::kModalWarn) {
            static DataNode &n = DataVariable("notify_level");
            switch (n.Int()) {
            case 2: {
                gRealCallback(ty, str, b3);
                return;
            }
            case 1: {
                Hmx::Object *cheatDisplay =
                    ObjectDir::Main()->Find<Hmx::Object>("cheat_display", false);
                if (cheatDisplay) {
                    static Message show("show", 0);
                    show[0] = str.c_str();
                    cheatDisplay->Handle(show, false);
                }
                return;
            }
            }
        }
        MILO_LOG("%s\n", str.c_str());
    }
}

Symbol RemoveDigitSuffix(const Symbol &s1) {
    char buffer[0x40];
    buffer[0] = '\0';
    memset(&buffer[1], 0, 0x40 - 1);
    const char *strBegin = s1.Str();
    int len = strlen(strBegin);
    MILO_ASSERT(len > 0, 0x2AB);
    const char *res = std::find_if(strBegin, strBegin + len, isdigit);
    if (res - strBegin != 0) {
        memmove(buffer, strBegin, res - strBegin);
    }
    return buffer;
}

bool IsUselessLoad(const char *file) {
    bool useless = false;
    if (!gMiloTool && file && TheGameData) {
        HamPlayerData *p0 = TheGameData->Player(0);
        HamPlayerData *p1 = TheGameData->Player(1);
        if (p0 && p1) {
            bool b13 = strstr(file, "sfx/loc/") == file && strstr(file, "/vo_bank_");
            Symbol fileBase = FileGetBase(file);
            bool b11 = strstr(file, "world/shared/camshots/") == file
                && GetCharacterEntry(fileBase, false);
            bool isCrewStr = strstr(file, "world/shared/camshots/crew_") == file;
            static Symbol dance_battle("dance_battle");
            if ((b13 || b11) && !strstr(file, p0->Char().Str())
                && !strstr(file, p1->Char().Str())) {
                if (g_LoaderModeCallback(dance_battle)) {
                    Symbol crew0 = GetCrewForCharacter(p0->Char());
                    Symbol crew1 = GetCrewForCharacter(p1->Char());
                    bool nostr = strstr(fileBase.Str(), GetCrewCharacter(crew0, 0).Str())
                        || strstr(fileBase.Str(), GetCrewCharacter(crew0, 1).Str())
                        || strstr(fileBase.Str(), GetCrewCharacter(crew1, 0).Str())
                        || strstr(fileBase.Str(), GetCrewCharacter(crew1, 1).Str());
                    if (!nostr) {
                        useless = true;
                    }
                } else {
                    useless = true;
                }
            }
            if (!g_LoaderModeCallback(dance_battle)
                && (isCrewStr || EndsWith(file, "/vo_bank.milo"))) {
                useless = true;
            }
            static Symbol practice("practice");
            static Symbol campaign_practice("campaign_practice");
            if (!g_LoaderModeCallback(practice)
                && !g_LoaderModeCallback(campaign_practice)
                && EndsWith(file, "/barks.milo")) {
                useless = true;
            }
            static Symbol is_in_campaign_mode("is_in_campaign_mode");
            static Symbol is_in_campaign_stinger("is_in_campaign_stinger");
            bool b14 =
                TheHamProvider && TheHamProvider->Property(is_in_campaign_mode)->Int();
            bool b12 =
                TheHamProvider && TheHamProvider->Property(is_in_campaign_stinger)->Int();
            if (!b14 && !b12 && strstr(file, "/campaign/camp_scene_")) {
                useless = true;
            }
            if (strstr(file, "/vo_bank_camp_")) {
                useless = !b14;
            }
            static Symbol just_intro("just_intro");
            static Symbol mind_control("mind_control");
            bool b12_2 =
                g_LoaderModeCallback(mind_control) || g_LoaderModeCallback(just_intro);
            if (TheHamWardrobe && (b12_2 || g_LoaderModeCallback(dance_battle))) {
                if (b13 || b11) {
                    Symbol bc;
                    Symbol b8;
                    if (b12_2) {
                        bc = TheHamWardrobe->GetBackupOutfitOverride(0);
                        b8 = TheHamWardrobe->GetBackupOutfitOverride(1);
                    } else if (g_LoaderModeCallback(dance_battle)) {
                        bc = GetAlternateCharacter(p0->Char());
                        b8 = GetAlternateCharacter(p1->Char());
                    }
                    if (!bc.Null() && !b8.Null()) {
                        Symbol remove0 = RemoveDigitSuffix(bc);
                        Symbol remove1 = RemoveDigitSuffix(b8);
                        if (strstr(file, remove0.Str()) || strstr(file, remove1.Str())) {
                            useless = false;
                        }
                    }
                }
            }
            useless = !EndsWith(file, "/vo_bank.milo") ? useless : false;
            static Symbol bustamove("bustamove");
            if (g_LoaderModeCallback(bustamove)) {
                useless = !EndsWith(file, "/vo_bank_bustamove.milo") ? useless : false;
            }
            static Symbol challenge("challenge");
            if (g_LoaderModeCallback(challenge)) {
                useless = !EndsWith(file, "/vo_bank_challenge.milo") ? useless : false;
            }
            static Symbol strike_a_pose("strike_a_pose");
            if (g_LoaderModeCallback(strike_a_pose)) {
                useless = !EndsWith(file, "/vo_bank_strikeapose.milo") ? useless : false;
            }
            static Symbol rhythm_battle("rhythm_battle");
            static Symbol gameplay_mode("gameplay_mode");
            static Symbol current_campaign_era("current_campaign_era");
            static Symbol era_tan_battle("era_tan_battle");
            bool u15 = TheHamProvider->Property(gameplay_mode)->Sym() == rhythm_battle;
            if (b14) {
                if (TheHamProvider->Property(current_campaign_era)->Sym()
                    == era_tan_battle) {
                    u15 = true;
                }
            }
            if (u15) {
                useless = !EndsWith(file, "/vo_bank_rhythmbattle.milo") ? useless : false;
            }
            if (u15 && EndsWith(file, "/vo_bank_rhythmbattle_finale.milo")) {
                useless = !b14 ? useless : false;
            }
            useless = !strstr(fileBase.Str(), "vo_bank_tutorial_") ? useless : false;
            if (g_LoaderModeCallback(practice)
                || g_LoaderModeCallback(campaign_practice)) {
                useless = !EndsWith(file, "/vo_bank_rehearse.milo") ? useless : false;
            }
        }
    }
    if (useless) {
        MILO_LOG("'%s' is a useless load\n", file ? file : "NULL");
    }
    return useless;
}

bool XShowNuiCallback(DWORD &id) {
    bool ret;

    MILO_ASSERT(TheGestureMgr, 0x87);

    Skeleton *skel = TheGestureMgr->GetActiveSkeleton();

    if (!HamNavList::sLastSelectInControllerMode && skel && skel->IsTracked()) {
        ret = true;
        id = skel->TrackingID();
    } else {
        ret = false;
    }

    return ret;
}

DWORD KinectGuideThread(LPVOID) {
    MILO_ASSERT_FMT(
        SUCCEEDED(NuiSkeletonTrackingDisable()), "NuiSkeletonTrackingDisable failed"
    );
    MILO_ASSERT_FMT(
        SUCCEEDED(NuiSkeletonTrackingEnable(nullptr, 0)),
        "NuiSkeletonTrackingEnable failed"
    );
    HANDLE kinect_listener = XNotifyCreateListener(1);
    MILO_ASSERT(kinect_listener, 0xA2);
    while (gListenForKinectGuide) {
        DWORD dwId;
        ULONG param;
        while (XNotifyGetNext(kinect_listener, 0, &dwId, &param)) {
            if (dwId == 0x6001A) {
                XShowNuiGuideUI(param);
            }
        }
    }
    CloseHandle(kinect_listener);
    MILO_ASSERT_FMT(
        SUCCEEDED(NuiSkeletonTrackingDisable()), "NuiSkeletonTrackingDisable failed"
    );
    MILO_ASSERT_FMT(
        SUCCEEDED(NuiSkeletonTrackingEnable(SkeletonUpdate::NewSkeletonEvent(), 2)),
        "NuiSkeletonTrackingEnable failed"
    );
    return 0;
}

static const int arkNums[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

App::App(int argc, char **argv) {
    Timer init_time;
    init_time.Start();
    EnableKeyCheats(false);
    SetFileChecksumData();
    SystemPreInit(argc, argv, "config/ham_preinit_keep.dta");
    if (TheArchive) {
        TheArchive->SetArchivePermission(0xB, arkNums);
    }
    TheRnd.PreInit();
    static DataNode &notify_level = DataVariable("notify_level");
    if (UsingCD()) {
        notify_level = 1;
    } else {
        notify_level = 1;
    }
    gRealCallback = TheDebug.SetModalCallback(DebugModal);
    SynthPreInit();
    Movie::Init();
    TheRnd.SetClearColor(Hmx::Color(0, 0, 0));
    Splash splash;
    bool fast = OptionBool("fast", false);
    if (fast || !UsingCD()) {
        splash.SetWaitForSplash(false);
    }
    if (fast) {
        SynthSample::Disable();
    }
    PlatformRegion region = ThePlatformMgr.GetRegion();
    if (ULSystemLocale() == XC_LOCALE_JAPAN) {
        splash.AddScreen("ui/splash/jpn/esrb_keep.milo", 4800);
    } else if (region == kRegionNA) {
        splash.AddScreen("ui/splash/eng/esrb_keep.milo", 4800);
    }
    splash.AddScreen("ui/splash/harmonix_keep.milo", 3000);
    splash.PrepareNext();
    splash.BeginSplasher();
    float ms = init_time.SplitMs();
    LiveCameraInput::PreInit();
    LiveCameraInput::Init();
    gListenForKinectGuide = true;
    HANDLE hThread = CreateThread(nullptr, 0, KinectGuideThread, nullptr, 4, nullptr);
    XSetThreadProcessor(hThread, 1);
    ResumeThread(hThread);
    splash.PrepareRemaining();
    SystemInit("config/ham_keep.dta");
    MagnuInit();
    if (TheSplasher) {
        TheSplasher->Poll();
    }
    splash.Suspend();
    TheRnd.Init();
    TheServer.Init();
    TheRockCentral.Init();
    splash.Resume();
    if (TheSplasher) {
        TheSplasher->Poll();
    }
    MILO_LOG("HMX Red Build!\n");
    FixedSizeSaveable::Init(0x5c, 0x1662);
    HamUserMgrInit(false);
    if (TheSplasher) {
        TheSplasher->Poll();
    }
    SynthInit();
    if (TheSplasher) {
        TheSplasher->Poll();
    }
    FlowInit();
    if (TheSplasher) {
        TheSplasher->Poll();
    }
    {
        ObjDirPtr<ObjectDir> dPtr;
        dPtr.LoadFile("sfx/audio_mixer.milo", false, true, kLoadFront, false);
        ObjDirPtr<ObjectDir> dPtr2;
        dPtr2.LoadFile(
            SystemConfig("sound", "banks", "common")->Str(1),
            false,
            true,
            kLoadFront,
            false
        );
        TheSynth->SetDir(dPtr2);
        if (TheSplasher) {
            TheSplasher->Poll();
        }
    }
    SaveLoadManager::Init();
    if (TheSplasher) {
        TheSplasher->Poll();
    }
    CharInit();
    if (TheSplasher) {
        TheSplasher->Poll();
    }
    MidiParser::Init();
    if (TheSplasher) {
        TheSplasher->Poll();
    }
    WorldInit();
    if (TheSplasher) {
        TheSplasher->Poll();
    }
    HamInit();
    if (TheSplasher) {
        TheSplasher->Poll();
    }
    TheHamSongMgr.Init();
    if (TheSplasher) {
        TheSplasher->Poll();
    }
    MetaPanel::Init();
    if (TheSplasher) {
        TheSplasher->Poll();
    }
    GameInit();
    DirLoader::SetPathEvalFunc(IsUselessLoad);
    if (TheSplasher) {
        TheSplasher->Poll();
    }
    ContextCheckerInit();
    if (TheSplasher) {
        TheSplasher->Poll();
    }
    PlatformMgr::sXShowCallback = XShowNuiCallback;
    if (TheSplasher) {
        TheSplasher->Poll();
    }
    AccomplishmentManager::Init(SystemConfig("accomplishment_info"));
    if (TheSplasher) {
        TheSplasher->Poll();
    }
    MetagameRank::Init();
    DataArray *cacheCfg = SystemConfig("persistent_filecache");
    if (cacheCfg) {
        gPersistentCache = new FileCache(cacheCfg->Int(1), kLoadFront, false, true);
        gPersistentCache->StartSet(0);
        for (int i = 2; i < cacheCfg->Size(); i++) {
            gPersistentCache->Add(cacheCfg->Str(i), 1, "");
        }
        gPersistentCache->EndSet();
        gPersistentCache->PollUntilLoaded();
    }
    static DataNode &extra_songs = DataVariable("extra_songs");
    if (UsingCD()) {
        extra_songs = 0;
    } else {
        extra_songs = 1;
    }
    TheUI->Init();
    if (TheSplasher) {
        TheSplasher->Poll();
    }
    GestureMgr::DebugInit();
    ThePresenceMgr.Init();
    if (TheSplasher) {
        TheSplasher->Poll();
    }
    MoveMgr::Init(nullptr);
    MiniGameMgr::Init();
    if (TheSplasher) {
        TheSplasher->Poll();
    }
    PartyModeMgr::Init();
    TheUI->GotoFirstScreen();
    float f15 = init_time.SplitMs();
    if (TheArchive && TheArchive->DebugArkOrder()) {
        MILO_LOG("Startup Time: %f %f\n", ms, f15 - ms);
    }
    splash.EndSplasher();
    EnableKeyCheats(true);
    AutoGlitchReport::EnableCallback();
    ThePlatformMgr.SetBackgroundDownloadPriority(true);
    gListenForKinectGuide = false;
    WaitForSingleObject(hThread, -1);
    CloseHandle(hThread);
    MemTrackEnable(true);
}

void App::Run() { RunWithoutDebugging(); }

void App::CaptureHiRes() {
    bool paused = AllPaused();

    if (paused)
        TheGame->SetTimePaused(true);

    DrawRegular();

    int tiles = TheHiResScreen.GetTiling() * TheHiResScreen.GetTiling();

    for (int i = 0; i <= tiles; i++) {
        DrawRegular();
        TheHiResScreen.Accumulate();
    }

    TheHiResScreen.Finish();

    if (paused)
        TheGame->SetTimePaused(false);
}

void App::DrawRegular() {
    TheRnd.BeginDrawing();
    TheUI->Draw();
    TheRnd.EndDrawing();
}

App::~App() { TheDebug.Exit(0, true); }

void App::RunWithoutDebugging() {
    while (true) {
        float glitchTime;
        do {
            Timer timer;
            timer.Restart();
            SystemPoll(false);
            {
                START_AUTO_TIMER("misc_poll");
                TheAchievements->Poll();
                TheAccomplishmentMgr->Poll();
                if (TheLeaderboards) {
                    TheLeaderboards->Poll();
                }
                if (TheChallenges) {
                    TheChallenges->Poll();
                }
                TheSaveLoadMgr->Poll();
            }
            {
                START_AUTO_TIMER("synth_poll");
                TheSynth->Poll();
            }
            {
                START_AUTO_TIMER("rock_central_poll");
                TheRockCentral.Poll();
            }
            {
                START_AUTO_TIMER("gesture_poll");
                TheGestureMgr->Poll();
            }
            TheUI->Poll();
            DataNode &hud_panel = DataVariable("hud_panel");
            if (hud_panel.CompatibleType(kDataObject)) {
                PanelDir *dir = hud_panel.Obj<PanelDir>();
                if (dir) {
                    dir->Handle(Message("update_all_flashcard_dance_pct"), true);
                }
            }
            TheTaskMgr.Poll();
            TheFlowMgr->Poll();
            {
                START_AUTO_TIMER("skeleton_post_update");
                SkeletonUpdateHandle h = SkeletonUpdate::InstanceHandle();
                h.PostUpdate();
            }
            FileDiscSpinUp();
            if (TheHiResScreen.IsActive()) {
                CaptureHiRes();
            } else {
                DrawRegular();
            }
            float timerMs = timer.SplitMs();
            glitchTime = timerMs
                - Min(Timer::SlowFrameTimer().SplitMs(), Timer::SlowFrameWaiver());
        } while (glitchTime <= 83.333298f);
        const char *glitchStr = nullptr;
        const char *currentScreenName =
            TheUI->CurrentScreen() ? TheUI->CurrentScreen()->Name() : "none";
        const char *transitionScreenName =
            TheUI->TransitionScreen() ? TheUI->TransitionScreen()->Name() : "none";

        switch (TheUI->GetTransitionState()) {
        case UIManager::kTransitionNone: {
            glitchStr =
                MakeString("GLITCH: %g ms, ACTIVE %s", glitchTime, currentScreenName);
            break;
        }
        case UIManager::kTransitionTo: {
            glitchStr = MakeString(
                "GLITCH: %g ms, %s TRANS TO %s",
                glitchTime,
                currentScreenName,
                transitionScreenName
            );
            break;
        }
        case UIManager::kTransitionFrom: {
            glitchStr = MakeString(
                "GLITCH: %g ms, %s TRANS FROM %s",
                glitchTime,
                currentScreenName,
                transitionScreenName
            );
            break;
        }
        case UIManager::kTransitionPop: {
            glitchStr =
                MakeString("GLITCH: %g ms, POPPING %s", glitchTime, transitionScreenName);
            break;
        }
        }
        static DataNode &notify_level = DataVariable("notify_level");
        if (notify_level.Int()) {
            static Hmx::Object *cheatDisplay =
                ObjectDir::Main()->Find<Hmx::Object>("cheat_display");
            static Message show("show", 0);
            show[0] = glitchStr;
            cheatDisplay->Handle(show, false);
        }
    }
}
