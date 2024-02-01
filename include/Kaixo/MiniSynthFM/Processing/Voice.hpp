#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Processing/Processor.hpp"
#include "Kaixo/Core/Processing/Voice.hpp"
#include "Kaixo/Core/Processing/Module.hpp"
#include "Kaixo/Core/Processing/Filter.hpp"
#include "Kaixo/Core/Processing/Random.hpp"

// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Controller.hpp"
#include "Kaixo/MiniSynthFM/Processing/ADSREnvelope.hpp"
#include "Kaixo/MiniSynthFM/Processing/FMOscillator.hpp"
#include "Kaixo/MiniSynthFM/Processing/CustomFilter.hpp"
#include "Kaixo/MiniSynthFM/Processing/Lfo.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------
    
    struct VoiceParameters : public ModuleContainer {

        // ------------------------------------------------
        
        VoiceParameters();

        // ------------------------------------------------

        LfoParameters lfo[Lfos];
        FilterParameters filter;
        ADSREnvelopeParameters envelope[Envelopes];
        FMOscillatorParameters oscillator[Oscillators];
        float fm[Oscillators]{};
        float volume[Oscillators]{};
        float envelopeLevel[Envelopes]{};
        float lfoLevel[Lfos]{};
        bool outputOscillator[Oscillators]{};

        float pitchBend = 0;
        float modWheel = 0;

        // ------------------------------------------------

        Quality quality = Quality::Normal;

        // ------------------------------------------------
        
        bool routing[(int)ModDestination::Amount][(int)ModSource::Amount]{};

        //std::array<std::bitset<(int)ModSource::Amount>, (int)ModDestination::Amount> routing{};

        void resetRouting() {
            std::memset(routing, 0, sizeof(routing));
            //for (auto& m : routing) m.reset();
        }

        // ------------------------------------------------
        
        std::size_t oversample() const;

        // ------------------------------------------------

    };

    // ------------------------------------------------
    
    class MiniSynthFMVoice : public Voice {
    public:

        // ------------------------------------------------
        
        VoiceParameters& params;

        // ------------------------------------------------

        MiniSynthFMVoice(VoiceParameters& p);

        // ------------------------------------------------
        
        float result = 0;

        // ------------------------------------------------

        void trigger() override;
        void release() override;

        // ------------------------------------------------

        void doModulations();

        void process() override;

        // ------------------------------------------------

        FMOscillator oscillator[Oscillators]{ params.oscillator[0], params.oscillator[1], params.oscillator[2] };
        ADSREnvelope envelope[Envelopes]{ params.envelope[0], params.envelope[1], params.envelope[2] };
        Lfo lfo[Lfos]{ params.lfo[0] };
        CustomFilter filter{ params.filter };

        // ------------------------------------------------
        
        Random random{};
        float randomValue = 0;

        // ------------------------------------------------
        
        float previousLfoLevel = 0;

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------
