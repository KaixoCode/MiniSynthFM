
// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Processing/FMOscillator.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------
    
    FMOscillatorParameters::FMOscillatorParameters(Quality& q)
        : quality(q) 
    {}

    // ------------------------------------------------

    void FMOscillatorParameters::tune(Note t) { m_Tune = t; m_FrequencyDirty = true; }
    void FMOscillatorParameters::octave(int o) { m_Octave = o; m_FrequencyDirty = true; }

    void FMOscillatorParameters::waveform(Waveform wf) { m_Waveform = wf; }
    void FMOscillatorParameters::waveform(float val) { waveform(normalToIndex(val, Waveform::Amount)); }

    // ------------------------------------------------

    float FMOscillatorParameters::frequencyMultiplier() { return m_FrequencyMultiplier; }

    // ------------------------------------------------

    void FMOscillatorParameters::updateFrequency() {
        if (!m_FrequencyDirty) return;
        m_FrequencyDirty = false;
        m_FrequencyMultiplier = Math::Fast::exp2(1. / 12. * m_Tune + m_Octave);
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
