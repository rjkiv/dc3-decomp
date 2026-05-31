#pragma once
#include <vector>

namespace DSP {
    namespace Synapse {
        // size 0x44
        class GranularSynth {
        public:
            // size 0x40
            struct Granule {
                bool unk0;
                float unk4;
                float unk8;
                float unkc;
                int unk10;
                int unk14;
                int unk18;
                int unk1c;
                int unk20;
                int unk24;
                int unk28;
                int unk2c;
                int unk30;
                int unk34;
                int unk38;
                int unk3c;
            };
            // size 0x18
            struct Voice {
                float unk0;
                int unk4;
                int unk8;
                bool enabled;
                double unk10;
            };

            GranularSynth(
                const std::vector<float> &, unsigned int, unsigned int, unsigned int
            );
            ~GranularSynth();

            void SetVoiceEnabled(unsigned int idx, bool enabled) {
                if (enabled && !unk2c[idx].enabled) {
                    if ((float)unk18 - unk2c[idx].unk10 > unk14 * 3) {
                        unk2c[idx].unk10 = unk18;
                    }
                }
                unk2c[idx].enabled = enabled;
            }

        private:
            const std::vector<float> &unk0;
            float unk4;
            float unk8;
            float unkc;
            unsigned int unk10;
            unsigned int unk14;
            unsigned int unk18;
            int unk1c;
            std::vector<Granule> unk20;
            std::vector<Voice> unk2c;
            std::vector<std::vector<float> > unk38;
        };
    }
}
