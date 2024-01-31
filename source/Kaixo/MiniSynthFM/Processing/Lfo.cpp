
// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Processing/Lfo.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    float LfoParameters::bars() {
        switch (m_Tempo) {
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

    float LfoParameters::samplesPerOscillation() {
        float samplesPerOscillation = 1;
        if (m_Synced) {
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

    Lfo::Lfo(LfoParameters& p)
        : params(p)
    {}

    // ------------------------------------------------

    void Lfo::process() {
        output = at(m_Phase);

        float delta = 1. / params.samplesPerOscillation();
        m_Phase = Math::fmod1(m_Phase + delta);

        // Phase wrapped around
        if (m_Phase < delta) {
            m_Quantized = m_Random.next() * 2 - 1;
        }
    }

    // ------------------------------------------------

    void Lfo::trigger() {
        m_Phase = 0;
    }

    // ------------------------------------------------

    float Lfo::at(float x) {
        switch (params.m_Waveform) {
        case LfoWaveform::Sine: return Math::Fast::nsin(0.5 - x);
        case LfoWaveform::Triangle: return 1 - Math::Fast::abs(2 - 4 * x);
        case LfoWaveform::Saw: return 1 - 2 * x;
        case LfoWaveform::Square: return 1 - 2 * (x > 0.5);
        case LfoWaveform::Quantized: return m_Quantized;
        case LfoWaveform::Noise: return m_Noise;
        }
    }

    // ------------------------------------------------

}

// ------------------------------------------------
