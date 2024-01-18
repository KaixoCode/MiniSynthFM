
// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Processing/Voice.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {
    
    // ------------------------------------------------

    VoiceParameters::VoiceParameters() {
        for (auto& lfo : lfo) registerModule(lfo);
        for (auto& env : envelope) registerModule(env);
        for (auto& osc : oscillator) registerModule(osc);
        registerModule(filter);
    }

    // ------------------------------------------------

    MiniSynthFMVoice::MiniSynthFMVoice(VoiceParameters& p) : params(p) {
        for (auto& lfo : lfo) registerModule(lfo);
        for (auto& osc : oscillator) registerModule(osc);
        for (auto& env : envelope) registerModule(env);
        registerModule(filter);
    }

    // ------------------------------------------------

    void MiniSynthFMVoice::trigger() {
        for (auto& osc : oscillator) osc.trigger();
        for (auto& env : envelope) env.trigger();
        for (auto& lfo : envelope) lfo.trigger();
    }

    void MiniSynthFMVoice::release() {
        for (auto& env : envelope) env.release();
    }

    // ------------------------------------------------
    
    void MiniSynthFMVoice::doModulations() {
        auto env1level = envelope[0].output * params.envelopeLevel[0];
        auto env2level = envelope[1].output * params.envelopeLevel[1];
        auto env3level = envelope[2].output * params.envelopeLevel[2];
        
        auto lfolevel = lfo[0].output * params.lfoLevel[0];

        auto op1level = oscillator[0].output * params.volume[0];
        auto op2level = oscillator[1].output * params.volume[1];
        auto op3level = oscillator[2].output * params.volume[2];
        
        auto op1fm = oscillator[0].fmOutput * params.volume[0];
        auto op2fm = oscillator[1].fmOutput * params.volume[1];
        auto op3fm = oscillator[2].fmOutput * params.volume[2];
        
        auto getEnv1  = [&](ModDestination dest) { return params.routing[(int)ModSource::Envelope1][(int)dest] * env1level; };
        auto getEnv2  = [&](ModDestination dest) { return params.routing[(int)ModSource::Envelope2][(int)dest] * env2level; };
        auto getEnv3  = [&](ModDestination dest) { return params.routing[(int)ModSource::Envelope3][(int)dest] * env3level; };
        auto getLfo   = [&](ModDestination dest) { return params.routing[(int)ModSource::LFO][(int)dest] * lfolevel; };
        auto getOp1   = [&](ModDestination dest) { return params.routing[(int)ModSource::Op1][(int)dest] * op1level; };
        auto getOp2   = [&](ModDestination dest) { return params.routing[(int)ModSource::Op2][(int)dest] * op2level; };
        auto getOp3   = [&](ModDestination dest) { return params.routing[(int)ModSource::Op3][(int)dest] * op3level; };
        auto getOp1FM = [&](ModDestination dest) { return params.routing[(int)ModSource::Op1][(int)dest] * op1fm; };
        auto getOp2FM = [&](ModDestination dest) { return params.routing[(int)ModSource::Op2][(int)dest] * op2fm; };
        auto getOp3FM = [&](ModDestination dest) { return params.routing[(int)ModSource::Op3][(int)dest] * op3fm; };

        auto getAllM = [&](ModDestination dest) {
            float amount = 1;
            if (params.routing[(int)ModSource::Envelope1][(int)dest]) amount *= env1level;
            if (params.routing[(int)ModSource::Envelope2][(int)dest]) amount *= env2level;
            if (params.routing[(int)ModSource::Envelope3][(int)dest]) amount *= env3level;
            if (params.routing[(int)ModSource::LFO][(int)dest]) amount *= lfolevel;
            if (params.routing[(int)ModSource::Op1][(int)dest]) amount *= op1level;
            if (params.routing[(int)ModSource::Op2][(int)dest]) amount *= op2level;
            if (params.routing[(int)ModSource::Op3][(int)dest]) amount *= op3level;
            return amount;
        };
        
        auto getAllA = [&](ModDestination dest) {
            return getOp1(dest) + getOp2(dest) + getOp3(dest) +
                   getEnv1(dest) + getEnv2(dest) + getEnv3(dest) +
                   getLfo(dest);
        };
        
        auto getAllANoOp = [&](ModDestination dest) {
            return getEnv1(dest) + getEnv2(dest) + getEnv3(dest) + getLfo(dest);
        };
        
        auto getAllOpFM = [&](ModDestination dest) {
            return getOp1FM(dest) + getOp2FM(dest) + getOp3FM(dest);
        };

        oscillator[0].note(note + params.pitchBend * 24 - 12 + 24 * Math::Fast::clamp(getAllANoOp(ModDestination::Op1FM), -1, 1));
        oscillator[1].note(note + params.pitchBend * 24 - 12 + 24 * Math::Fast::clamp(getAllANoOp(ModDestination::Op2FM), -1, 1));
        oscillator[2].note(note + params.pitchBend * 24 - 12 + 24 * Math::Fast::clamp(getAllANoOp(ModDestination::Op3FM), -1, 1));

        oscillator[0].fm(params.fm[0] * getAllM(ModDestination::Op1Amount) * (getAllOpFM(ModDestination::Op1FM)));
        oscillator[1].fm(params.fm[1] * getAllM(ModDestination::Op2Amount) * (getAllOpFM(ModDestination::Op2FM)));
        oscillator[2].fm(params.fm[2] * getAllM(ModDestination::Op3Amount) * (getAllOpFM(ModDestination::Op3FM)));

        if (getOp1(ModDestination::Op1Sync)) oscillator[0].hardSync(oscillator[0]);
        if (getOp2(ModDestination::Op1Sync)) oscillator[0].hardSync(oscillator[1]);
        if (getOp3(ModDestination::Op1Sync)) oscillator[0].hardSync(oscillator[2]);
        if (getOp1(ModDestination::Op2Sync)) oscillator[1].hardSync(oscillator[0]);
        if (getOp2(ModDestination::Op2Sync)) oscillator[1].hardSync(oscillator[1]);
        if (getOp3(ModDestination::Op2Sync)) oscillator[1].hardSync(oscillator[2]);
        if (getOp1(ModDestination::Op3Sync)) oscillator[2].hardSync(oscillator[0]);
        if (getOp2(ModDestination::Op3Sync)) oscillator[2].hardSync(oscillator[1]);
        if (getOp3(ModDestination::Op3Sync)) oscillator[2].hardSync(oscillator[2]);

        filter.frequencyModulation = getAllA(ModDestination::FilterFreq);
    }

    // ------------------------------------------------

    void MiniSynthFMVoice::process() {
        for (auto& lfo : lfo) lfo.process();
        for (auto& env : envelope) env.process();

        doModulations();

        for (auto& osc : oscillator) osc.process();

        result = 
            params.outputOscillator[0] * oscillator[0].output * params.volume[0] +
            params.outputOscillator[1] * oscillator[1].output * params.volume[1] +
            params.outputOscillator[2] * oscillator[2].output * params.volume[2];

        filter.input = result;

        filter.process();
        result = filter.output * envelope[2].output * params.envelopeLevel[2];
    }

    // ------------------------------------------------

}

// ------------------------------------------------
