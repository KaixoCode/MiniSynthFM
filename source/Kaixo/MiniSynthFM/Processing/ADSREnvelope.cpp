
// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Processing/ADSREnvelope.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    void ADSREnvelopeParameters::attack(float millis) {
        if (m_AttackMillis != millis) {
            m_AttackMillis = millis;
            updateAttack();
        }
    }

    void ADSREnvelopeParameters::decay(float millis) {
        if (m_DecayMillis != millis) {
            m_DecayMillis = millis;
            updateDecay();
        }
    }

    void ADSREnvelopeParameters::release(float millis) {
        if (m_ReleaseMillis != millis) {
            m_ReleaseMillis = millis;
            updateRelease();
        }
    }

    void ADSREnvelopeParameters::sustain(float level) { m_Sustain = level; }

    // ------------------------------------------------

    void ADSREnvelopeParameters::prepare(double sampleRate, std::size_t maxBufferSize) {
        Module::prepare(sampleRate, maxBufferSize);

        updateAttack();
        updateDecay();
        updateRelease();
    }

    // ------------------------------------------------
    
    ADSREnvelope::ADSREnvelope(ADSREnvelopeParameters& p) 
        : params(p) 
    {}

    // ------------------------------------------------

    void ADSREnvelope::gate(bool gate) {
        if (gate) trigger();
        else release();
    }

    void ADSREnvelope::release() {
        if (m_State != State::Idle) {
            m_State = State::Release;
            m_Phase = 0;
            m_ReleaseValue = output;
        }
    }

    void ADSREnvelope::trigger() {
        m_State = State::Attack;
        m_Phase = 0;
        m_AttackValue = output;
    }

    // ------------------------------------------------

    void ADSREnvelope::process() {
        switch (m_State) {
        case State::Idle: output = 0; break;
        case State::Sustain:
            output = params.m_Sustain;
            if (params.loop) trigger();
            break;
        case State::Attack:
            m_Phase += 1 / params.m_Attack;
            if (m_Phase >= 1.0) {
                output = 1;
                m_Phase = 0;
                if (params.trigger) { 
                    m_State = State::Release; 
                    m_ReleaseValue = output;
                } else m_State = State::Decay;
            } else {
                auto powerPhase = (1 - (1 - m_Phase) * (1 - m_Phase));
                output = m_AttackValue + (1 - m_AttackValue) * powerPhase;
            }
            break;
        case State::Decay:
            m_Phase += 1 / params.m_Decay;
            if (m_Phase >= 1.0) {
                output = params.m_Sustain;
                m_Phase = 0;
                m_State = State::Sustain;
            } else {
                auto powerPhase = (1 - (1 - m_Phase) * (1 - m_Phase) * (1 - m_Phase) * (1 - m_Phase));
                output = 1 - (1 - params.m_Sustain) * powerPhase;
            }
            break;
        case State::Release:
            m_Phase += 1 / params.m_Release;
            if (m_Phase >= 1.0) {
                output = 0;
                m_Phase = 0;
                m_State = State::Idle;
            } else {
                auto powerPhase = (1 - (1 - m_Phase) * (1 - m_Phase) * (1 - m_Phase) * (1 - m_Phase));
                output = m_ReleaseValue - m_ReleaseValue * powerPhase;
            }
            break;
        }
    }

    void ADSREnvelope::reset() {
        Module::reset();

        m_State = State::Idle;
        m_Phase = 0;
        output = 0;
    }

    // ------------------------------------------------

}

// ------------------------------------------------
