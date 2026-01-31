#include "meta_ham/ContextChecker.h"
#include "game/Game.h"
#include "game/GameMode.h"
#include "hamobj/Difficulty.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamPlayerData.h"
#include "math/Rand.h"
#include "meta_ham/CampaignPerformer.h"
#include "meta_ham/ProfileMgr.h"
#include "obj/Data.h"
#include "obj/DataFunc.h"
#include "os/Debug.h"
#include "utl/Symbol.h"
#include "utl/Std.h"
#include <vector>
#include <set>

namespace {
    std::set<Symbol> gUsedContexts;

    Rand gContextRand(0);
    int gContextWeight = 10;

    bool IsContextUsed(Symbol ctx) {
        return gUsedContexts.find(ctx) != gUsedContexts.end();
    }

    bool InternalCheckContext(const DataArray *);
    bool CheckContextAnd(const DataArray *a) {
        for (int i = 1; i < a->Size(); i++) {
            if (!InternalCheckContext(a->Array(i))) {
                return false;
            }
        }
        return true;
    }

    bool CheckContextOr(const DataArray *a) {
        for (int i = 1; i < a->Size(); i++) {
            if (InternalCheckContext(a->Array(i))) {
                return true;
            }
        }
        return false;
    }

    bool CheckContextNot(const DataArray *arr) {
        MILO_ASSERT(arr->Size() == 2, 0x4A);
        return !InternalCheckContext(arr->Array(1));
    }

    bool CheckContextModeProperty(const DataArray *arr) {
        MILO_ASSERT(arr->Size() == 3, 0x5B);
        DataNode &other = arr->Node(2);
        return TheGameMode->Property(arr->Sym(1))->Equal(other, nullptr, true);
    }

    bool CheckContextMode(const DataArray *a) {
        for (int i = 1; i < a->Size(); i++) {
            if (TheGameMode->InMode(a->Sym(i), true)) {
                return true;
            }
        }
        return false;
    }

    bool CheckContextDiff(const DataArray *a) {
        Difficulty d = TheGameData->Player(0)->GetDifficulty();
        for (int i = 1; i < a->Size(); i++) {
            if (SymToDifficulty(a->Sym(i)) == d) {
                return true;
            }
        }
        return false;
    }

    Symbol GetSong() {
        MetaPerformer *performer = MetaPerformer::Current();
        MILO_ASSERT(performer, 0x29);
        Symbol out;
        if (performer->IsSetComplete()) {
            out = performer->GetCompletedSong();
        } else {
            out = TheGameData->GetSong();
        }
        return out;
    }

    bool CheckContextSong(const DataArray *a) {
        Symbol song = GetSong();
        for (int i = 1; i < a->Size(); i++) {
            if (a->Sym(i) == song) {
                return true;
            }
        }
        return false;
    }

    bool CheckContextLastSong(const DataArray *a) {
        Symbol song = MetaPerformer::Current()->GetCompletedSong();
        for (int i = 1; i < a->Size(); i++) {
            if (a->Sym(i) == song) {
                return true;
            }
        }
        return false;
    }

    bool CheckContextVenue(const DataArray *a) {
        Symbol venue = TheGameData->Venue();
        for (int i = 1; i < a->Size(); i++) {
            if (a->Sym(i) == venue) {
                return true;
            }
        }
        return false;
    }

    bool CheckContextNumRestarts(const DataArray *arr) {
        if (TheGame) {
            int numRestarts = TheGame->GetNumRestarts();
            MILO_ASSERT(arr->Size() >= 2, 0x98);
            return numRestarts - arr->Int(1) == 0;
        }
    }

    bool CheckContextNumRestartsGreater(const DataArray *arr) {
        if (TheGame) {
            int numRestarts = TheGame->GetNumRestarts();
            MILO_ASSERT(arr->Size() >= 2, 0xa9);
            return arr->Int(1) < numRestarts;
        }
    }

    bool CheckContextNumRestartsNot(const DataArray *arr) {
        if (TheGame) {
            int numRestarts = TheGame->GetNumRestarts();
            MILO_ASSERT(arr->Size() >= 2, 0xba);
            return numRestarts != arr->Int(1);
        }
    }

    bool CheckContextNumPlayers(const DataArray *arr) {
        int numPlaying = 0;
        HamPlayerData *player1 = TheGameData->Player(0);
        MILO_ASSERT(player1, 0xCB);
        HamPlayerData *player2 = TheGameData->Player(1);
        MILO_ASSERT(player2, 0xCD);
        numPlaying += (int)player1->IsPlaying();
        if (player2->IsPlaying()) {
            numPlaying++;
        }
        MILO_ASSERT(arr->Size() >= 2, 0xD8);
        return numPlaying == arr->Int(1);
    }

