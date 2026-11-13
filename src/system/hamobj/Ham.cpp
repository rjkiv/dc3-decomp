#include "hamobj/Ham.h"
#include "CharFeedback.h"
#include "HamCharacter.h"
#include "HamDriver.h"
#include "HamMove.h"
#include "HamRegulate.h"
#include "HamSong.h"
#include "MoveGraph.h"
#include "PoseFatalities.h"
#include "SongCollision.h"
#include "flow/PropertyEventProvider.h"
#include "gesture/Gesture.h"
#include "gesture/SpeechMgr.h"
#include "hamobj/BustAMoveData.h"
#include "hamobj/CamShotCatVO.h"
#include "hamobj/CrazeHollaback.h"
#include "hamobj/DanceRemixer.h"
#include "hamobj/DancerSequence.h"
#include "hamobj/Difficulty.h"
#include "hamobj/HamBattleData.h"
#include "hamobj/HamCamShot.h"
#include "hamobj/HamCamTransform.h"
#include "hamobj/HamDirector.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamIKEffector.h"
#include "hamobj/HamIKSkeleton.h"
#include "hamobj/HamIconMan.h"
#include "hamobj/HamLabel.h"
#include "hamobj/HamList.h"
#include "hamobj/HamListRibbon.h"
#include "hamobj/HamNavList.h"
#include "hamobj/HamNavProvider.h"
#include "hamobj/HamPartyJumpData.h"
#include "hamobj/HamPhotoDisplay.h"
#include "hamobj/HamPhraseMeter.h"
#include "hamobj/HamProviderPrinter.h"
#include "hamobj/HamRibbon.h"
#include "hamobj/HamScrollSpeedIndicator.h"
#include "hamobj/HamSkeletonConverter.h"
#include "hamobj/HamSupereasyData.h"
#include "hamobj/HamVisDir.h"
#include "hamobj/HamWardrobe.h"
#include "hamobj/HollaBackMinigame.h"
#include "hamobj/MeterDisplay.h"
#include "hamobj/MiniLeaderboardDisplay.h"
#include "hamobj/MoveDir.h"
#include "hamobj/OriginalChoreoRemixer.h"
#include "hamobj/PhotoSpotlightPositioner.h"
#include "hamobj/PracticeOptionsProvider.h"
#include "hamobj/PracticeSection.h"
#include "hamobj/RhythmBattle.h"
#include "hamobj/RhythmBattlePlayer.h"
#include "hamobj/RhythmDetector.h"
#include "hamobj/RhythmDetectorGroup.h"
#include "hamobj/SongDifficultyDisplay.h"
#include "hamobj/SongLayout.h"
#include "hamobj/StarsDisplay.h"
#include "hamobj/SuperEasyRemixer.h"
#include "hamobj/TransConstraint.h"
#include "obj/Data.h"
#include "obj/DataUtl.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/System.h"

void HamTerminate() {
    DataArray *dataMacro = DataGetMacro("INIT_HAM");
    if (dataMacro) {
        ObjectDir::Terminate();
        GestureTerminate();
    }
}

void HamInit() {
    GestureInit();
    if (DataGetMacro("INIT_HAM")) {
        CharFeedback::Register();
        CamShotCatVOInit();
        DancerSequence::Register();
        HamBattleData::Register();
        SongLayout::Register();
        HamDriver::Register();
        REGISTER_OBJ_FACTORY(Hmx::Object);
        HamGameData::Init();
        HamIconMan::Register();
        HamIKEffector::Register();
        HamIKSkeleton::Register();
        HamLabel::Init();
        HamList::Init();
        HamWardrobe::Register();
        HamDirector::Register();
        HamCamShot::Register();
        HamCamTransform::Register();
        HamCharacter::Init();
        HamRegulate::Register();
        HamRibbon::Register();
        HamMove::Register();
        HamSkeletonConverter::Register();
        HamSong::Register();
        HamSupereasyData::Register();
        PracticeSection::Register();
        HamVisDir::Register();
        HamPhotoDisplay::Init();
        HamPhraseMeter::Register();
        MeterDisplay::Init();
        MiniLeaderboardDisplay::Init();
        SongCollision::Init();
        SongDifficultyDisplay::Init();
        StarsDisplay::Init();
        TransConstraint::Register();
        HamListRibbon::Register();
        HamScrollSpeedIndicator::Register();
        HamNavList::Init();
        HamNavProvider::Init();
        PracticeOptionsProvider::Init();
        PhotoSpotlightPositioner::Init();
        RhythmDetector::Register();
        RhythmBattle::Register();
        RhythmBattlePlayer::Register();
        RhythmDetectorGroup::Register();
        HollaBackMinigame::Register();
        MoveGraph::Register();
        CrazeHollaback::Register();
        HamPartyJumpData::Register();
        DanceRemixer::Register();
        OriginalChoreoRemixer::Register();
        SuperEasyRemixer::Register();
        BustAMoveData::Register();
        PoseFatalities::Register();
        MoveDir::Init();
        DifficultyInit();
        TheDebug.AddExitCallback(HamTerminate);
        if (SystemConfig("objects", "PropertyEventProvider", "types")
                ->FindArray("HamProvider", false)) {
            SystemConfig("ham_init")->ExecuteBlock(1);
            TheHamProvider =
                ObjectDir::Main()->Find<PropertyEventProvider>("hamprovider", false);
            static Symbol language("language");
            TheHamProvider->SetProperty(language, SystemLanguage());
            if (TheSpeechMgr) {
                static Symbol voice_available("voice_available");
                TheHamProvider->SetProperty(
                    voice_available, TheSpeechMgr->SpeechSupported()
                );
            }
            HamProviderPrinter *printer = new HamProviderPrinter(); // uhhh...
        }
        PreloadSharedSubdirs("ham");
    }
}
