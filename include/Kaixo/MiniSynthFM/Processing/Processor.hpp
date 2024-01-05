#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Processing/Processor.hpp"
#include "Kaixo/Core/Processing/Voice.hpp"
#include "Kaixo/Core/Processing/Modules/Envelope.hpp"
#include "Kaixo/Core/Processing/ParameterDatabase.hpp"

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
    
    class FMOscillator : public Module {
    public:

        // ------------------------------------------------

        enum class Waveform { Sine, Triangle, Saw, Square, Amount };

        // ------------------------------------------------

        float output{};

        // ------------------------------------------------
        
        void trigger() {
            m_Phase = 0;
        }

        // ------------------------------------------------

        void note(float note) {
            if (m_Note != note) {
                m_Note = note;
                updateFrequency();
            }
        }

        void tune(float noteOffset) {
            if (m_Tune != noteOffset) {
                m_Tune = noteOffset;
                updateFrequency();
            }
        }

        void fm(float phase) {
            m_PhaseModulation += phase;
        }

        void volume(float v) { m_Volume = v; }

        void waveform(Waveform wf) { m_Waveform = wf; }
        void waveform(float val) { m_Waveform = normalToIndex(val, Waveform::Amount); }

        // ------------------------------------------------
        
        float at(float p) {
            // requires 0 <= p <= 1
            switch (m_Waveform) {
            case Waveform::Sine: return Math::Fast::nsin(p - 0.5);
            case Waveform::Triangle: return 0;
            case Waveform::Saw: return Math::Fast::saw(p, m_Frequency / sampleRate());
            case Waveform::Square: return 0;
            }
        }

        // ------------------------------------------------
        
        void process() override {
            float phase = m_Phase + m_PhaseModulation;
            float wave = at(Math::Fast::fmod1(phase + 10));
            output = wave * m_Volume;
            m_Phase = Math::Fast::fmod1(m_Phase + m_Frequency / sampleRate());
            m_PhaseModulation = 0;
        }

        // ------------------------------------------------
        
    private:
        float m_Phase = 0;
        float m_PhaseModulation = 0;
        float m_Frequency = 440;
        float m_Volume = 1;

        // ------------------------------------------------
        
        Waveform m_Waveform = Waveform::Sine;

        // ------------------------------------------------

        Note m_Note = 0;
        Note m_Tune = 0;

        // ------------------------------------------------
        
        void updateFrequency() {
            m_Frequency = noteToFreq(m_Note + m_Tune);
        }

        // ------------------------------------------------

    };

    // ------------------------------------------------
    
    class MiniSynthFMVoice : public ModuleContainer {
    public:

        // ------------------------------------------------
        
        MiniSynthFMVoice() {
            for (auto& osc : oscillator) registerModule(osc);
            for (auto& env : envelope) registerModule(env);
        }

        // ------------------------------------------------
        
        Note note = 64;

        // ------------------------------------------------

        Stereo output{};

        // ------------------------------------------------

        void trigger() {
            for (auto& osc : oscillator) osc.trigger();
            for (auto& env : envelope) env.trigger();
        }

        void release() {
            for (auto& env : envelope) env.release();
        }

        // ------------------------------------------------
        
        void process() {
            for (auto& osc : oscillator) {
                osc.note(note);
                osc.process();
            }

            for (auto& env : envelope) env.process();

            oscillator[0].fm(oscillator[1].output);
            oscillator[1].fm(oscillator[2].output);

            output = oscillator[0].output * envelope[0].output;
        }

        // ------------------------------------------------

        FMOscillator oscillator[Oscillators];
        ADSREnvelope envelope[Envelopes];

        // ------------------------------------------------

    };

    // ------------------------------------------------

    class MiniSynthFMProcessor : public Processor {
    public:

        // ------------------------------------------------
        
        MiniSynthFMProcessor();

        // ------------------------------------------------

        void process() override;
        
        // ------------------------------------------------

        void noteOn(Note note, double velocity, int channel) override {
            voices[0].note = note;
            voices[0].trigger();
        }

        void noteOff(Note note, double velocity, int channel) override {
            voices[0].release();
        }

        // ------------------------------------------------

        ParameterDatabase<MiniSynthFMProcessor> parameters{ this };

        MiniSynthFMVoice voices[1];

        // ------------------------------------------------

    };
    
    // ------------------------------------------------

}
    
// ------------------------------------------------
