
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

    template<Quality Q> 
    float FMOscillator::atImpl(float p) {
        if constexpr (Q == Quality::Low || Q == Quality::Normal) {
            switch (params.m_Waveform) {
            case Waveform::Sine: return Math::Fast::nsin(p - 0.5);
            case Waveform::Triangle: return 1 - Math::Fast::abs(2 - 4 * p);
            case Waveform::Saw: return Math::Fast::saw(p, Math::Fast::max(1.5 * m_Frequency / sampleRate(), 0.002));
            case Waveform::Square: return p > 0.5 ? 1 : -1;
            }
        } else {
            switch (params.m_Waveform) {
            case Waveform::Sine: return Math::nsin(p - 0.5);
            case Waveform::Triangle: return 1 - Math::Fast::abs(2 - 4 * p);
            case Waveform::Saw: return Math::Fast::saw(p, Math::Fast::max(1.5 * m_Frequency / sampleRate(), 0.002));
            case Waveform::Square: return p > 0.5 ? 1 : -1;
            }
        }
    }

    template<Quality Q> 
    float FMOscillator::fmAtImpl(float p) {
        if constexpr (Q == Quality::Low || Q == Quality::Normal) {
            switch (params.m_Waveform) {
            case Waveform::Sine: return Math::Fast::nsin(p - 0.5);
            case Waveform::Triangle: return 2 * (2 * p - 1) * ((2 * p - 1) * Math::Fast::sign(0.5 - p) + 1);
            case Waveform::Saw: return 4 * (p - p * p);
            case Waveform::Square: return 1 - Math::Fast::abs(2 - 4 * p);
            }
        } else {
            switch (params.m_Waveform) {
            case Waveform::Sine: return Math::nsin(p - 0.5);
            case Waveform::Triangle: return 2 * (2 * p - 1) * ((2 * p - 1) * Math::Fast::sign(0.5 - p) + 1);
            case Waveform::Saw: return 4 * (p - p * p);
            case Waveform::Square: return 1 - Math::Fast::abs(2 - 4 * p);
            }
        }
    }

    template<std::size_t Oversample, Quality Q>
    void FMOscillator::processImpl() {
        float delta = m_Frequency / sampleRate();

        if constexpr (Oversample != 1) {
            float realSampleRate = sampleRate() * Oversample;

            m_AAF.sampleRateIn = realSampleRate;
            m_AAF.sampleRateOut = sampleRate();

            float actualDelta = m_Frequency / realSampleRate;
            float deltaPM = (m_PhaseModulation - m_PreviousPhaseModulation) / Oversample;
            for (std::size_t i = 0; i < Oversample; ++i) {
                float phaseModulation = m_PreviousPhaseModulation + i * deltaPM;
                float currentPhase = m_Phase + i * actualDelta;
                float phase = currentPhase + phaseModulation;

                float out = this->atImpl<Q>(Math::Fast::fmod1(phase + 10));
                float fmOut = this->fmAtImpl<Q>(Math::Fast::fmod1(phase + 10));

                auto [outLP, fmOutLP] = m_AAF.process({ out, fmOut });
                output = outLP;
                fmOutput = fmOutLP;
            }
        } else {
            float phase = m_Phase + m_PhaseModulation;

            output = this->atImpl<Q>(Math::Fast::fmod1(phase + 10));
            fmOutput = this->fmAtImpl<Q>(Math::Fast::fmod1(phase + 10));
        }

        m_Phase = Math::Fast::fmod1(m_Phase + delta);
        
        m_PreviousPhaseModulation = m_PhaseModulation;
        m_PhaseModulation = 0;

        m_DidCycle = m_Phase < delta;
    }

    void FMOscillator::process() {
        auto timer = 10 * sampleRate() / 1000.;
        if (m_Counter++ > timer) {
            m_RandomTune = m_Random.next();
            m_Counter = 0;
        }

        updateFrequency();
        float delta = m_Frequency / sampleRate();
        float phase = m_Phase + m_PhaseModulation;
        output = at(Math::Fast::fmod1(phase + 10));
        fmOutput = fmAt(Math::Fast::fmod1(phase + 10));
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
        m_Frequency = noteToFreq(m_Note + params.m_Tune + params.m_Octave * 12 + m_RandomTune * 0.05 - 0.025);
    }

    // ------------------------------------------------

    float FMOscillator::at(float p) {
        // requires 0 <= p <= 1
        switch (params.m_Waveform) {
        case Waveform::Sine: return Math::Fast::nsin(p - 0.5);
        case Waveform::Triangle: return 1 - Math::Fast::abs(2 - 4 * p);
        case Waveform::Saw: return Math::Fast::saw(p, Math::Fast::max(1.5 * m_Frequency / sampleRate(), 0.002));
        case Waveform::Square: return p > 0.5 ? 1 : -1;
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
