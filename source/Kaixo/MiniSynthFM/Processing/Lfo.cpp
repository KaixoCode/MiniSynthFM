
// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Processing/Lfo.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------
    
    void LfoParameters::frequency(float hz) { m_Frequency = hz; }
    void LfoParameters::tempo(float t) { m_Tempo = normalToIndex(t, Tempo::Amount); }
    void LfoParameters::tempo(Tempo t) { m_Tempo = t; }
    void LfoParameters::waveform(float w) { m_Waveform = normalToIndex(w, LfoWaveform::Amount); }
    void LfoParameters::waveform(LfoWaveform w) { m_Waveform = w; }
    void LfoParameters::synced(bool s) { m_Synced = s; }

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

    void Lfo::trigger(std::size_t i) {
        m_Phase[i] = 0;
    }

    // ------------------------------------------------

}

// ------------------------------------------------
