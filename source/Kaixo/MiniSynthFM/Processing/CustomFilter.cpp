
// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Processing/CustomFilter.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    // ------------------------------------------------

    std::size_t FilterParameters::oversample() const {
        switch (quality) {
        case Quality::Low: return 1;
        case Quality::Normal: return 2;
        case Quality::High: return 4;
        case Quality::Ultra: return 8;
        case Quality::Extreme: return 16;
        default: return 1;
        }
    }

    // ------------------------------------------------

    CustomFilter::CustomFilter(FilterParameters& p)
        : params(p)
    {
        registerModule(m_Filter);
    }

    // ------------------------------------------------

    void CustomFilter::process() {

        // every 2 ms
        float timer = 2 * sampleRate() / 1000.;
        if (m_Counter++ > timer) {
            m_RandomFreq = m_Random.next();
            m_Counter = 0;
        }

        m_FrequencyModulation = m_FrequencyModulation * m_Ratio + frequencyModulation * (1 - m_Ratio);

        float freqValue = Math::Fast::magnitude_to_log(params.frequency + m_FrequencyModulation, 16., 16000.);

        if (params.keytrack) {
            freqValue = Math::Fast::clamp(freqValue * Math::Fast::exp2((note - 60) / 12.), 16, 16000);
        } else {
            freqValue = Math::Fast::clamp(freqValue, 16, 16000);
        }

        float nfreq = (freqValue / 16000);
        float randRange = 24 * (1 - (1 - params.drive) * (1 - params.drive)) + 6 * (1 - nfreq * nfreq);
        float frequency = Math::Fast::clamp(freqValue + m_RandomFreq * randRange * 2 - randRange, 16, 16000);
        float resonance = params.resonance * (1 - nfreq * nfreq * nfreq * nfreq);

        // Less resonance when low frequency
        if (nfreq < 0.01) {
            resonance *= 0.2 + 0.8 * (nfreq / 0.01);
        }

        m_Filter[0].type(FilterType::LowPass);
        m_Filter[0].frequency(frequency);
        m_Filter[0].resonance(resonance);
        m_Filter[1].type(FilterType::PeakingEQ);
        m_Filter[1].frequency(frequency * 0.9);
        m_Filter[1].resonance(resonance * 0.2 + 0.2);
        m_Filter[1].gain(-resonance * 15 + params.drive * 12);
        m_Filter[2].type(FilterType::PeakingEQ);
        m_Filter[2].frequency(frequency * 1.1);
        m_Filter[2].resonance(-resonance * 0.2 + 0.2);
        m_Filter[2].gain(resonance * 15 - params.drive * 12);

        auto drive = Math::Fast::db_to_magnitude(params.drive * 12);

        float res = 0;
        if (params.oversample() == 1) {
            res = params.drive * Math::Fast::tanh_like(input[0] * drive) + input[0] * (1 - params.drive);
        } else {
            m_AAF.sampleRateIn = sampleRate() * params.oversample();
            m_AAF.sampleRateOut = sampleRate();

            for (std::size_t i = 0; i < params.oversample(); ++i) {
                input[i] = params.drive * Math::Fast::tanh_like(input[i] * drive) + input[i] * (1 - params.drive);
                res = m_AAF.process(input[i]);
            }
        }

        m_Filter.input = res;
        m_Filter.process();
        output = m_Filter.output.average();
        output = params.drive * Math::Fast::tanh_like(1.115 * output) + output * (1 - 0.9 * params.drive);
    }

    // ------------------------------------------------

    void CustomFilter::prepare(double sampleRate, std::size_t maxBufferSize) {
        m_Ratio = Math::smoothCoef(0.99, 96000 / sampleRate);
    }

    void CustomFilter::reset() {
        ModuleContainer::reset();
        m_AAF.reset();
    }



    // ------------------------------------------------

}

// ------------------------------------------------
