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

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------
    
    struct VoiceParameters : public ModuleContainer {

        // ------------------------------------------------
        
        VoiceParameters();

        // ------------------------------------------------

        FilterParameters filter;
        ADSREnvelopeParameters envelope[Envelopes];
        FMOscillatorParameters oscillator[Oscillators];
        float fm[Oscillators]{};
        float volume[Oscillators]{};
        float envelopeLevel[Envelopes]{};

        bool outputOscillator[Oscillators]{};

        float pitchBend = 0;

        // ------------------------------------------------
        
        bool routing[(int)ModSource::Amount][(int)ModDestination::Amount]{};

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
        
        Stereo result{ 0, 0 };

        // ------------------------------------------------

        Note note = 64;

        // ------------------------------------------------

        void trigger() override;
        void release() override;

        // ------------------------------------------------

        void doModulations();

        void process() override;

        // ------------------------------------------------

        FMOscillator oscillator[Oscillators]{ params.oscillator[0], params.oscillator[1], params.oscillator[2] };
        ADSREnvelope envelope[Envelopes]{ params.envelope[0], params.envelope[1] };
        CustomFilter filter{ params.filter };

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------
