
// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Processing/Voice.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    VoiceParameters::VoiceParameters() {
        for (auto& env : envelope) registerModule(env);
        for (auto& osc : oscillator) registerModule(osc);
    }

    // ------------------------------------------------

    MiniSynthFMVoice::MiniSynthFMVoice(VoiceParameters& p) : params(p) {
        for (auto& osc : oscillator) registerModule(osc);
        for (auto& env : envelope) registerModule(env);
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
            osc.note(note);
            osc.process();
        }

        for (auto& env : envelope) env.process();

        oscillator[0].fm(oscillator[1].output * params.fm[1]);
        oscillator[1].fm(oscillator[2].output * params.fm[2]);

        result = oscillator[0].output * envelope[0].output;
    }

    // ------------------------------------------------

}

// ------------------------------------------------
