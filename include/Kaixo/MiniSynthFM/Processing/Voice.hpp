#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Processing/Processor.hpp"
#include "Kaixo/Core/Processing/Voice.hpp"
#include "Kaixo/Core/Processing/Module.hpp"

// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Controller.hpp"
#include "Kaixo/MiniSynthFM/Processing/ADSREnvelope.hpp"
#include "Kaixo/MiniSynthFM/Processing/FMOscillator.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------
    
    struct VoiceParameters : public ModuleContainer {

        // ------------------------------------------------
        
        VoiceParameters() {
            for (auto& env : envelope) registerModule(env);
        }

        // ------------------------------------------------

        ADSREnvelopeParameters envelope[Envelopes];

        // ------------------------------------------------

    };

    // ------------------------------------------------

    class MiniSynthFMVoice : public Voice {
    public:

        // ------------------------------------------------
        
        VoiceParameters& params;

        // ------------------------------------------------

        MiniSynthFMVoice(VoiceParameters& p) : params(p) {
            for (auto& osc : oscillator) registerModule(osc);
            for (auto& env : envelope) registerModule(env);
        }

        // ------------------------------------------------

        Note note = 64;

        // ------------------------------------------------

        void trigger() override {
            for (auto& osc : oscillator) osc.trigger();
            for (auto& env : envelope) env.trigger();
        }

        void release() override {
            for (auto& env : envelope) env.release();
        }

        // ------------------------------------------------

        void process() override {
            for (auto& out : output) {

                for (auto& osc : oscillator) {
                    osc.note(note);
                    osc.process();
                }

                for (auto& env : envelope) env.process();

                oscillator[0].fm(oscillator[1].output);
                oscillator[1].fm(oscillator[2].output);

                out = oscillator[0].output * envelope[0].output;
            }
        }

        // ------------------------------------------------

        FMOscillator oscillator[Oscillators];
        ADSREnvelope envelope[Envelopes]{ params.envelope[0], params.envelope[1], params.envelope[2] };

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------
