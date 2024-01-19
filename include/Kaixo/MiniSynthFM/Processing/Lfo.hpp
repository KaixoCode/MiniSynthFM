#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Processing/Module.hpp"
#include "Kaixo/Core/Processing/Random.hpp"

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

        float bars();

        // ------------------------------------------------

        float samplesPerOscillation();

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

        Lfo(LfoParameters& p);

        // ------------------------------------------------

        float output = 0;

        // ------------------------------------------------

        void process() override;

        // ------------------------------------------------

        void trigger();

        // ------------------------------------------------

        float at(float x);

        // ------------------------------------------------

    private:
        float m_Phase = 0;
        float m_Quantized = 0;
        float m_Noise = 0;

        Random m_Random{};

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------
