
// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Processing/FMOscillator.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    void FMOscillatorParameters::tune(Note t) { m_Tune = t; m_FrequencyDirty = true; }
    void FMOscillatorParameters::octave(int o) { m_Octave = o; m_FrequencyDirty = true; }

    void FMOscillatorParameters::waveform(Waveform wf) { m_Waveform = wf; }
    void FMOscillatorParameters::waveform(float val) { waveform(normalToIndex(val, Waveform::Amount)); }

    void FMOscillatorParameters::quality(Quality val) { m_Quality = val; }
    void FMOscillatorParameters::quality(float val) { quality(normalToIndex(val, Quality::Amount)); }

    // ------------------------------------------------

    float FMOscillatorParameters::frequencyMultiplier() { return m_FrequencyMultiplier; }

    // ------------------------------------------------

    void FMOscillatorParameters::updateFrequency() {
        if (!m_FrequencyDirty) return;
        m_FrequencyDirty = false;
        m_FrequencyMultiplier = Math::Fast::exp2(1. / 12. * m_Tune + m_Octave);
    }

    // ------------------------------------------------

    std::size_t FMOscillatorParameters::oversample() const {
        switch (m_Quality) {
        case Quality::Low: return 1;
        case Quality::Normal: return 2;
        case Quality::High: return 4;
        case Quality::Ultra: return 8;
        case Quality::Extreme: return 16;
        default: return 1;
        }
    }

    // ------------------------------------------------

    FMOscillator::FMOscillator(FMOscillatorParameters& p) 
        : params(p) 
    {}

    // ------------------------------------------------
    
    void FMOscillator::trigger(std::size_t i) { m_Phase[i] = 0; }

    // ------------------------------------------------
    
}

// ------------------------------------------------
