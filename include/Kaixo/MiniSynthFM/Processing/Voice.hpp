#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Processing/Processor.hpp"
#include "Kaixo/Core/Processing/Voice.hpp"
#include "Kaixo/Core/Processing/Module.hpp"
#include "Kaixo/Core/Processing/Filter.hpp"
#include "Kaixo/Core/Processing/Random.hpp"

// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Controller.hpp"
#include "Kaixo/MiniSynthFM/Processing/ADSREnvelope.hpp"
#include "Kaixo/MiniSynthFM/Processing/FMOscillator.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    struct FilterParameters : public Module {

        // ------------------------------------------------

        float frequency;
        float resonance;
        float drive;
        bool enable;

        // ------------------------------------------------

    };

    // ------------------------------------------------

    class CustomFilter : public ModuleContainer {
    public:

        // ------------------------------------------------

        FilterParameters& params;

        // ------------------------------------------------

        CustomFilter(FilterParameters& p);

        // ------------------------------------------------

        Stereo input{ 0, 0 };
        Stereo output{ 0, 0 };

        // ------------------------------------------------
        
        void process() override;
        void prepare(double sampleRate, std::size_t maxBufferSize) override;

        // ------------------------------------------------
        
        float frequencyModulation = 0;

        // ------------------------------------------------

    private:
        StereoEqualizer<3, float, Kaixo::Math::Fast, false> m_Filter{};

        Random m_Random{};
        std::size_t m_Counter = 0;
        float m_RandomFreq{};
        float m_FrequencyModulation{};
        float m_Ratio = 0.99;

        // ------------------------------------------------

    };

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
