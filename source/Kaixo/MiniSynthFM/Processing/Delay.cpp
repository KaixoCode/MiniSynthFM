
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

        // Random delay every 1000 millis
        auto timer = 1000 * sampleRate() / 1000.;
        if (m_Counter++ > timer) {
            m_Counter = 0;
            m_RandomDelayL = 0.1 * m_Random.next();
            m_RandomDelayR = 0.1 * m_Random.next();
        }

        m_TargetDelayL = m_RealDelayL + m_RandomDelayL * 2 - 1;
        m_TargetDelayR = m_RealDelayR + m_RandomDelayR * 2 - 1;

        float out = read(m_DelayL);
        m_FilterParameters.frequency = 0.8;
        m_FilterParameters.drive = 0.3;
        m_FilterParameters.resonance = 0.4;
        m_HighpassFilter[0].frequency(200);
        m_HighpassFilter[0].resonance(0.2);
        m_HighpassFilter[0].type(FilterType::HighPass);

        float back = input + m_Feedback * out;
        m_Filter.input = back;
        m_Filter.process();
        m_HighpassFilter.input = m_Filter.output;
        m_HighpassFilter.process();
        back = m_HighpassFilter.output.average();

        m_Samples[m_Write] = back;

        if (m_PingPong) {
            float outpp = read(m_DelayR);
            output = input * (1 - m_Mix) + m_Mix * Stereo{ out, outpp };
        } else {
            output = input * (1 - m_Mix) + m_Mix * out;
        }

        m_Write = (m_Write + 1) % size();
        m_DelayL = m_DelayL * m_Smooth + m_TargetDelayL * (1 - m_Smooth);
        m_DelayR = m_DelayR * m_Smooth + m_TargetDelayR * (1 - m_Smooth);
    }

    void Delay::prepare(double sampleRate, std::size_t maxBufferSize) {
        resize(sampleRate * MaxDelaySeconds * 2);
        m_Smooth = Math::smoothCoef(0.9999, 48000. / sampleRate);
    }

    void Delay::reset() {
        m_DelayR = m_TargetDelayR;
        m_DelayL = m_TargetDelayL;
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
