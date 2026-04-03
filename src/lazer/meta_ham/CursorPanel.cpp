#include "meta_ham/CursorPanel.h"
#include "flow/PropertyEventProvider.h"
#include "gesture/BaseSkeleton.h"
#include "gesture/GestureMgr.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamPlayerData.h"
#include "math/Mtx.h"
#include "meta_ham/PassiveMessagesPanel.h"
#include "net_ham/RockCentral.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rndobj/Mat.h"
#include "rndobj/Tex.h"
#include "utl/Symbol.h"
#include <cmath>
#include <cstring>

static int sInt = -1;

CursorPanel::CursorPanel() {}

CursorPanel::~CursorPanel() {}

void CursorPanel::Poll() {
    PassiveMessagesPanel::Poll();
    static Symbol ui_crown_player("ui_crown_player");
    const DataNode *pCrownPlayerNode = TheHamProvider->Property(ui_crown_player, true);
    MILO_ASSERT(pCrownPlayerNode, 0x1f);
    for (int i = 0; i < 2; i++) {
        SkeletonSide side = TheGameData->Player(i)->Side();
        static Symbol player_present("player_present");
        bool present =
            TheGameData->Player(i)->Provider()->Property(player_present)->Int();
        RndTex *pBufferLeftTex = Dir()->Find<RndTex>("depth_buffer_left_crown.tex");
        RndMat *pMat = Dir()->Find<RndMat>(
            side == kSkeletonLeft ? "depth_buffer_left_crown.mat"
                                  : "depth_buffer_right_crown.mat"
        );
        if (TheRockCentral.GetMiscArt()) {
            TheRockCentral.SetMiscArt(pBufferLeftTex);
        }
        pMat->SetDiffuseTex(pBufferLeftTex);

        Transform trans;
        memcpy(&trans, &(pMat->TexXfm()), 0x40);

        int skeletonID = TheGestureMgr->GetPlayerSkeletonID(i);
        bool check = true;
        if (!present || side != pCrownPlayerNode->Int() || skeletonID < 0) {
            check = false;
        } else {
            Skeleton *skel = TheGestureMgr->GetSkeletonByTrackingID(skeletonID);
            if (!skel) {
                check = false;
            }
        }
    }
}

BEGIN_HANDLERS(CursorPanel)
    HANDLE_SUPERCLASS(PassiveMessagesPanel)
END_HANDLERS
