#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Processing/Processor.hpp"
#include "Kaixo/Core/Processing/Voice.hpp"
#include "Kaixo/Core/Processing/Module.hpp"
#include "Kaixo/Core/Processing/Filter.hpp"
#include "Kaixo/Core/Processing/Random.hpp"

// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Controller.hpp"
#include "Kaixo/MiniSynthFM/Processing/ADSREnvelope.hpp"
#include "Kaixo/MiniSynthFM/Processing/FMOscillator.hpp"
#include "Kaixo/MiniSynthFM/Processing/CustomFilter.hpp"
#include "Kaixo/MiniSynthFM/Processing/Lfo.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------
    
    struct VoiceParameters : public ModuleContainer {

        // ------------------------------------------------
        
        VoiceParameters();

        // ------------------------------------------------

        LfoParameters lfo[Lfos];
        FilterParameters filter;
        ADSREnvelopeParameters envelope[Envelopes];
        FMOscillatorParameters oscillator[Oscillators];
        float fm[Oscillators]{};
        float volume[Oscillators]{};
        float envelopeLevel[Envelopes]{};
        float lfoLevel[Lfos]{};
        bool outputOscillator[Oscillators]{};

        float pitchBend = 0;
        float modWheel = 0;
        float modWheelAmount = 0;
        float velocityAmount = 0;
        float randomAmount = 0;

        // ------------------------------------------------

        Random random{};

        // ------------------------------------------------

        Quality quality = Quality::Normal;

        // ------------------------------------------------
        
        bool routing[(int)ModDestination::Amount][(int)ModSource::Amount]{};

        void resetRouting();

        // ------------------------------------------------
        
        std::size_t oversample() const;

        // ------------------------------------------------

    };

    // ------------------------------------------------
    
    class MiniSynthFMVoice : public Voice {
    public:

        // ------------------------------------------------
        
        VoiceParameters& params;

        // ------------------------------------------------

        MiniSynthFMVoice(VoiceParameters& p);

        // ------------------------------------------------
        
        float result = 0;

        // ------------------------------------------------

        void trigger() override;
        void release() override;

        // ------------------------------------------------

        void doModulations();

        void process() override;

        // ------------------------------------------------

        FMOscillator oscillator[Oscillators]{ params.oscillator[0], params.oscillator[1], params.oscillator[2] };
        ADSREnvelope envelope[Envelopes]{ params.envelope[0], params.envelope[1], params.envelope[2] };
        Lfo lfo[Lfos]{ params.lfo[0] };
        CustomFilter filter{ params.filter };

        // ------------------------------------------------
        
        float randomValue = 0;

        // ------------------------------------------------
        
        float previousLfoLevel = 0;

        // ------------------------------------------------

    };

    // ------------------------------------------------
    
    class SimdMiniSynthFMVoice : public ModuleContainer {
    public:

        // ------------------------------------------------

        VoiceParameters& params;

        // ------------------------------------------------

        SimdMiniSynthFMVoice(VoiceParameters& p)
            : params(p)
        {
            for (auto& env : envelope) registerModule(env);
            for (auto& osc : oscillator) registerModule(osc);
            for (auto& lfo : lfo) registerModule(lfo);
            registerModule(filter);
        }

        // ------------------------------------------------

        void trigger(std::size_t i, Note n, float vel) {
            velocity[i] = vel;
            note[i] = n;
            randomValue[i] = params.random.next();
            for (auto& osc : oscillator) osc.trigger(i);
            for (auto& env : envelope) env.trigger(i);
            for (auto& lfo : lfo) lfo.trigger(i);
        }

        // ------------------------------------------------

        void release(std::size_t i) {
            for (auto& env : envelope) env.release(i);
        }

        // ------------------------------------------------
        
        template<class SimdType>
        void doModulations() {
            constexpr std::size_t Count = sizeof(SimdType) / sizeof(float);
            for (std::size_t i = 0; i < Voices; i += Count) {
                auto _velocity = Kaixo::at<SimdType>(velocity, i);
                auto _random = Kaixo::at<SimdType>(randomValue, i);
                auto _note = Kaixo::at<SimdType>(note, i);

                auto velocitylevel = _velocity * params.velocityAmount;
                auto modwheellevel = params.modWheel * params.modWheelAmount;
                auto randomlevel = _random * params.randomAmount;

                auto env1level = Kaixo::at<SimdType>(envelope[0].output, i) * params.envelopeLevel[0];
                auto env2level = Kaixo::at<SimdType>(envelope[1].output, i) * params.envelopeLevel[1];
                auto env3level = Kaixo::at<SimdType>(envelope[2].output, i) * params.envelopeLevel[2];

                auto op1level = Kaixo::at<SimdType>(oscillator[0].output[0], i) * params.volume[0];
                auto op2level = Kaixo::at<SimdType>(oscillator[1].output[0], i) * params.volume[1];
                auto op3level = Kaixo::at<SimdType>(oscillator[2].output[0], i) * params.volume[2];

                auto op1fm = Kaixo::at<SimdType>(oscillator[0].fmOutput[0], i) * params.volume[0];
                auto op2fm = Kaixo::at<SimdType>(oscillator[1].fmOutput[0], i) * params.volume[1];
                auto op3fm = Kaixo::at<SimdType>(oscillator[2].fmOutput[0], i) * params.volume[2];

                auto lfolevel = Kaixo::at<SimdType>(previousLfoLevel, i);

                auto getAllM = [&](ModDestination dest) {
                    SimdType amount = 1.f;
                    if (params.routing[(int)dest][(int)ModSource::ModWheel])  amount = amount * modwheellevel;
                    if (params.routing[(int)dest][(int)ModSource::Random])    amount = amount * randomlevel;
                    if (params.routing[(int)dest][(int)ModSource::Velocity])  amount = amount * velocitylevel;
                    if (params.routing[(int)dest][(int)ModSource::Envelope1]) amount = amount * env1level;
                    if (params.routing[(int)dest][(int)ModSource::Envelope2]) amount = amount * env2level;
                    if (params.routing[(int)dest][(int)ModSource::Envelope3]) amount = amount * env3level;
                    if (params.routing[(int)dest][(int)ModSource::LFO])       amount = amount * lfolevel;
                    if (params.routing[(int)dest][(int)ModSource::Op1])       amount = amount * op1level;
                    if (params.routing[(int)dest][(int)ModSource::Op2])       amount = amount * op2level;
                    if (params.routing[(int)dest][(int)ModSource::Op3])       amount = amount * op3level;
                    return amount;
                };

                // Assign new value
                lfolevel = Kaixo::at<SimdType>(lfo[0].output, i) * params.lfoLevel[0] * getAllM(ModDestination::LfoDepth);
                Kaixo::store<SimdType>(previousLfoLevel + i, lfolevel); // Store for recursive

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
                    return getVel(dest)  +
                           getMW(dest)   + getRand(dest) + getOp1(dest) +
                           getOp2(dest)  + getOp3(dest)  + getEnv1(dest) +
                           getEnv2(dest) + getEnv3(dest) + getLfo(dest);
                };

                auto getAllANoOp = [&](ModDestination dest) {
                    return getVel(dest)  +
                           getMW(dest)   + getRand(dest) + getEnv1(dest) +
                           getEnv2(dest) + getEnv3(dest) + getLfo(dest);
                };

                auto getAllOpFM = [&](ModDestination dest) {
                    return getOp1FM(dest) + getOp2FM(dest) + getOp3FM(dest);
                };

                auto fm1 = params.fm[0] * getAllM(ModDestination::Op1Amount);
                auto fm2 = params.fm[1] * getAllM(ModDestination::Op2Amount);
                auto fm3 = params.fm[2] * getAllM(ModDestination::Op3Amount);

                Kaixo::store<SimdType>(filter.note + i, _note + params.pitchBend * 24 - 12);

                oscillator[0].note<SimdType>(i, _note + params.pitchBend * 24 - 12 + fm1 * 24 * getAllANoOp(ModDestination::Op1FM));
                oscillator[1].note<SimdType>(i, _note + params.pitchBend * 24 - 12 + fm2 * 24 * getAllANoOp(ModDestination::Op2FM));
                oscillator[2].note<SimdType>(i, _note + params.pitchBend * 24 - 12 + fm3 * 24 * getAllANoOp(ModDestination::Op3FM));

                for (std::size_t j = 0; j < params.oversample(); ++j) {
                    op1fm = Kaixo::at<SimdType>(oscillator[0].fmOutput[j], i) * params.volume[0];
                    op2fm = Kaixo::at<SimdType>(oscillator[1].fmOutput[j], i) * params.volume[1];
                    op3fm = Kaixo::at<SimdType>(oscillator[2].fmOutput[j], i) * params.volume[2];

                    oscillator[0].fm<SimdType>(i, 4 * fm1 * (getAllOpFM(ModDestination::Op1FM)), j);
                    oscillator[1].fm<SimdType>(i, 4 * fm2 * (getAllOpFM(ModDestination::Op2FM)), j);
                    oscillator[2].fm<SimdType>(i, 4 * fm3 * (getAllOpFM(ModDestination::Op3FM)), j);
                }

                oscillator[0].hardSync<SimdType>(i, params.routing[(int)ModDestination::Op1Sync][(int)ModSource::Op1], oscillator[0]);
                oscillator[0].hardSync<SimdType>(i, params.routing[(int)ModDestination::Op2Sync][(int)ModSource::Op2], oscillator[1]);
                oscillator[0].hardSync<SimdType>(i, params.routing[(int)ModDestination::Op1Sync][(int)ModSource::Op3], oscillator[2]);
                oscillator[1].hardSync<SimdType>(i, params.routing[(int)ModDestination::Op2Sync][(int)ModSource::Op1], oscillator[0]);
                oscillator[1].hardSync<SimdType>(i, params.routing[(int)ModDestination::Op2Sync][(int)ModSource::Op2], oscillator[1]);
                oscillator[1].hardSync<SimdType>(i, params.routing[(int)ModDestination::Op2Sync][(int)ModSource::Op3], oscillator[2]);
                oscillator[2].hardSync<SimdType>(i, params.routing[(int)ModDestination::Op3Sync][(int)ModSource::Op1], oscillator[0]);
                oscillator[2].hardSync<SimdType>(i, params.routing[(int)ModDestination::Op3Sync][(int)ModSource::Op2], oscillator[1]);
                oscillator[2].hardSync<SimdType>(i, params.routing[(int)ModDestination::Op3Sync][(int)ModSource::Op3], oscillator[2]);

                Kaixo::store<SimdType>(filter.frequencyModulation + i, getAllA(ModDestination::FilterFreq));
            }
        }

        void process() {
            switch (simd_path::path) {
            case simd_path::s512: 
            case simd_path::s256: return processImpl<simd<float, 256>>();
            case simd_path::s128: return processImpl<simd<float, 128>>();
            case simd_path::s0:   return processImpl<float>();
            }
        }

        template<class SimdType>
        void processImpl() {
            constexpr std::size_t Count = sizeof(SimdType) / sizeof(float);

            for (auto& lfo : lfo) lfo.process<SimdType>();
            for (auto& env : envelope) env.process<SimdType>();

            this->doModulations<SimdType>();

            for (auto& osc : oscillator) osc.process<SimdType>();

            for (std::size_t i = 0; i < Voices; i += Count) {
                for (std::size_t j = 0; j < params.oversample(); ++j) {
                    SimdType filterInput = 
                        params.outputOscillator[0] * Kaixo::at<SimdType>(oscillator[0].output[j], i) * params.volume[0] +
                        params.outputOscillator[1] * Kaixo::at<SimdType>(oscillator[1].output[j], i) * params.volume[1] +
                        params.outputOscillator[2] * Kaixo::at<SimdType>(oscillator[2].output[j], i) * params.volume[2];
                    Kaixo::store<SimdType>(filter.input[j] + i, filterInput);
                }
            }

            filter.process<SimdType>();

            output = 0;
            for (std::size_t i = 0; i < Voices; i += Count) {
                output += Kaixo::sum<SimdType>(
                    Kaixo::at<SimdType>(filter.output, i) * 
                    Kaixo::at<SimdType>(envelope[2].output, i) * 
                    params.envelopeLevel[2]);
            }
        }

        // ------------------------------------------------
        
        SimdADSREnvelope envelope[Envelopes]{ params.envelope[0], params.envelope[1], params.envelope[2] };
        SimdFMOscillator oscillator[Oscillators]{ params.oscillator[0], params.oscillator[1], params.oscillator[2] };
        SimdCustomFilter filter{ params.filter };
        SimdLfo lfo[Lfos]{ params.lfo[0] };

        // ------------------------------------------------
        
        float previousLfoLevel[Voices]{};
        float velocity[Voices]{};
        float note[Voices]{};
        float randomValue[Voices]{};

        // ------------------------------------------------
        
        float output{};

        // ------------------------------------------------

    };

    // ------------------------------------------------

    class SimdVoice : public Voice {
    public:

        // ------------------------------------------------

        SimdMiniSynthFMVoice& voice;
        std::size_t index = 0;

        // ------------------------------------------------

        SimdVoice(std::pair<std::reference_wrapper<SimdMiniSynthFMVoice>, std::size_t> o)
            : voice(o.first.get()), index(o.second)
        {}

        // ------------------------------------------------

        void trigger() override {
            voice.trigger(index, note, velocity);
        }
        
        void release() override {
            voice.release(index);
        }

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------