    bool CheckContextEra(const DataArray *arr) {
        if (!TheGameMode->InMode("campaign", true)) {
            return false;
        } else {
            CampaignPerformer *pPerformer =
                dynamic_cast<CampaignPerformer *>(MetaPerformer::Current());
            MILO_ASSERT(pPerformer, 0xE8);
            Symbol era = pPerformer->Era();
            MILO_ASSERT(arr->Size() >= 2, 0xEC);
            Symbol s = arr->Sym(1);
            return s == era;
        }
    }

    bool CheckContextVoiceCommanderEnabled(const DataArray *arr) {
        MILO_ASSERT(arr->Size() >= 2, 0xF7);
        bool myBool = arr->Int(1);
        bool enabled = !TheProfileMgr.GetDisableVoiceCommander();
        return myBool == enabled;
    }

    bool CheckContextVoicePauseEnabled(const DataArray *arr) {
        MILO_ASSERT(arr->Size() >= 2, 0x103);
        bool myBool = arr->Int(1);
        bool enabled = !TheProfileMgr.GetDisableVoicePause();
        return myBool == enabled;
    }

    bool CheckContextVoicePracticeEnabled(const DataArray *arr) {
        MILO_ASSERT(arr->Size() >= 2, 0x103);
        bool myBool = arr->Int(1);
        bool enabled = !TheProfileMgr.GetDisableVoicePractice();
        return myBool == enabled;
    }

    bool InternalCheckContext(const DataArray *a) {
        static Symbol and ("and");
        static Symbol or ("or");
        static Symbol not("not");
        static Symbol mode_property("mode_property");
        static Symbol mode("mode");
        static Symbol diff("diff");
        static Symbol song("song");
        static Symbol last_song("last_song");
        static Symbol venue("venue");
        static Symbol unlocked("unlocked");
        static Symbol weight("weight");
        static Symbol song_specific("song_specific");
        static Symbol num_restarts("num_restarts");
        static Symbol num_restarts_greater("num_restarts_greater");
        static Symbol num_restarts_not("num_restarts_not");
        static Symbol num_players("num_players");
        static Symbol era("era");
        static Symbol voice_commander_enabled("voice_commander_enabled");
        static Symbol voice_pause_enabled("voice_pause_enabled");
        static Symbol voice_practice_enabled("voice_practice_enabled");
        Symbol ctxSym = a->Sym(0);
        if (ctxSym == and) {
            return CheckContextAnd(a);
        } else if (ctxSym == or) {
            return CheckContextOr(a);
        } else if (ctxSym == not) {
            return CheckContextNot(a);
        } else if (ctxSym == mode_property) {
            return CheckContextModeProperty(a);
        } else if (ctxSym == mode) {
            return CheckContextMode(a);
        } else if (ctxSym == diff) {
            return CheckContextDiff(a);
        } else if (ctxSym == song) {
            return CheckContextSong(a);
        } else if (ctxSym == last_song) {
            return CheckContextLastSong(a);
        } else if (ctxSym == venue) {
            return CheckContextVenue(a);
        } else if (ctxSym == num_restarts) {
            return CheckContextNumRestarts(a);
        } else if (ctxSym == num_restarts_greater) {
            return CheckContextNumRestartsGreater(a);
        } else if (ctxSym == num_restarts_not) {
            return CheckContextNumRestartsNot(a);
        } else if (ctxSym == unlocked) {
            return false;
        } else if (ctxSym == num_players) {
            return CheckContextNumPlayers(a);
        } else if (ctxSym == era) {
            return CheckContextEra(a);
        } else if (ctxSym == voice_commander_enabled) {
            return CheckContextVoiceCommanderEnabled(a);
        } else if (ctxSym == voice_pause_enabled) {
            return CheckContextVoicePauseEnabled(a);
        } else if (ctxSym == voice_practice_enabled) {
            return CheckContextVoicePracticeEnabled(a);
        } else if (ctxSym == weight) {
            gContextWeight = a->Int(1);
            return true;
        } else if (ctxSym == song_specific) {
            return true;
        } else {
            MILO_NOTIFY(
                "Data array at file %s line %d was not valid context-checker",
                a->File(),
                a->Line()
            );
            return false;
        }
    }

}

struct WeightedEntry {
    int unk0;
    Symbol unk4;
};

void HandleContextUsed(Symbol ctx) { gUsedContexts.insert(ctx); }

__declspec(noinline) int CheckContext(const DataArray *a) {
    gContextWeight = 10;
    return gContextWeight & CheckContextAnd(a);
}

