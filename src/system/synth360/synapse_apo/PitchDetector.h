#pragma once
#include "synth360/synapse_apo/SpectralAnalysis.h"
#include <vector>

namespace DSP {
    namespace Synapse {
        // size 0x128
        class PitchDetector {
        public:
            PitchDetector(const std::vector<float> &, unsigned int, unsigned int);
            ~PitchDetector();

            void Detect(unsigned int);

        private:
            const std::vector<float> &unk0;
            unsigned int unk4;
            unsigned int unk8;
            float unkc;
            float unk10;
            float unk14;
            SpectralAnalysis unk18;
            int unkf4;
            int unkf8;
            int unkfc;
            std::vector<float> unk100;
            std::vector<float> unk10c;
            std::vector<float> unk118;
            int unk124;
        };
    }
}
