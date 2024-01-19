#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Processing/Module.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------
    
    struct ADSREnvelopeParameters : public Module {

        // ------------------------------------------------
        
        bool loop = false;
        bool trigger = false; // Trigger mode (attack -> release, no sustain)

        // ------------------------------------------------

        void attack(float millis);
        void decay(float millis);
        void release(float millis);
        void sustain(float level);

        // ------------------------------------------------

        void prepare(double sampleRate, std::size_t maxBufferSize) override;

        // ------------------------------------------------

    private:
        float m_AttackMillis = 10;
        float m_DecayMillis = 10;
        float m_ReleaseMillis = 10;

        // ------------------------------------------------

        float m_Attack = 0;
        float m_Decay = 0;
        float m_Sustain = 1;
        float m_Release = 0;

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
