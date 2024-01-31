
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

    std::size_t VoiceParameters::oversample() const {
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

    MiniSynthFMVoice::MiniSynthFMVoice(VoiceParameters& p) : params(p) {
        for (auto& lfo : lfo) registerModule(lfo);
        for (auto& osc : oscillator) registerModule(osc);
        for (auto& env : envelope) registerModule(env);
        registerModule(filter);
    }

    // ------------------------------------------------

    void MiniSynthFMVoice::trigger() {
        randomValue = random.next() * 2 - 1;
        for (auto& osc : oscillator) osc.trigger();
        for (auto& env : envelope) env.trigger();
        for (auto& lfo : lfo) lfo.trigger();
    }

    void MiniSynthFMVoice::release() {
        for (auto& env : envelope) env.release();
    }

    // ------------------------------------------------
    
    void MiniSynthFMVoice::doModulations() {
        auto notelevel = currentNote() / 60.f - 1.;
        auto velocitylevel = velocity;
        auto modwheellevel = params.modWheel;
        auto pitchbendlevel = params.pitchBend;
        auto randomlevel = randomValue;

        auto env1level = envelope[0].output * params.envelopeLevel[0];
        auto env2level = envelope[1].output * params.envelopeLevel[1];
        auto env3level = envelope[2].output * params.envelopeLevel[2];
        
        auto op1level = oscillator[0].output[0] * params.volume[0];
        auto op2level = oscillator[1].output[0] * params.volume[1];
        auto op3level = oscillator[2].output[0] * params.volume[2];
        
        auto op1fm = oscillator[0].fmOutput[0] * params.volume[0];
        auto op2fm = oscillator[1].fmOutput[0] * params.volume[1];
        auto op3fm = oscillator[2].fmOutput[0] * params.volume[2];

        auto lfolevel = previousLfoLevel; 

        auto getAllM = [&](ModDestination dest) {
            float amount = 1;
            if (params.routing[(int)dest][(int)ModSource::Note]) amount *= notelevel;
            if (params.routing[(int)dest][(int)ModSource::PitchBend]) amount *= pitchbendlevel;
            if (params.routing[(int)dest][(int)ModSource::ModWheel]) amount *= modwheellevel;
            if (params.routing[(int)dest][(int)ModSource::Random]) amount *= randomlevel;
            if (params.routing[(int)dest][(int)ModSource::Velocity]) amount *= velocitylevel;
            if (params.routing[(int)dest][(int)ModSource::Envelope1]) amount *= env1level;
            if (params.routing[(int)dest][(int)ModSource::Envelope2]) amount *= env2level;
            if (params.routing[(int)dest][(int)ModSource::Envelope3]) amount *= env3level;
            if (params.routing[(int)dest][(int)ModSource::LFO]) amount *= lfolevel;
            if (params.routing[(int)dest][(int)ModSource::Op1]) amount *= op1level;
            if (params.routing[(int)dest][(int)ModSource::Op2]) amount *= op2level;
            if (params.routing[(int)dest][(int)ModSource::Op3]) amount *= op3level;
            return amount;
        };

        // Assign new value
        lfolevel = lfo[0].output * params.lfoLevel[0] * getAllM(ModDestination::LfoDepth);
        previousLfoLevel = lfolevel; // Store for recursive

        auto getNote  = [&](ModDestination dest) { return params.routing[(int)dest][(int)ModSource::Note] * notelevel; };
        auto getPB    = [&](ModDestination dest) { return params.routing[(int)dest][(int)ModSource::PitchBend] * pitchbendlevel; };
        auto getMW    = [&](ModDestination dest) { return params.routing[(int)dest][(int)ModSource::ModWheel] * modwheellevel; };
        auto getRand  = [&](ModDestination dest) { return params.routing[(int)dest][(int)ModSource::Random] * randomlevel; };
        auto getVel   = [&](ModDestination dest) { return params.routing[(int)dest][(int)ModSource::Velocity] * velocitylevel; };
        auto getEnv1  = [&](ModDestination dest) { return params.routing[(int)dest][(int)ModSource::Envelope1] * env1level; };
        auto getEnv2  = [&](ModDestination dest) { return params.routing[(int)dest][(int)ModSource::Envelope2] * env2level; };
        auto getEnv3  = [&](ModDestination dest) { return params.routing[(int)dest][(int)ModSource::Envelope3] * env3level; };
        auto getLfo   = [&](ModDestination dest) { return params.routing[(int)dest][(int)ModSource::LFO] * lfolevel; };
        auto getOp1   = [&](ModDestination dest) { return params.routing[(int)dest][(int)ModSource::Op1] * op1level; };
        auto getOp2   = [&](ModDestination dest) { return params.routing[(int)dest][(int)ModSource::Op2] * op2level; };
        auto getOp3   = [&](ModDestination dest) { return params.routing[(int)dest][(int)ModSource::Op3] * op3level; };
        auto getOp1FM = [&](ModDestination dest) { return params.routing[(int)dest][(int)ModSource::Op1] * op1fm; };
        auto getOp2FM = [&](ModDestination dest) { return params.routing[(int)dest][(int)ModSource::Op2] * op2fm; };
        auto getOp3FM = [&](ModDestination dest) { return params.routing[(int)dest][(int)ModSource::Op3] * op3fm; };

        auto getAllA = [&](ModDestination dest) {
            return getNote(dest) + getVel(dest)  + getPB(dest) + 
                   getMW(dest)   + getRand(dest) + getOp1(dest) + 
                   getOp2(dest)  + getOp3(dest)  + getEnv1(dest) + 
                   getEnv2(dest) + getEnv3(dest) + getLfo(dest);
        };
        
        auto getAllANoOp = [&](ModDestination dest) {
            return getNote(dest) + getVel(dest)  + getPB(dest) + 
                   getMW(dest)   + getRand(dest) + getEnv1(dest) + 
                   getEnv2(dest) + getEnv3(dest) + getLfo(dest);
        };
        
        auto getAllOpFM = [&](ModDestination dest) {
            return getOp1FM(dest) + getOp2FM(dest) + getOp3FM(dest);
        };

        auto fm1 = params.fm[0] * getAllM(ModDestination::Op1Amount);
        auto fm2 = params.fm[1] * getAllM(ModDestination::Op2Amount);
        auto fm3 = params.fm[2] * getAllM(ModDestination::Op3Amount);

        filter.note = note + params.pitchBend * 24 - 12;

        oscillator[0].note(note + params.pitchBend * 24 - 12 + fm1 * 24 * getAllANoOp(ModDestination::Op1FM));
        oscillator[1].note(note + params.pitchBend * 24 - 12 + fm2 * 24 * getAllANoOp(ModDestination::Op2FM));
        oscillator[2].note(note + params.pitchBend * 24 - 12 + fm3 * 24 * getAllANoOp(ModDestination::Op3FM));

        for (std::size_t i = 0; i < params.oversample(); ++i) {
            op1fm = oscillator[0].fmOutput[i] * params.volume[0];
            op2fm = oscillator[1].fmOutput[i] * params.volume[1];
            op3fm = oscillator[2].fmOutput[i] * params.volume[2];

            oscillator[0].fm(4 * fm1 * (getAllOpFM(ModDestination::Op1FM)), i);
            oscillator[1].fm(4 * fm2 * (getAllOpFM(ModDestination::Op2FM)), i);
            oscillator[2].fm(4 * fm3 * (getAllOpFM(ModDestination::Op3FM)), i);
        }

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

        for (std::size_t i = 0; i < params.oversample(); ++i) {
            filter.input[i] =
                params.outputOscillator[0] * oscillator[0].output[i] * params.volume[0] +
                params.outputOscillator[1] * oscillator[1].output[i] * params.volume[1] +
                params.outputOscillator[2] * oscillator[2].output[i] * params.volume[2];
        }

        filter.process();
        result = filter.output * envelope[2].output * params.envelopeLevel[2];
    }

    // ------------------------------------------------

}

// ------------------------------------------------
