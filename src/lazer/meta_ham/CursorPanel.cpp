#include "meta_ham/CursorPanel.h"
#include "flow/PropertyEventProvider.h"
#include "gesture/BaseSkeleton.h"
#include "gesture/GestureMgr.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamPlayerData.h"
#include "math/Mtx.h"
#include "math/Rot.h"
#include "meta_ham/PassiveMessagesPanel.h"
#include "net_ham/RockCentral.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rndobj/Mat.h"
#include "rndobj/Tex.h"
#include "ui/PanelDir.h"
#include "utl/Symbol.h"
#include <cmath>
#include <cstring>

static int sInt = -1;

CursorPanel::CursorPanel() {}

CursorPanel::~CursorPanel() {}

void CursorPanel::Poll() {
    PassiveMessagesPanel::Poll();
    static Symbol ui_crown_player("ui_crown_player");
    const DataNode *pCrownPlayerNode = TheHamProvider->Property(ui_crown_player);
    MILO_ASSERT(pCrownPlayerNode, 0x1f);
    int crownPlayer = pCrownPlayerNode->Int();
    for (int i = 0; i < 2; i++) {
        SkeletonSide side = TheGameData->Player(i)->Side();
        static Symbol player_present("player_present");
        bool present =
            TheGameData->Player(i)->Provider()->Property(player_present)->Int();
        RndTex *pBufferLeftTex = LoadedDir()->Find<RndTex>("depth_buffer_left_crown.tex");
        RndMat *pMat = nullptr;
        if (side == kSkeletonLeft) {
            pMat = LoadedDir()->Find<RndMat>("depth_buffer_left_crown.mat");
        } else {
            pMat = LoadedDir()->Find<RndMat>("depth_buffer_right_crown.mat");
        }
        RndTex *tex = TheRockCentral.GetUtilityArt();
        if (!tex) {
            tex = pBufferLeftTex;
        }
        pMat->SetDiffuseTex(tex);

        Transform trans = pMat->TexXfm();
        int skeletonID = TheGestureMgr->GetPlayerSkeletonID(i);
        Skeleton *skeleton;
        bool check = present && side == crownPlayer && skeletonID >= 0
            && (skeleton = TheGestureMgr->GetSkeletonByTrackingID(skeletonID), skeleton);
        static int sCrownPlayerIndex = -1;
        if (check && sCrownPlayerIndex == -1) {
            sCrownPlayerIndex = i;
        }
        if ((i == sCrownPlayerIndex && !check) || check) {
            MILO_LOG("player %d lost his crown\n", sCrownPlayerIndex);
            sCrownPlayerIndex = -1;
            TheHamProvider->SetProperty(ui_crown_player, -1);
            check = false;
        }
        if (check) {
            Vector2 v120;
            skeleton->ScreenPos(kJointHead, v120);
            Vector2 v128;
            skeleton->ScreenPos(kJointShoulderCenter, v128);
            trans.v.x = v120.x;
            trans.v.y = -v120.y;
            v128.x = v120.x - v128.x;
            v128.y = v120.y - v128.y;
            float tanned = atan2(v128.y, v128.x);
            Vector3 v110(0, 0, tanned + (PI / 2));
            MakeRotMatrix(v110, trans.m, true);
            trans.m.x *= 4;
            trans.m.y *= 4;
            trans.m.z *= 4;
            pMat->SetTexXfm(trans);
        } else {
            trans.v.x = 2;
            trans.v.y = 2;
            pMat->SetTexXfm(trans);
        }
    }
}

BEGIN_HANDLERS(CursorPanel)
    HANDLE_SUPERCLASS(PassiveMessagesPanel)
END_HANDLERS
