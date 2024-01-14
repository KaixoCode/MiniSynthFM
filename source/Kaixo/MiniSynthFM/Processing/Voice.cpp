
// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Processing/Voice.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

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

        float freqValue = Math::Fast::magnitude_to_log(Math::Fast::clamp(params.frequency + frequencyModulation, 0, 1), 16., 16000.);

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
        input = params.drive * Kaixo::Math::Fast::tanh(input * drive) + input * (1 - params.drive);
        
        m_Filter.input = input;
        m_Filter.process();
        output = m_Filter.output;
        output = params.drive * Kaixo::Math::Fast::tanh(1.115 * output) + output * (1 - 0.9 * params.drive);
    }

    // ------------------------------------------------

    VoiceParameters::VoiceParameters() {
        for (auto& env : envelope) registerModule(env);
        for (auto& osc : oscillator) registerModule(osc);
        registerModule(filter);
    }

    // ------------------------------------------------

    MiniSynthFMVoice::MiniSynthFMVoice(VoiceParameters& p) : params(p) {
        for (auto& osc : oscillator) registerModule(osc);
        for (auto& env : envelope) registerModule(env);
        registerModule(filter);
    }

    // ------------------------------------------------

    void MiniSynthFMVoice::trigger() {
        for (auto& osc : oscillator) osc.trigger();
        for (auto& env : envelope) env.trigger();
    }

    void MiniSynthFMVoice::release() {
        for (auto& env : envelope) env.release();
    }

    // ------------------------------------------------

    void MiniSynthFMVoice::process() {
        for (auto& osc : oscillator) {
            osc.note(note + params.pitchBend * 24 - 12);
            osc.process();
        }

        for (auto& env : envelope) env.process();

        oscillator[0].fm(oscillator[1].fmOutput * params.fm[1]);
        oscillator[1].fm(oscillator[2].fmOutput * params.fm[2]);
        oscillator[2].fm(oscillator[0].fmOutput * params.fm[0]);

        result = (oscillator[0].output * params.volume[0]
                + oscillator[1].output * params.volume[1]
                + oscillator[2].output * params.volume[2]);

        filter.input = result;
        filter.frequencyModulation = envelope[1].output * params.envelopeLevel[1];
        filter.process();
        result = filter.output
            * envelope[0].output * params.envelopeLevel[0];
    }

    // ------------------------------------------------

}

// ------------------------------------------------
