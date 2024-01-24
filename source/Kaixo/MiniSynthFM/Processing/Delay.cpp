
// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Processing/Delay.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------
    
    Delay::Delay() {
        registerModule(m_Filter);
        registerModule(m_HighpassFilter);
    }

    // ------------------------------------------------
    
    void Delay::process() {

        auto bars = [&]() {
            switch (m_Tempo) {
            case Tempo::T1_1:   return 1.f;
            case Tempo::T1_2:   return 0.5f;
            case Tempo::T1_4:   return 0.25f;
            case Tempo::T1_6:   return 0.16666667f;
            case Tempo::T1_8:   return 0.125f;
            case Tempo::T1_16:  return 0.0625f;
            case Tempo::T1_32:  return 0.03125f;
            case Tempo::T1_64:  return 0.015625f;
            }
        };

        float delay = m_Delay;
        if (m_Synced) {
            float nmrBarsForTempo = bars();
            float beatsPerSecond = bpm() / 60;
            float beatsPerBar = timeSignature().numerator;
            float secondsPerBar = beatsPerBar / beatsPerSecond;
            float seconds = nmrBarsForTempo * secondsPerBar;
            delay = seconds * 1000;
        }

        if (m_PingPong) delay *= 2;

        float out1 = read(delay);
        m_FilterParameters.frequency = 0.8;
        m_FilterParameters.drive = 0.3;
        m_FilterParameters.resonance = 0.4;
        m_HighpassFilter[0].frequency(200);
        m_HighpassFilter[0].resonance(0.2);
        m_HighpassFilter[0].type(FilterType::HighPass);

        float back = input + m_Feedback * out1;
        m_Filter.input = back;
        m_Filter.process();
        m_HighpassFilter.input = m_Filter.output;
        m_HighpassFilter.process();
        back = m_HighpassFilter.output.average();

        m_Samples[m_Write] = back;

        if (m_PingPong) {
            float out2 = read(delay * 0.5);
            output = input * (1 - m_Mix) + m_Mix * Stereo{ out1, out2 };
        } else {
            output = input * (1 - m_Mix) + m_Mix * out1;
        }

        m_Write = (m_Write + 1) % size();
        m_Delay = m_TargetDelay;
        //m_Delay = m_Delay * m_Smooth + m_TargetDelay * (1 - m_Smooth);
    }

    void Delay::prepare(double sampleRate, std::size_t maxBufferSize) {
        resize(sampleRate * MaxDelaySeconds * 2);
        m_Smooth = Math::smoothCoef(0.99, 48000. / sampleRate);
    }

    void Delay::reset() {
        m_Delay = m_TargetDelay;
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
