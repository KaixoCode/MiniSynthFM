#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Processing/Module.hpp"

// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Controller.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------
    
    struct ADSREnvelopeParameters : public Module {

        // ------------------------------------------------

        void attack(float millis);
        void decay(float millis);
        void release(float millis);
        void sustain(float level);

        // ------------------------------------------------

        void prepare(double sampleRate, std::size_t maxBufferSize) override;

        // ------------------------------------------------

    private:
        float m_AttackMillis;
        float m_DecayMillis;
        float m_SustainMillis;
        float m_ReleaseMillis;

        // ------------------------------------------------

        float m_Attack;
        float m_Decay;
        float m_Sustain;
        float m_Release;

        // ------------------------------------------------

        void updateAttack() { m_Attack = 0.001 * m_AttackMillis * sampleRate(); }
        void updateDecay() { m_Decay = 0.001 * m_DecayMillis * sampleRate(); }
        void updateRelease() { m_Release = 0.001 * m_ReleaseMillis * sampleRate(); }

        // ------------------------------------------------
        
        friend class ADSREnvelope;

        // ------------------------------------------------

    };

    // ------------------------------------------------

    class ADSREnvelope : public Module {
    public:

        // ------------------------------------------------

        enum class State { Idle, Attack, Decay, Sustain, Release, Amount };

        // ------------------------------------------------
        
        ADSREnvelopeParameters& params;
        
        // ------------------------------------------------

        ADSREnvelope(ADSREnvelopeParameters& p);

        // ------------------------------------------------

        float output = 0;

        // ------------------------------------------------

        bool idle() const { return m_State == State::Idle; }
        bool active() const override { return !idle(); }

        // ------------------------------------------------

        void gate(bool gate);
        void release();
        void trigger();

        // ------------------------------------------------

        void process() override;
        void reset() override;

        // ------------------------------------------------

    private:
        State m_State = State::Idle;

        // ------------------------------------------------

        float m_Phase = 0;
        float m_ReleaseValue = 0;
        float m_AttackValue = 0;

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------
