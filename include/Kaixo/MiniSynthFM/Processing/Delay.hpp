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
        
        enum class Tempo {
            T1_1, T1_2, T1_4, T1_6, T1_8, T1_16, T1_32, T1_64, Amount
        };

        // ------------------------------------------------

        constexpr static std::size_t MaxDelaySeconds = 10;

        // ------------------------------------------------

        float input = 0;
        Stereo output{ 0, 0 };

        // ------------------------------------------------
        
        Delay();

        // ------------------------------------------------

        void mix(float v) { m_Mix = v; }
        void delay(float millis) { m_TargetDelay = Math::Fast::clamp(millis, 0, MaxDelaySeconds * 1000); }
        void feedback(float fb) { m_Feedback = fb; }
        void pingpong(bool v) { m_PingPong = v; }
        void synced(bool v) { m_Synced = v; }
        void tempo(float v) { tempo(normalToIndex(v, Tempo::Amount)); }
        void tempo(Tempo v) { m_Tempo = v; }

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
        std::size_t m_Write = 0;
        Tempo m_Tempo = Tempo::T1_6;
        float m_Delay = 0;
        float m_TargetDelay = 0;
        float m_Smooth = 0.99;
        float m_Feedback = 0.5;
        float m_Mix = true;
        bool m_PingPong = false;
        bool m_Synced = false;

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------
