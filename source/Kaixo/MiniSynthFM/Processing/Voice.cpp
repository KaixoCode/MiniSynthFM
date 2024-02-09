
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

    void VoiceParameters::resetRouting() {
        std::memset(routing, 0, sizeof(routing));
    }

    // ------------------------------------------------

    MiniSynthFMVoice::MiniSynthFMVoice(VoiceParameters& p) 
        : params(p) 
    {
        for (auto& lfo : lfo) registerModule(lfo);
        for (auto& osc : oscillator) registerModule(osc);
        for (auto& env : envelope) registerModule(env);
        registerModule(filter);
    }

    // ------------------------------------------------

    bool MiniSynthFMVoice::active(std::size_t i) const {
        for (auto& env : envelope) {
            if (env.active(i)) return true;
        }
        return false;
    }

    // ------------------------------------------------

    void MiniSynthFMVoice::trigger(std::size_t i, Note n, float vel) {
        velocity[i] = vel;
        note[i] = n;
        randomValue[i] = params.random.next();
        for (auto& osc : oscillator) osc.trigger(i);
        for (auto& env : envelope) env.trigger(i);
        for (auto& lfo : lfo) lfo.trigger(i);
    }

    // ------------------------------------------------

    void MiniSynthFMVoice::release(std::size_t i) {
        for (auto& env : envelope) env.release(i);
    }

    // ------------------------------------------------
    
    VoiceBankVoice::VoiceBankVoice(Settings s)
        : settings(s)
    {}

    // ------------------------------------------------

    bool VoiceBankVoice::active() const {
        return settings.voice.active(settings.index);
    }

    // ------------------------------------------------

    void VoiceBankVoice::trigger() {
        settings.voice.trigger(settings.index, note, velocity);
    }

    void VoiceBankVoice::release() {
        settings.voice.release(settings.index);
    }
    // ------------------------------------------------

}

// ------------------------------------------------
