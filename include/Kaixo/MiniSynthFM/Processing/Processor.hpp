#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Processing/Processor.hpp"
#include "Kaixo/Core/Processing/VoiceBank.hpp"
#include "Kaixo/Core/Processing/ParameterDatabase.hpp"
#include "Kaixo/Core/Processing/DelayBuffer.hpp"

// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Controller.hpp"
#include "Kaixo/MiniSynthFM/Processing/Voice.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {
    
    // ------------------------------------------------
    
    class Delay : public Module {
    public:
        
        // ------------------------------------------------
        
        constexpr static std::size_t MaxDelaySeconds = 10;

        // ------------------------------------------------
        
        float input;
        Stereo output;

        // ------------------------------------------------
        
        void mix(bool v) { m_Mix = v; }
        void delay(float millis) { m_TargetDelay = Math::Fast::clamp(millis, 0, MaxDelaySeconds * 1000); }
        void feedback(float fb) { m_Feedback = fb; }
        void pingpong(bool v) { m_PingPong = v; }

        // ------------------------------------------------
        
        void process() override {
            float out = read(m_Delay);
            m_Samples[m_Write] = input + m_Feedback * out;
            
            if (m_PingPong) {
                float outpp = read(m_Delay * 2);
                output = input * (1 - m_Mix) + m_Mix * Stereo{ out, outpp };
            } else {
                output = input * (1 - m_Mix) + m_Mix * out;
            }

            m_Write = (m_Write + 1) % size();
            m_Delay = m_Delay * m_Smooth + m_TargetDelay * (1 - m_Smooth);
        }

        void prepare(double sampleRate, std::size_t maxBufferSize) override {
            resize(sampleRate * MaxDelaySeconds * 2);
            m_Smooth = Math::smoothCoef(0.99, 48000. / sampleRate);
        }

        void reset() override {
            m_Delay = m_TargetDelay;
            std::ranges::fill(m_Samples, 0);
        }

        // ------------------------------------------------

        float read(float delayMs) const {
            float delaySamples = sampleRate() * delayMs / 1000.;
            float read = Math::Fast::fmod(m_Write + 2 * size() - delaySamples, size());
            std::size_t delay1 = static_cast<std::size_t>(read);
            std::size_t delay2 = static_cast<std::size_t>(read + 1) % size();
            float ratio = read - delay1;
            return m_Samples[delay2] * ratio + m_Samples[delay1] * (1 - ratio);
        }

        // ------------------------------------------------

        void resize(std::size_t size) {
            m_Samples.resize(size);
            reset();
        }

        // ------------------------------------------------

        std::size_t size() const { return m_Samples.size(); }

        // ------------------------------------------------

    private:        
        std::vector<float> m_Samples{};
        std::size_t m_Write = 0;
        float m_Delay = 0;
        float m_TargetDelay = 0;
        float m_Smooth = 0.99;
        float m_Feedback = 0.5;
        float m_Mix = true;
        bool m_PingPong = false;
        
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

        void noteOn(Note note, double velocity, int channel) override;
        void noteOff(Note note, double velocity, int channel) override;

        // ------------------------------------------------

        ParameterDatabase<MiniSynthFMProcessor> parameters{ this };
        VoiceParameters params;
        VoiceBank<MiniSynthFMVoice, Voices> voices{ params, params, params, params, params, params, params, params };
        Delay delay;

        // ------------------------------------------------
        
        void init() override;
        basic_json serialize() override;
        void deserialize(basic_json& data) override;

        // ------------------------------------------------

    };
    
    // ------------------------------------------------

}
    
// ------------------------------------------------
