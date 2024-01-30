
// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Processing/FMOscillator.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    FMOscillator::FMOscillator(FMOscillatorParameters& p) 
        : params(p) 
    {}

    // ------------------------------------------------

    void FMOscillator::trigger() {
        m_Phase = 0;
    }

    // ------------------------------------------------
    
    void FMOscillator::note(float note) { 
        if (m_Note != note) {
            m_Note = note;
            m_NoteFrequency = noteToFreq(m_Note);
        }
    }

    void FMOscillator::fm(float phase) { m_PhaseModulation += phase; }

    // ------------------------------------------------

    void FMOscillator::process() {
        updateFrequency();
        float delta = m_Frequency / sampleRate();
        float phase = Math::Fast::fmod1(m_Phase + m_PhaseModulation + 10);
        output = at(phase);
        fmOutput = fmAt(phase);
        m_Phase = Math::Fast::fmod1(m_Phase + delta);
        m_PhaseModulation = 0;
        
        m_DidCycle = m_Phase < delta;
    }

    // ------------------------------------------------
    
    void FMOscillator::hardSync(FMOscillator& osc) {
        if (osc.m_DidCycle) {
            float ratio = m_Frequency / osc.m_Frequency;
            m_Phase = Math::Fast::fmod1(osc.m_Phase * ratio);
        }
    }

    // ------------------------------------------------
    
    void FMOscillator::resetPhase() {
        m_Phase = 0;
    }

    // ------------------------------------------------

    void FMOscillator::updateFrequency() {
        m_Frequency = m_NoteFrequency * params.frequencyMultiplier();
    }

    // ------------------------------------------------

    float FMOscillator::at(float p) {
        // requires 0 <= p <= 1
        float xd = Math::Fast::max(1.5 * m_Frequency / sampleRate(), 0.002);
        switch (params.m_Waveform) {
        case Waveform::Sine: return Math::Fast::nsin(p - 0.5);
        case Waveform::Triangle: return 1 - Math::Fast::abs(2 - 4 * p);
        case Waveform::Saw: return Math::Fast::saw(p + xd, xd);
        case Waveform::Square: return Math::Fast::saw(p + xd, xd) + Math::Fast::saw(0.5 - p + xd, xd);
        }
    }

    float FMOscillator::fmAt(float p) {
        // requires 0 <= p <= 1
        switch (params.m_Waveform) {
        case Waveform::Sine: return Math::Fast::nsin(p - 0.5);
        case Waveform::Triangle: return 2 * (2 * p - 1) * ((2 * p - 1) * Math::Fast::sign(0.5 - p) + 1);
        case Waveform::Saw: return 4 * (p - p * p);
        case Waveform::Square: return 1 - Math::Fast::abs(2 - 4 * p);
        }
    }

    // ------------------------------------------------

}

// ------------------------------------------------
