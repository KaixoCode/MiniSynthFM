
// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Processing/Delay.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------
    
    Delay::Delay() {
        registerModule(m_Filter);
    }

    // ------------------------------------------------

    void Delay::mix(float v) { m_Mix = v; }
    void Delay::delay(float millis) { m_Delay = Math::Fast::clamp(millis, 0, MaxDelaySeconds * 1000); }
    void Delay::feedback(float fb) { m_Feedback = fb; }
    void Delay::pingpong(bool v) { m_PingPong = v; }
    void Delay::synced(bool v) { m_Synced = v; }
    void Delay::tempo(float v) { tempo(normalToIndex(v, Tempo::Amount)); }
    void Delay::tempo(Tempo v) { m_Tempo = v; }
    void Delay::algorithm(float v) { algorithm(normalToIndex(v, Algorithm::Amount)); }
    void Delay::algorithm(Algorithm v) { m_Algorithm = v; }

    // ------------------------------------------------

    bool Delay::active() const {
        // If more than 'delay' silence: inactive
        return input != 0 || m_SamplesSilence < m_ActualDelay * sampleRate() / 1000.f;
    }

    // ------------------------------------------------
    
    void Delay::process() {

        // every 2 ms
        float timer = 2 * sampleRate() / 1000.;
        if (m_Counter++ > timer) {
            m_RandomFrequency = Random::next();
            m_Counter = 0;
        }

        auto bars = [&]() {
            switch (m_Tempo) {
            case Tempo::T1_1:   return 1.f;
            case Tempo::T1_2:   return 0.5f;
            case Tempo::T1_4:   return 0.25f;
            case Tempo::T1_6:   return 0.1875f;
            case Tempo::T1_8:   return 0.125f;
            case Tempo::T1_16:  return 0.0625f;
            case Tempo::T1_32:  return 0.03125f;
            case Tempo::T1_64:  return 0.015625f;
            }
        };

        float delay = m_SmoothedDelay;
        if (m_Synced) {
            float nmrBarsForTempo = bars();
            float beatsPerSecond = bpm() / 60;
            float beatsPerBar = timeSignature().numerator;
            float secondsPerBar = beatsPerBar / beatsPerSecond;
            float seconds = nmrBarsForTempo * secondsPerBar;
            delay = seconds * 1000;
        } else {
            float ratio = sampleRate() / 48000;
            float r = Math::pow(0.9999, ratio);
            float maxDiff = ratio * 0.1;
            float diff = Math::Fast::clamp((1 - r) * (m_Delay - m_SmoothedDelay), -maxDiff, maxDiff);
            m_SmoothedDelay += diff;
        }

        if (m_PingPong) delay *= 2;
        m_ActualDelay = delay;

        float out1 = read(delay);

        bool doDrive = false;
        switch (m_Algorithm) {
        case Algorithm::Dirty: {
            doDrive = true;
            m_Filter[0].type(FilterType::HighPass);
            m_Filter[0].frequency(200);
            m_Filter[0].resonance(0.2);
            m_Filter[1].type(FilterType::PeakingEQ);
            m_Filter[1].frequency(4000);
            m_Filter[1].resonance(0.4);
            m_Filter[2].type(FilterType::LowPass);
            m_Filter[2].frequency(3600 + (m_RandomFrequency * 2 - 1) * 100);
            m_Filter[2].resonance(0.25);
            m_Filter[2].gain(-2.4);
            m_Filter[3].type(FilterType::PeakingEQ);
            m_Filter[3].frequency(4400);
            m_Filter[3].resonance(0);
            m_Filter[3].gain(2.4);
            break;
        }
        case Algorithm::Dark: {
            m_Filter[0].type(FilterType::HighPass);
            m_Filter[0].frequency(70);
            m_Filter[0].resonance(0.4);
            m_Filter[1].type(FilterType::PeakingEQ);
            m_Filter[1].frequency(9000);
            m_Filter[1].resonance(0.1);
            m_Filter[2].type(FilterType::LowPass);
            m_Filter[2].frequency(1400);
            m_Filter[2].resonance(0.1);
            m_Filter[2].gain(-2.4);
            m_Filter[3].type(FilterType::PeakingEQ);
            m_Filter[3].frequency(250);
            m_Filter[3].resonance(0);
            m_Filter[3].gain(-1.2);
            break;
        }
        case Algorithm::Open: {
            m_Filter[0].type(FilterType::HighPass);
            m_Filter[0].frequency(200);
            m_Filter[0].resonance(0.2);
            m_Filter[1].type(FilterType::PeakingEQ);
            m_Filter[1].frequency(7000);
            m_Filter[1].resonance(0.4);
            m_Filter[2].type(FilterType::LowPass);
            m_Filter[2].frequency(3600 + (m_RandomFrequency * 2 - 1) * 100);
            m_Filter[2].resonance(0.25);
            m_Filter[2].gain(-1.8);
            m_Filter[3].type(FilterType::PeakingEQ);
            m_Filter[3].frequency(4400);
            m_Filter[3].resonance(0);
            m_Filter[3].gain(2.4);
            break;
        }
        case Algorithm::Clean: {
            m_Filter[0].type(FilterType::HighPass);
            m_Filter[0].frequency(200);
            m_Filter[0].resonance(0.2);
            m_Filter[1].type(FilterType::LowPass);
            m_Filter[1].frequency(16000);
            m_Filter[1].resonance(0.1);
            m_Filter[2].type(FilterType::HighShelf);
            m_Filter[2].frequency(1500);
            m_Filter[2].resonance(0.0);
            m_Filter[2].gain(-0.5);
            m_Filter[3].type(FilterType::LowShelf);
            m_Filter[3].frequency(2000);
            m_Filter[3].resonance(0);
            m_Filter[3].gain(-0.8);
            break;
        }
        }

        if (doDrive) {
            float drive = 0.4;
            float fedBack = 2 * m_Feedback * out1;
            m_Filter.input = input + fedBack + drive * (Math::Fast::tanh_like(1.2 * fedBack * out1) - fedBack);
            m_Filter.process();
            float back = m_Filter.output.average();
            m_Samples[m_Write] = 0.4 * Math::Fast::tanh_like(1.115 * back) + 0.64 * back;
        } else {
            float drive = 0.2;
            float fedBack = 1.7 * m_Feedback * out1;
            m_Filter.input = input + fedBack + drive * (Math::Fast::tanh_like(fedBack * out1) - 0.8 * fedBack);
            m_Filter.process();
            float back = m_Filter.output.average();
            m_Samples[m_Write] = 0.82 * back;
        }

        float myOutput = 0;
        if (m_PingPong) {
            float out2 = read(delay * 0.5);
            myOutput = Math::Fast::abs(out1 + out2);
            output = input * (1 - m_Mix) + m_Mix * Stereo{ out1, out2 };
        } else {
            output = input * (1 - m_Mix) + m_Mix * out1;
            myOutput = Math::Fast::abs(out1);
        }

        if (input == 0 && myOutput < std::numeric_limits<float>::epsilon()) {
            ++m_SamplesSilence;
        } else {
            m_SamplesSilence = 0;
        }

        m_Write = (m_Write + 1) % size();
    }

    void Delay::prepare(double sampleRate, std::size_t maxBufferSize) {
        ModuleContainer::prepare(sampleRate, maxBufferSize);
        resize(sampleRate * MaxDelaySeconds * 2);
    }

    void Delay::reset() {
        std::ranges::fill(m_Samples, 0);
    }

    // ------------------------------------------------

    float Delay::read(float delayMs) const {
        float delaySamples = sampleRate() * delayMs / 1000.;
        float read = Math::Fast::fmod(m_Write + 2 * size() - delaySamples, size());
        std::size_t delay1 = static_cast<std::size_t>(read);
        std::size_t delay2 = static_cast<std::size_t>(read + 1) % size();
        float ratio = read - delay1;
        return m_Samples[delay2] * ratio + m_Samples[delay1] * (1 - ratio);
    }

    // ------------------------------------------------

    void Delay::resize(std::size_t size) {
        m_Samples.resize(size);
        reset();
    }

    // ------------------------------------------------

}

// ------------------------------------------------
