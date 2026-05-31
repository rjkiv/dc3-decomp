#pragma once
#include <vector>

namespace DSP {
    namespace Synapse {
        class PeakDetector {
        public:
            PeakDetector(const std::vector<float> &, unsigned int, unsigned int);
            void Detect(unsigned int);

        private:
            float gaussianWindow(unsigned int) const;

            const std::vector<float> &unk0;
            float unk4;
            unsigned int unk8;
            unsigned int unkc;
            int unk10;
            int unk14;
            int unk18;
            int unk1c;
            bool unk20;
            float unk24;
            float unk28;
            float unk2c;
            float unk30;
        };
    }
}