bool IsSongSpecificEntry(const DataArray *a) {
    for (int i = 1; i < a->Size(); i++) {
        static Symbol song_specific("song_specific");
        Symbol sym = a->Array(i)->Sym(0);
        if (sym == song_specific)
            return true;
    }
    return false;
}

std::list<Symbol> GetSongSpecificEntriesForCategory(Symbol s, bool b) {
    std::list<Symbol> entries;
    if (b) {
        s = MetaPerformer::Current()->GetCompletedSong();
    }
    return entries;
}

std::list<Symbol> GetSongSpecificEntries(const DataArray *a) {
    for (int i = 1; i < a->Size(); i++) {
        static Symbol song_specific("song_specific");
        static Symbol last_song("last_song");
        static Symbol current_song("current_song");
        DataArray *thisEntry = a->Array(i);
        Symbol s0 = thisEntry->Sym(0);
        if (s0 == song_specific) {
            MILO_ASSERT(thisEntry->Size() >= 3, 0x1BB);
            Symbol s1 = thisEntry->Sym(1);
            Symbol s2 = thisEntry->Sym(2);
            return GetSongSpecificEntriesForCategory(s1, s2 == last_song);
        }
    }
    std::list<Symbol> empty;
    return empty;
}

void PotentiallyCreateAndAddEntry(
    Symbol s, int x, bool b, std::vector<WeightedEntry> &entries, int &iref
) {
    if (b && IsContextUsed(s)) {
        return;
    }
    WeightedEntry entry;
    iref += x;
    entry.unk0 = iref;
    entry.unk4 = s;
    entries.push_back(entry);
}

int InqMatchedEntries(const DataArray *a, bool b, std::vector<WeightedEntry> &entries) {
    int i70 = 0;
    for (int i = 1; i < a->Size(); i++) {
        DataArray *cur = a->Array(i);
        int context = CheckContext(cur);
        if (context > 0) {
            if (IsSongSpecificEntry(cur)) {
                std::list<Symbol> songSpecificEntries = GetSongSpecificEntries(cur);
                if (!songSpecificEntries.empty()) {
                    float f5 = (float)context / (float)songSpecificEntries.size();
                    if (f5 < 1.0f) {
                        f5 = 1.0f;
                    }
                    FOREACH (it, songSpecificEntries) {
                        PotentiallyCreateAndAddEntry(*it, (int)f5, b, entries, i70);
                    }
                }
            } else {
                Symbol sym = cur->Sym(0);
                PotentiallyCreateAndAddEntry(sym, context, b, entries, i70);
            }
        }
    }
    return i70;
}

Symbol RandomContextSensitiveItemInternal(const DataArray *a, bool b2, bool fail = true) {
    std::vector<WeightedEntry> entries;
    int inq = InqMatchedEntries(a, b2, entries);
    if (inq <= 0) {
        if (!b2 && fail) {
            MILO_FAIL("No valid context-sensitive item");
        }
        return "";
    } else {
        int randInt = gContextRand.Int(0, inq);
        for (int i = 0; i < entries.size(); i++) {
            if (randInt < entries[i].unk0) {
                Symbol cur = entries[i].unk4;
                HandleContextUsed(cur);
                return cur;
            }
        }
        if (fail) {
            MILO_FAIL("No valid context-sensitive item");
        }
        return "";
    }
}

Symbol RandomContextSensitiveItem(const DataArray *a1, bool fail) {
    Symbol s = RandomContextSensitiveItemInternal(a1, true, fail);
    if (s == "") {
        s = RandomContextSensitiveItemInternal(a1, false, fail);
    }
    if (fail && s == "") {
        MILO_FAIL("No valid context-sensitive item");
    }
    return s;
}

DataNode OnRandomContext(DataArray *a) { return RandomContextSensitiveItem(a->Array(1)); }

DataNode OnRandomContextAllowFailure(DataArray *a) {
    return RandomContextSensitiveItem(a->Array(1), false);
}

DataNode OnSeedRandomContext(DataArray *a) {
    gContextRand.Seed(a->Int(1));
    return 1;
}

DataNode OnHandleContextUsed(DataArray *a) {
    HandleContextUsed(a->Sym(1));
    return 1;
}

DataNode OnRandomContextCount(DataArray *a) { return a->Array(1)->Size() - 1; }

void ContextCheckerInit() {
    DataRegisterFunc("random_context", OnRandomContext);
    DataRegisterFunc("random_context_allow_failure", OnRandomContextAllowFailure);
    DataRegisterFunc("seed_random_context", OnSeedRandomContext);
    DataRegisterFunc("handle_context_used", OnHandleContextUsed);
    DataRegisterFunc("random_context_count", OnRandomContextCount);
    gContextRand.Seed(RandomInt());
}
