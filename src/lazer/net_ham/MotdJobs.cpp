#include "net_ham/MotdJobs.h"
#include "net/JsonUtils.h"
#include "net_ham/RCJobDingo.h"
#include "obj/Object.h"
#include "os/DateTime.h"
#include "os/System.h"
#include "utl/DataPointMgr.h"
#include "utl/Symbol.h"
#include <cstdlib>

GetMotdJob::GetMotdJob(Hmx::Object *callback) : RCJob("motd/getmotd/", callback) {
    DataPoint dataP;
    static Symbol locale("locale");
    dataP.AddPair(locale, SystemLanguage());
    SetDataPoint(dataP);
}

void GetMotdJob::GetMotdData(
    unsigned int &challengeInterval,
    int &lastNewSongDt,
    bool &motdXPFlag,
    int &motdFreq,
    std::vector<String> &toasts,
    String &motd,
    String &motdImage,
    String &motdSound,
    String &motdAux,
    String &motdImageAux,
    String &motdSoundAux,
    String &motdMiscImage
) {
    lastNewSongDt = -1;
    motdXPFlag = false;
    motdFreq = 0;
    toasts.clear();
    motd = gNullStr;
    motdImage = gNullStr;
    motdSound = gNullStr;
    motdAux = gNullStr;
    motdImageAux = gNullStr;
    motdSoundAux = gNullStr;
    if (mResult == 1) {
        JsonConverter &c = GetJsonConverter();
        JsonObject *o = GetJsonResponse();
        if (o) {
            JsonObject *intervalName = c.GetByName(o, "challenge_interval");
            if (intervalName) {
                challengeInterval = intervalName->Int() * 1000;
                int i = challengeInterval;
                MILO_LOG(">>>>>>>>>> [challenge_interval] = %i\n", i);
            }
            JsonObject *lastSongName = c.GetByName(o, "last_new_song");
            if (lastSongName) {
                MILO_LOG(">>>>>>>>>> [last_new_song] = %s\n", lastSongName->Str());
                if (strcmp(lastSongName->Str(), "0000-00-00")) {
                    DateTime dt;
                    dt.ParseDate(lastSongName->Str());
                    lastNewSongDt = dt.ToCode();
                }
            }
            JsonObject *motdXPName = c.GetByName(o, "motd_xp_flag");
            if (motdXPName) {
                motdXPFlag = motdXPName->Int();
                MILO_LOG(
                    ">>>>>>>>>> [motd_xp_flag] = %s\n", motdXPFlag ? "TRUE" : "FALSE"
                );
            }
            JsonObject *motdFreqName = c.GetByName(o, "motd_freq");
            if (motdFreqName) {
                motdFreq = motdFreqName->Int();
                MILO_LOG(">>>>>>>>>> [motd_freq] = %i\n", motdFreq);
            }
            JsonObject *numToastsName = c.GetByName(o, "num_toasts");
            if (numToastsName) {
                int numToasts = numToastsName->Int();
                MILO_LOG(">>>>>>>>>> [num_toasts] = %i\n", numToasts);
                for (unsigned int i = 0; i < numToasts; i++) {
                    char num[4];
                    itoa(i, num, 10);
                    String toast = "toast_";
                    toast += num;
                    JsonObject *toastName = c.GetByName(o, toast.c_str());
                    if (toastName) {
                        const char *toastStr = toastName->Str();
                        toasts.push_back(toastStr);
                        MILO_LOG(">>>>>>>>>> [%s] = %s\n", toast.c_str(), toastStr);
                    }
                }
            }
            JsonObject *motdName = c.GetByName(o, "motd");
            if (motdName) {
                motd = motdName->Str();
                MILO_LOG(">>>>>>>>>> [motd] = %s\n", motd.c_str());
            }

            motdImage = c.GetByName(o, "motd_image")->Str();
            MILO_LOG(">>>>>>>>>> [motd_image] = %s\n", motdImage.c_str());

            JsonObject *motdSoundName = c.GetByName(o, "motd_sound");
            if (motdSoundName) {
                motdSound = motdSoundName->Str();
                MILO_LOG(">>>>>>>>>> [motd_sound] = %s\n", motdSound.c_str());
            }

            JsonObject *motdAuxName = c.GetByName(o, "motd_aux");
            if (motdAuxName) {
                motdAux = motdAuxName->Str();
                MILO_LOG(">>>>>>>>>> [motd_aux] = %s\n", motdAux.c_str());
            }

            motdImageAux = c.GetByName(o, "motd_image_aux")->Str();
            MILO_LOG(">>>>>>>>>> [motd_image_aux] = %s\n", motdImageAux.c_str());

            JsonObject *motdSoundAuxName = c.GetByName(o, "motd_sound_aux");
            if (motdSoundAuxName) {
                String str(motdSoundAuxName->Str());
                str = str.substr(0, str.length() - 4);
                motdSoundAux = MakeString("motd_%s.snd", str.c_str());
                MILO_LOG(">>>>>>>>>> [motd_sound_aux] = %s\n", motdSoundAux.c_str());
            }

            motdMiscImage = c.GetByName(o, "motd_misc_image")->Str();
            MILO_LOG(">>>>>>>>>> [motd_misc_image] = %s\n", motdMiscImage.c_str());
        }
    }
}
