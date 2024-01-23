#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Processing/Module.hpp"
#include "Kaixo/Core/Processing/Random.hpp"

// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Processing/CustomFilter.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    class Delay : public ModuleContainer {
    public:

        // ------------------------------------------------

        constexpr static std::size_t MaxDelaySeconds = 10;

        // ------------------------------------------------

        float input = 0;
        Stereo output{ 0, 0 };

        // ------------------------------------------------
        
        Delay();

        // ------------------------------------------------

        void mix(float v) { m_Mix = v; }
        void delay(float millis) { 
            m_RealDelayL = Math::Fast::clamp(millis, 0, MaxDelaySeconds * 1000); 
            m_RealDelayR = Math::Fast::clamp(millis * 0.5, 0, MaxDelaySeconds * 1000); 
        }
        void feedback(float fb) { m_Feedback = fb; }
        void pingpong(bool v) { m_PingPong = v; }

        // ------------------------------------------------

        void process() override;
        void prepare(double sampleRate, std::size_t maxBufferSize) override;
        void reset() override;

        // ------------------------------------------------

        float read(float delayMs) const;

        // ------------------------------------------------

        void resize(std::size_t size);

        // ------------------------------------------------

        std::size_t size() const { return m_Samples.size(); }

        // ------------------------------------------------

    private:
        StereoEqualizer<1, float, Kaixo::Math::Fast, false> m_HighpassFilter{};
        FilterParameters m_FilterParameters;
        CustomFilter m_Filter{ m_FilterParameters };
        std::vector<float> m_Samples{};
        Random m_Random{};
        std::size_t m_Counter = 0;
        float m_RandomDelayL{};
        float m_RandomDelayR{};
        std::size_t m_Write = 0;
        float m_DelayL = 0;
        float m_DelayR = 0;
        float m_TargetDelayL = 0;
        float m_TargetDelayR = 0;
        float m_RealDelayL = 0;
        float m_RealDelayR = 0;
        float m_Smooth = 0.99;
        float m_Feedback = 0.5;
        float m_Mix = true;
        bool m_PingPong = false;

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------
