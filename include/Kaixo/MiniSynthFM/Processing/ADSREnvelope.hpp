#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Processing/Module.hpp"

// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Controller.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    class ADSREnvelope : public Module {
    public:

        // ------------------------------------------------

        enum class State { Idle, Attack, Decay, Sustain, Release, Amount };

        // ------------------------------------------------

        float output = 0;

        // ------------------------------------------------

        void attack(float millis) {
            if (m_AttackMillis != millis) {
                m_AttackMillis = millis;
                updateAttack();
            }
        }

        void decay(float millis) {
            if (m_DecayMillis != millis) {
                m_DecayMillis = millis;
                updateDecay();
            }
        }

        void release(float millis) {
            if (m_ReleaseMillis != millis) {
                m_ReleaseMillis = millis;
                updateRelease();
            }
        }

        void sustain(float level) { m_Sustain = level; }

        // ------------------------------------------------

        bool idle() const { return m_State == State::Idle; }
        bool active() const override { return !idle(); }

        // ------------------------------------------------

        void gate(bool gate) {
            if (gate) trigger();
            else release();
        }

        void release() {
            if (m_State != State::Idle) {
                m_State = State::Release;
                m_Phase = 0;
                m_ReleaseValue = output;
            }
        }

        void trigger() {
            m_State = State::Attack;
            m_Phase = 0;
            m_AttackValue = output;
        }

        // ------------------------------------------------

        void process() override {
            switch (m_State) {
            case State::Idle: output = 0; break;
            case State::Sustain: output = m_Sustain; break;
            case State::Attack:
                m_Phase += 1 / m_Attack;
                if (m_Phase >= 1.0) {
                    output = 1;
                    m_Phase = 0;
                    m_State = State::Decay;
                }
                else {
                    output = m_AttackValue + (1 - m_AttackValue) * m_Phase;
                }
                break;
            case State::Decay:
                m_Phase += 1 / m_Decay;
                if (m_Phase >= 1.0) {
                    output = m_Sustain;
                    m_Phase = 0;
                    m_State = State::Sustain;
                }
                else {
                    output = 1 - (1 - m_Sustain) * m_Phase;
                }
                break;
            case State::Release:
                m_Phase += 1 / m_Release;
                if (m_Phase >= 1.0) {
                    output = 0;
                    m_Phase = 0;
                    m_State = State::Idle;
                }
                else {
                    output = m_ReleaseValue - m_ReleaseValue * m_Phase;
                }
                break;
            }
        }

        void prepare(double sampleRate, std::size_t maxBufferSize) override {
            Module::prepare(sampleRate, maxBufferSize);

            updateAttack();
            updateDecay();
            updateRelease();
        }

        void reset() override {
            Module::reset();

            m_State = State::Idle;
            m_Phase = 0;
            output = 0;
        }

        // ------------------------------------------------

    private:
        State m_State = State::Idle;

        // ------------------------------------------------

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

        float m_Phase = 0;
        float m_ReleaseValue = 0;
        float m_AttackValue = 0;

        // ------------------------------------------------

        void updateAttack() { m_Attack = 0.001 * m_AttackMillis * sampleRate(); }
        void updateDecay() { m_Decay = 0.001 * m_DecayMillis * sampleRate(); }
        void updateRelease() { m_Release = 0.001 * m_ReleaseMillis * sampleRate(); }

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------
