
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

        constexpr auto g = [](auto x) {
            return (x - 1) * x * (2 * x - 1);
        };
        
        constexpr auto saw = [](float x, float nf) {
            float v1 = Math::Fast::fmod1(x - nf + 1);
            float v2 = Math::Fast::fmod1(x + nf);
            float v3 = x;

            return (g(v1) + g(v2) - 2 * g(v3)) / (6.f * nf * nf);
        };
        
        constexpr auto square = [](float x, float nf) {
            float v1 = Math::Fast::fmod1(x - nf + 1);
            float v2 = Math::Fast::fmod1(x + nf);
            float v3 = x;
            float v4 = Math::Fast::fmod1(2.5 - x - nf);
            float v5 = Math::Fast::fmod1(1.5 - x + nf);
            float v6 = Math::Fast::fmod1(1.5 - x);

            return (g(v1) + g(v2) - 2 * g(v3) + g(v4) + g(v5) - 2 * g(v6)) / (6.f * nf * nf);
        };

        // requires 0 <= p <= 1
        float xd = Math::Fast::max(m_Frequency / 30000.f, 0.002f);
        switch (params.m_Waveform) {
        case Waveform::Sine: return Math::Fast::nsin(p - 0.5);
        case Waveform::Triangle: return 1 - Math::Fast::abs(2 - 4 * p);
        case Waveform::Saw: return saw(p, xd);
        case Waveform::Square: return square(p, xd);
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
