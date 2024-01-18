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
    
    enum class LfoWaveform {
        Sine, Triangle, Saw, Square, Quantized, Noise, Amount
    };

    // ------------------------------------------------
    
    struct LfoParameters : public Module {

        // ------------------------------------------------
        
        enum class Tempo {
            Freeze, T32_1, T16_1, T8_1, T4_1, T2_1, T1_1, T1_2, T1_4, T1_8, T1_16, T1_32, T1_64, Amount
        };

        // ------------------------------------------------

        void frequency(float hz) { m_Frequency = hz; }
        void tempo(float t) { m_Tempo = normalToIndex(t, Tempo::Amount); }
        void tempo(Tempo t) { m_Tempo = t; }
        void waveform(float w) { m_Waveform = normalToIndex(w, LfoWaveform::Amount); }
        void waveform(LfoWaveform w) { m_Waveform = w; }
        void synched(bool s) { m_Synched = s; }

        // ------------------------------------------------
        
        float bars() {
            switch (m_Tempo) {
            case Tempo::Freeze: return 0.f;
            case Tempo::T32_1:  return 32.f;
            case Tempo::T16_1:  return 16.f;
            case Tempo::T8_1:   return 8.f;
            case Tempo::T4_1:   return 4.f;
            case Tempo::T2_1:   return 2.f;
            case Tempo::T1_1:   return 1.f;
            case Tempo::T1_2:   return 0.5f;
            case Tempo::T1_4:   return 0.25f;
            case Tempo::T1_8:   return 0.125f;
            case Tempo::T1_16:  return 0.0625f;
            case Tempo::T1_32:  return 0.03125f;
            case Tempo::T1_64:  return 0.015625f;
            }
        };

        // ------------------------------------------------

        float samplesPerOscillation() {
            float samplesPerOscillation = 1;
            if (m_Synched) {
                float nmrBarsForTempo = bars();
                float beatsPerSecond = bpm() / 60;
                float beatsPerBar = timeSignature().numerator;
                float barsPerSecond = beatsPerSecond / beatsPerBar;
                float samplesPerBar = sampleRate() / barsPerSecond;
                return nmrBarsForTempo * samplesPerBar;
            } else {
                return sampleRate() / m_Frequency;
            }
        }

        // ------------------------------------------------

    private:
        float m_Frequency; // Hz
        
        bool m_Synched;
        
        Tempo m_Tempo = Tempo::T1_1;
        LfoWaveform m_Waveform = LfoWaveform::Sine;

        // ------------------------------------------------
    
        friend class Lfo;

        // ------------------------------------------------
    
    };

    // ------------------------------------------------
    
    class Lfo : public Module {
    public:
        
        // ------------------------------------------------
        
        LfoParameters& params;

        // ------------------------------------------------

        Lfo(LfoParameters& p) 
            : params(p) 
        {}

        // ------------------------------------------------
        
        float output = 0;

        // ------------------------------------------------
        
        void process() override {
            output = at(m_Phase);

            float delta = 1. / params.samplesPerOscillation();
            m_Phase = Math::fmod1(m_Phase + delta);

            // Phase wrapped around
            if (m_Phase < delta) {
                m_Quantized = m_Random.next() * 2 - 1;
            }
        }

        // ------------------------------------------------
        
        void trigger() {
            m_Phase = 0;
        }

        // ------------------------------------------------
        
        float at(float x) {
            switch (params.m_Waveform) {
            case LfoWaveform::Sine: return Math::Fast::nsin(x - 0.5);
            case LfoWaveform::Triangle: return 1 - Math::Fast::abs(2 - 4 * x);
            case LfoWaveform::Saw: return 1 - 2 * x;
            case LfoWaveform::Square: return 1 - 2 * (x > 0.5);
            case LfoWaveform::Quantized: return m_Quantized;
            case LfoWaveform::Noise: return m_Noise;
            }
        }

        // ------------------------------------------------

    private:
        float m_Phase = 0;
        float m_Quantized = 0;
        float m_Noise = 0;

        Random m_Random{};

        // ------------------------------------------------

    };

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

    };

    // ------------------------------------------------

}

// ------------------------------------------------
