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
        
        Quality quality = Quality::Normal;

        // ------------------------------------------------

        LfoParameters lfo[Lfos];
        FilterParameters filter{ quality };
        ADSREnvelopeParameters envelope[Envelopes];
        FMOscillatorParameters oscillator[Oscillators]{ quality, quality, quality };
        float fm[Oscillators]{};
        float volume[Oscillators]{};
        float envelopeLevel[Envelopes]{};
        float lfoLevel[Lfos]{};
        bool outputOscillator[Oscillators]{};

        float velToGain = 0.5;
        float glide = 0;
        float pitchBendRange = 0;
        float pitchBend = 0;
        float modWheel = 0;
        float modWheelAmount = 0;
        float velocityAmount = 0;
        float randomAmount = 0;

        // ------------------------------------------------
        
        bool routing[(int)ModDestination::Amount][(int)ModSource::Amount]{};

        void resetRouting();

        // ------------------------------------------------
        
    };

    // ------------------------------------------------
    
    class MiniSynthFMVoice : public ModuleContainer {
    public:

        // ------------------------------------------------

        VoiceParameters& params;

        // ------------------------------------------------

        MiniSynthFMVoice(VoiceParameters& p);

        // ------------------------------------------------

        float output{};

        // ------------------------------------------------
        
        bool active(std::size_t i) const;

        // ------------------------------------------------

        void trigger(std::size_t i, Note n, float vel, Note fromNote, bool stolen);
        void release(std::size_t i);

        // ------------------------------------------------
        
        template<class SimdType> KAIXO_INLINE void doModulations();
        template<class SimdType> KAIXO_INLINE void process();

        // ------------------------------------------------
        
        SimdADSREnvelope envelope[Envelopes]{ params.envelope[0], params.envelope[1], params.envelope[2] };
        FMOscillator oscillator[Oscillators]{ params.oscillator[0], params.oscillator[1], params.oscillator[2] };
        CustomFilter filter{ params.filter };
        Lfo lfo[Lfos]{ params.lfo[0] };

        // ------------------------------------------------
        
        alignas(sizeof(float) * Voices) float previousLfoLevel[Voices]{};
        alignas(sizeof(float) * Voices) float velocity[Voices]{};
        alignas(sizeof(float) * Voices) float note[Voices]{};
        alignas(sizeof(float) * Voices) float randomValue[Voices]{};
        alignas(sizeof(float) * Voices) float currentNote[Voices]{};
        alignas(sizeof(float) * Voices) float pitchBend[Voices]{};

        bool isGliding[Voices]{};
        float deltaNote[Voices]{};

        // ------------------------------------------------

    };

    // ------------------------------------------------

    // Interfaces with the VoiceBank class and forwards calls 
    // to the MiniSynthFMVoice with the correct index
    class VoiceBankVoice : public Voice {
    public:

        // ------------------------------------------------

        struct Settings {
            MiniSynthFMVoice& voice;
            std::size_t index = 0;
        } settings;

        // ------------------------------------------------

        VoiceBankVoice(Settings s);

        // ------------------------------------------------
        
        bool active() const override;

        // ------------------------------------------------

        void trigger() override;
        void release() override;

        // ------------------------------------------------

        Note currentNote() const override;

        // ------------------------------------------------
        
        void notePitchBendMPE(double value) override;

        // ------------------------------------------------

    };

    // ------------------------------------------------
    
    template<class SimdType>
    KAIXO_INLINE void MiniSynthFMVoice::doModulations() {
        constexpr std::size_t Count = sizeof(SimdType) / sizeof(float);
        const auto oversampleAmount = oversampleForQuality(params.quality);

        float _pitchBend = params.pitchBend * 2 * params.pitchBendRange - params.pitchBendRange;

        for (std::size_t i = 0; i < Voices; i += Count) {
            auto _velocity = Kaixo::load<SimdType>(velocity, i);
            auto _random = Kaixo::load<SimdType>(randomValue, i);
            auto _note = Kaixo::load<SimdType>(currentNote, i);
            auto _mpePitchBend = Kaixo::load<SimdType>(pitchBend, i) * 96.f - 48.f;

            auto velocitylevel = _velocity * params.velocityAmount;
            auto modwheellevel = params.modWheel * params.modWheelAmount;
            auto randomlevel = _random * params.randomAmount;

            auto env1level = Kaixo::load<SimdType>(envelope[0].output, i) * params.envelopeLevel[0];
            auto env2level = Kaixo::load<SimdType>(envelope[1].output, i) * params.envelopeLevel[1];
            auto env3level = Kaixo::load<SimdType>(envelope[2].output, i) * params.envelopeLevel[2];

            auto op1level = Kaixo::load<SimdType>(oscillator[0].output[0], i) * params.volume[0];
            auto op2level = Kaixo::load<SimdType>(oscillator[1].output[0], i) * params.volume[1];
            auto op3level = Kaixo::load<SimdType>(oscillator[2].output[0], i) * params.volume[2];

            auto op1fm = Kaixo::load<SimdType>(oscillator[0].fmOutput[0], i) * params.volume[0];
            auto op2fm = Kaixo::load<SimdType>(oscillator[1].fmOutput[0], i) * params.volume[1];
            auto op3fm = Kaixo::load<SimdType>(oscillator[2].fmOutput[0], i) * params.volume[2];

            auto lfolevel = Kaixo::load<SimdType>(previousLfoLevel, i);

            auto getAllM = [&](ModDestination dest) {
                SimdType amount = 1.f;
                if (params.routing[(int)dest][(int)ModSource::ModWheel])  amount = amount * modwheellevel;
                if (params.routing[(int)dest][(int)ModSource::Random])    amount = amount * randomlevel;
                if (params.routing[(int)dest][(int)ModSource::Velocity])  amount = amount * velocitylevel;
                if (params.routing[(int)dest][(int)ModSource::Envelope1]) amount = amount * env1level;
                if (params.routing[(int)dest][(int)ModSource::Envelope2]) amount = amount * env2level;
                if (params.routing[(int)dest][(int)ModSource::Envelope3]) amount = amount * env3level;
                if (params.routing[(int)dest][(int)ModSource::LFO])       amount = amount * (lfolevel * 0.5 + 0.5);
                if (params.routing[(int)dest][(int)ModSource::Op1])       amount = amount * op1level;
                if (params.routing[(int)dest][(int)ModSource::Op2])       amount = amount * op2level;
                if (params.routing[(int)dest][(int)ModSource::Op3])       amount = amount * op3level;
                return amount;
            };

            // Assign new value
            lfolevel = Kaixo::load<SimdType>(lfo[0].output, i) * params.lfoLevel[0] * getAllM(ModDestination::LfoDepth);
            Kaixo::store<SimdType>(previousLfoLevel + i, lfolevel); // Store for recursive

            auto getMW     = [&](ModDestination dest) { return static_cast<float>(params.routing[(int)dest][(int)ModSource::ModWheel]) * modwheellevel; };
            auto getRand   = [&](ModDestination dest) { return static_cast<float>(params.routing[(int)dest][(int)ModSource::Random]) * randomlevel; };
            auto getVel    = [&](ModDestination dest) { return static_cast<float>(params.routing[(int)dest][(int)ModSource::Velocity]) * velocitylevel; };
            auto getEnv1   = [&](ModDestination dest) { return static_cast<float>(params.routing[(int)dest][(int)ModSource::Envelope1]) * env1level; };
            auto getEnv2   = [&](ModDestination dest) { return static_cast<float>(params.routing[(int)dest][(int)ModSource::Envelope2]) * env2level; };
            auto getEnv3   = [&](ModDestination dest) { return static_cast<float>(params.routing[(int)dest][(int)ModSource::Envelope3]) * env3level; };
            auto getLfo    = [&](ModDestination dest) { return static_cast<float>(params.routing[(int)dest][(int)ModSource::LFO]) * lfolevel; };
            auto getOp1    = [&](ModDestination dest) { return static_cast<float>(params.routing[(int)dest][(int)ModSource::Op1]) * op1level; };
            auto getOp2    = [&](ModDestination dest) { return static_cast<float>(params.routing[(int)dest][(int)ModSource::Op2]) * op2level; };
            auto getOp3    = [&](ModDestination dest) { return static_cast<float>(params.routing[(int)dest][(int)ModSource::Op3]) * op3level; };
            auto getOp1FM  = [&](ModDestination dest) { return static_cast<float>(params.routing[(int)dest][(int)ModSource::Op1]) * op1fm; };
            auto getOp2FM  = [&](ModDestination dest) { return static_cast<float>(params.routing[(int)dest][(int)ModSource::Op2]) * op2fm; };
            auto getOp3FM  = [&](ModDestination dest) { return static_cast<float>(params.routing[(int)dest][(int)ModSource::Op3]) * op3fm; };

            auto getAllA = [&](ModDestination dest) {
                return getVel(dest)  + getMW(dest)   + getRand(dest) +
                       getOp1(dest)  + getOp2(dest)  + getOp3(dest)  + 
                       getEnv1(dest) + getEnv2(dest) + getEnv3(dest) + 
                       getLfo(dest);
            };

            auto getAllANoOp = [&](ModDestination dest) {
                return getVel(dest)  + getMW(dest)   + getRand(dest) + 
                       getEnv1(dest) + getEnv2(dest) + getEnv3(dest) + 
                       getLfo(dest);
            };

            auto getAllOpFM = [&](ModDestination dest) {
                return getOp1FM(dest) + getOp2FM(dest) + getOp3FM(dest);
            };

            auto fm1 = params.fm[0] + getAllA(ModDestination::Op1Amount);
            auto fm2 = params.fm[1] + getAllA(ModDestination::Op2Amount);
            auto fm3 = params.fm[2] + getAllA(ModDestination::Op3Amount);

            Kaixo::store<SimdType>(filter.note + i, _note + _pitchBend + _mpePitchBend);

            oscillator[0].note<SimdType>(i, _note + _pitchBend + _mpePitchBend + fm1 * 24.f * getAllANoOp(ModDestination::Op1FM));
            oscillator[1].note<SimdType>(i, _note + _pitchBend + _mpePitchBend + fm2 * 24.f * getAllANoOp(ModDestination::Op2FM));
            oscillator[2].note<SimdType>(i, _note + _pitchBend + _mpePitchBend + fm3 * 24.f * getAllANoOp(ModDestination::Op3FM));

            bool doOp1RM = params.oscillator[0].modType() == ModType::Volume;
            bool doOp2RM = params.oscillator[1].modType() == ModType::Volume;
            bool doOp3RM = params.oscillator[2].modType() == ModType::Volume;
            bool doRingMod = doOp1RM || doOp2RM || doOp3RM;

            for (std::size_t j = 0; j < oversampleAmount; ++j) {
                op1fm = Kaixo::load<SimdType>(oscillator[0].fmOutput[j], i) * params.volume[0];
                op2fm = Kaixo::load<SimdType>(oscillator[1].fmOutput[j], i) * params.volume[1];
                op3fm = Kaixo::load<SimdType>(oscillator[2].fmOutput[j], i) * params.volume[2];

                oscillator[0].fm<SimdType>(i, 4.f * fm1 * getAllOpFM(ModDestination::Op1FM), j);
                oscillator[1].fm<SimdType>(i, 4.f * fm2 * getAllOpFM(ModDestination::Op2FM), j);
                oscillator[2].fm<SimdType>(i, 4.f * fm3 * getAllOpFM(ModDestination::Op3FM), j);
            
                if (doRingMod) {
                    op1level = Kaixo::load<SimdType>(oscillator[0].output[j], i) * params.volume[0];
                    op2level = Kaixo::load<SimdType>(oscillator[1].output[j], i) * params.volume[1];
                    op3level = Kaixo::load<SimdType>(oscillator[2].output[j], i) * params.volume[2];

                    if (doOp1RM) oscillator[0].ringMod<SimdType>(i, getAllM(ModDestination::Op1Mod), j);
                    if (doOp2RM) oscillator[1].ringMod<SimdType>(i, getAllM(ModDestination::Op2Mod), j);
                    if (doOp3RM) oscillator[2].ringMod<SimdType>(i, getAllM(ModDestination::Op3Mod), j);
                }
            }

            if (params.oscillator[0].modType() == ModType::Sync) {
                if (params.routing[(int)ModDestination::Op1Mod][(int)ModSource::Op1]) oscillator[0].hardSync<SimdType>(i, oscillator[0]);
                if (params.routing[(int)ModDestination::Op1Mod][(int)ModSource::Op2]) oscillator[0].hardSync<SimdType>(i, oscillator[1]);
                if (params.routing[(int)ModDestination::Op1Mod][(int)ModSource::Op3]) oscillator[0].hardSync<SimdType>(i, oscillator[2]);
            }

            if (params.oscillator[1].modType() == ModType::Sync) {
                if (params.routing[(int)ModDestination::Op2Mod][(int)ModSource::Op1]) oscillator[1].hardSync<SimdType>(i, oscillator[0]);
                if (params.routing[(int)ModDestination::Op2Mod][(int)ModSource::Op2]) oscillator[1].hardSync<SimdType>(i, oscillator[1]);
                if (params.routing[(int)ModDestination::Op2Mod][(int)ModSource::Op3]) oscillator[1].hardSync<SimdType>(i, oscillator[2]);
            }

            if (params.oscillator[2].modType() == ModType::Sync) {
                if (params.routing[(int)ModDestination::Op3Mod][(int)ModSource::Op1]) oscillator[2].hardSync<SimdType>(i, oscillator[0]);
                if (params.routing[(int)ModDestination::Op3Mod][(int)ModSource::Op2]) oscillator[2].hardSync<SimdType>(i, oscillator[1]);
                if (params.routing[(int)ModDestination::Op3Mod][(int)ModSource::Op3]) oscillator[2].hardSync<SimdType>(i, oscillator[2]);
            }

            Kaixo::store<SimdType>(filter.frequencyModulation + i, getAllA(ModDestination::FilterFreq));
        }
    }

    // ------------------------------------------------
    
    template<class SimdType>
    KAIXO_INLINE void MiniSynthFMVoice::process() {
        constexpr std::size_t Count = sizeof(SimdType) / sizeof(float);

        for (std::size_t i = 0; i < Voices; ++i) {
            if (isGliding[i]) {
                currentNote[i] += deltaNote[i];
                if (deltaNote[i] >= 0 && currentNote[i] >= note[i] ||
                    deltaNote[i] <= 0 && currentNote[i] <= note[i])
                {
                    currentNote[i] = note[i];
                    isGliding[i] = false;
                }
            }
        }

        for (auto& lfo : lfo) lfo.process<SimdType>();
        for (auto& env : envelope) env.process<SimdType>();

        this->doModulations<SimdType>();

        for (auto& osc : oscillator) osc.process<SimdType>();

        for (std::size_t i = 0; i < Voices; i += Count) {
            for (std::size_t j = 0; j < oversampleForQuality(params.quality); ++j) {
                SimdType filterInput =
                    static_cast<float>(params.outputOscillator[0]) * Kaixo::load<SimdType>(oscillator[0].output[j], i) * params.volume[0] +
                    static_cast<float>(params.outputOscillator[1]) * Kaixo::load<SimdType>(oscillator[1].output[j], i) * params.volume[1] +
                    static_cast<float>(params.outputOscillator[2]) * Kaixo::load<SimdType>(oscillator[2].output[j], i) * params.volume[2];
                Kaixo::store<SimdType>(filter.input[j] + i, filterInput);
            }
        }

        filter.process<SimdType>();

        output = 0;
        for (std::size_t i = 0; i < Voices; i += Count) {
            SimdType velocityGain = Kaixo::load<SimdType>(velocity, i) * params.velToGain + (1 - params.velToGain);
            output += Kaixo::sum<SimdType>(
                Kaixo::load<SimdType>(filter.output, i) *
                Kaixo::load<SimdType>(envelope[2].output, i) *
                params.envelopeLevel[2] * 
                velocityGain);
        }
    }

    // ------------------------------------------------

}

// ------------------------------------------------
