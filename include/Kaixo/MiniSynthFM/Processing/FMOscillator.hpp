#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Processing/Module.hpp"

// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Controller.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    class FMOscillator : public Module {
    public:

        // ------------------------------------------------

        enum class Waveform { Sine, Triangle, Saw, Square, Amount };

        // ------------------------------------------------

        float output{};

        // ------------------------------------------------

        void trigger() {
            m_Phase = 0;
        }

        // ------------------------------------------------

        void note(float note) {
            if (m_Note != note) {
                m_Note = note;
                updateFrequency();
            }
        }

        void tune(float noteOffset) {
            if (m_Tune != noteOffset) {
                m_Tune = noteOffset;
                updateFrequency();
            }
        }

        void fm(float phase) {
            m_PhaseModulation += phase;
        }

        void volume(float v) { m_Volume = v; }

        void waveform(Waveform wf) { m_Waveform = wf; }
        void waveform(float val) { m_Waveform = normalToIndex(val, Waveform::Amount); }

        // ------------------------------------------------

        float at(float p) {
            // requires 0 <= p <= 1
            switch (m_Waveform) {
            case Waveform::Sine: return Math::Fast::nsin(p - 0.5);
            case Waveform::Triangle: return 0;
            case Waveform::Saw: return Math::Fast::saw(p, m_Frequency / sampleRate());
            case Waveform::Square: return 0;
            }
        }

        // ------------------------------------------------

        void process() override {
            float phase = m_Phase + m_PhaseModulation;
            float wave = at(Math::Fast::fmod1(phase + 10));
            output = wave * m_Volume;
            m_Phase = Math::Fast::fmod1(m_Phase + m_Frequency / sampleRate());
            m_PhaseModulation = 0;
        }

        // ------------------------------------------------

    private:
        float m_Phase = 0;
        float m_PhaseModulation = 0;
        float m_Frequency = 440;
        float m_Volume = 1;

        // ------------------------------------------------

        Waveform m_Waveform = Waveform::Sine;

        // ------------------------------------------------

        Note m_Note = 0;
        Note m_Tune = 0;

        // ------------------------------------------------

        void updateFrequency() {
            m_Frequency = noteToFreq(m_Note + m_Tune);
        }

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------
