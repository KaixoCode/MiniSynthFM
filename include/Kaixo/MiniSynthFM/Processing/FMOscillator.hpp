#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Processing/Module.hpp"

// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Controller.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    enum class Waveform { Sine, Triangle, Saw, Square, Amount };

    // ------------------------------------------------
    
    struct FMOscillatorParameters : public Module {

        // ------------------------------------------------

        void volume(float v) { m_Volume = v; }
        void tune(Note t) { m_Tune = t; }

        void waveform(Waveform wf) { m_Waveform = wf; }
        void waveform(float val) { m_Waveform = normalToIndex(val, Waveform::Amount); }

        // ------------------------------------------------
    private:
        float m_Volume;
        Note m_Tune;
        Waveform m_Waveform;

        // ------------------------------------------------
        
        friend class FMOscillator;

        // ------------------------------------------------

    };

    // ------------------------------------------------

    class FMOscillator : public Module {
    public:

        // ------------------------------------------------
        
        FMOscillatorParameters& params;
        
        // ------------------------------------------------
        
        FMOscillator(FMOscillatorParameters& p) : params(p) {}

        // ------------------------------------------------

        float output{};

        // ------------------------------------------------

        void trigger() {
            m_Phase = 0;
        }

        // ------------------------------------------------

        void note(float note) { m_Note = note; }

        void fm(float phase) { m_PhaseModulation += phase; }

        // ------------------------------------------------

        float at(float p) {
            // requires 0 <= p <= 1
            switch (params.m_Waveform) {
            case Waveform::Sine: return Math::Fast::nsin(p - 0.5);
            case Waveform::Triangle: return 0;
            case Waveform::Saw: return Math::Fast::saw(p, m_Frequency / sampleRate());
            case Waveform::Square: return 0;
            }
        }

        // ------------------------------------------------

        void process() override {
            updateFrequency();
            float phase = m_Phase + m_PhaseModulation;
            float wave = at(Math::Fast::fmod1(phase + 10));
            output = wave * params.m_Volume;
            m_Phase = Math::Fast::fmod1(m_Phase + m_Frequency / sampleRate());
            m_PhaseModulation = 0;
        }

        // ------------------------------------------------

    private:
        float m_Phase = 0;
        float m_PhaseModulation = 0;
        float m_Frequency = 440;

        // ------------------------------------------------

        Note m_Note = 0;

        // ------------------------------------------------

        void updateFrequency() {
            m_Frequency = noteToFreq(m_Note + params.m_Tune);
        }

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------
