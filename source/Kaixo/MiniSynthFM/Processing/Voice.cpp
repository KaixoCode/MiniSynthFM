
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
        if (!params.enable) {
            output = input;
            return;
        }

        // every 2 ms
        float timer = 2 * sampleRate() / 1000.;
        if (m_Counter++ > timer) {
            m_RandomFreq = m_Random.next();
            m_Counter = 0;
        }

        m_FrequencyModulation = m_FrequencyModulation * m_Ratio + frequencyModulation * (1 - m_Ratio);

        float freqValue = Math::Fast::magnitude_to_log(Math::Fast::clamp(params.frequency + m_FrequencyModulation, 0, 1), 16., 16000.);

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

    void CustomFilter::prepare(double sampleRate, std::size_t maxBufferSize) {
        m_Ratio = Math::smoothCoef(0.99, 96000 / sampleRate);
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
    
    void MiniSynthFMVoice::doModulations() {
        auto env1level = envelope[0].output * params.envelopeLevel[0];
        auto env2level = envelope[1].output * params.envelopeLevel[1];
        
        auto lfolevel = 0;

        auto op1level = oscillator[0].output * params.volume[0];
        auto op2level = oscillator[1].output * params.volume[1];
        auto op3level = oscillator[2].output * params.volume[2];
        
        auto op1fm = oscillator[0].fmOutput * params.volume[0];
        auto op2fm = oscillator[1].fmOutput * params.volume[1];
        auto op3fm = oscillator[2].fmOutput * params.volume[2];
        
        auto getEnv1 = [&](ModDestination dest) { return params.routing[(int)ModSource::Envelope1][(int)dest] * env1level; };
        auto getEnv2 = [&](ModDestination dest) { return params.routing[(int)ModSource::Envelope2][(int)dest] * env2level; };
        auto getLfo = [&](ModDestination dest) { return params.routing[(int)ModSource::LFO][(int)dest] * lfolevel; };
        auto getOp1 = [&](ModDestination dest) { return params.routing[(int)ModSource::Op1][(int)dest] * op1level; };
        auto getOp2 = [&](ModDestination dest) { return params.routing[(int)ModSource::Op2][(int)dest] * op2level; };
        auto getOp3 = [&](ModDestination dest) { return params.routing[(int)ModSource::Op3][(int)dest] * op3level; };
        auto getOp1FM = [&](ModDestination dest) { return params.routing[(int)ModSource::Op1][(int)dest] * op1fm; };
        auto getOp2FM = [&](ModDestination dest) { return params.routing[(int)ModSource::Op2][(int)dest] * op2fm; };
        auto getOp3FM = [&](ModDestination dest) { return params.routing[(int)ModSource::Op3][(int)dest] * op3fm; };

        auto getAll = [&](ModDestination dest) {
            float amount = 1;
            if (params.routing[(int)ModSource::Envelope1][(int)dest]) amount *= env1level;
            if (params.routing[(int)ModSource::Envelope2][(int)dest]) amount *= env2level;
            if (params.routing[(int)ModSource::LFO][(int)dest]) amount *= lfolevel;
            if (params.routing[(int)ModSource::Op1][(int)dest]) amount *= op1level;
            if (params.routing[(int)ModSource::Op2][(int)dest]) amount *= op2level;
            if (params.routing[(int)ModSource::Op3][(int)dest]) amount *= op3level;
            return amount;
        };

        oscillator[0].note(note + params.pitchBend * 24 - 12 + 24 * Math::Fast::clamp(
            getEnv1(ModDestination::Op1FM) +
            getEnv2(ModDestination::Op1FM) +
            getLfo(ModDestination::Op1FM), -1, 1));
        
        oscillator[1].note(note + params.pitchBend * 24 - 12 + 24 * Math::Fast::clamp(
            getEnv1(ModDestination::Op2FM) +
            getEnv2(ModDestination::Op2FM) +
            getLfo(ModDestination::Op2FM), -1, 1));
        
        oscillator[2].note(note + params.pitchBend * 24 - 12 + 24 * Math::Fast::clamp(
            getEnv1(ModDestination::Op3FM) +
            getEnv2(ModDestination::Op3FM) +
            getLfo(ModDestination::Op3FM), -1, 1));

        oscillator[0].fm(params.fm[0] * getAll(ModDestination::Op1Amount) * (
            getOp1FM(ModDestination::Op1FM) +
            getOp2FM(ModDestination::Op1FM) +
            getOp3FM(ModDestination::Op1FM) 
        ));
        
        oscillator[1].fm(params.fm[1] * getAll(ModDestination::Op2Amount) * (
            getOp1FM(ModDestination::Op2FM) +
            getOp2FM(ModDestination::Op2FM) +
            getOp3FM(ModDestination::Op2FM)
        ));
        
        oscillator[2].fm(params.fm[2] * getAll(ModDestination::Op3Amount) * (
            getOp1FM(ModDestination::Op3FM) +
            getOp2FM(ModDestination::Op3FM) +
            getOp3FM(ModDestination::Op3FM)
        ));

        if (getOp1(ModDestination::Op1Sync)) oscillator[0].hardSync(oscillator[0]);
        if (getOp2(ModDestination::Op1Sync)) oscillator[0].hardSync(oscillator[1]);
        if (getOp3(ModDestination::Op1Sync)) oscillator[0].hardSync(oscillator[2]);
        if (getOp1(ModDestination::Op2Sync)) oscillator[1].hardSync(oscillator[0]);
        if (getOp2(ModDestination::Op2Sync)) oscillator[1].hardSync(oscillator[1]);
        if (getOp3(ModDestination::Op2Sync)) oscillator[1].hardSync(oscillator[2]);
        if (getOp1(ModDestination::Op3Sync)) oscillator[2].hardSync(oscillator[0]);
        if (getOp2(ModDestination::Op3Sync)) oscillator[2].hardSync(oscillator[1]);
        if (getOp3(ModDestination::Op3Sync)) oscillator[2].hardSync(oscillator[2]);

        filter.frequencyModulation =
            getOp1(ModDestination::FilterFreq) +
            getOp2(ModDestination::FilterFreq) +
            getOp3(ModDestination::FilterFreq) +
            getEnv1(ModDestination::FilterFreq) +
            getEnv2(ModDestination::FilterFreq) +
            getLfo(ModDestination::FilterFreq);
    }

    // ------------------------------------------------

    void MiniSynthFMVoice::process() {
        for (auto& env : envelope) env.process();

        doModulations();

        for (auto& osc : oscillator) osc.process();

        result = 
            params.outputOscillator[0] * oscillator[0].output * params.volume[0] +
            params.outputOscillator[1] * oscillator[1].output * params.volume[1] +
            params.outputOscillator[2] * oscillator[2].output * params.volume[2];

        filter.input = result;

        filter.process();
        result = filter.output * envelope[0].output * params.envelopeLevel[0];
    }

    // ------------------------------------------------

}

// ------------------------------------------------
