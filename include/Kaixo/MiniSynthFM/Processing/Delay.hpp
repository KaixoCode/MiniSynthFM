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

        void mix(float v);
        void delay(float millis);
        void feedback(float fb);
        void pingpong(bool v);
        void synced(bool v);
        void tempo(float v);
        void tempo(Tempo v);

        // ------------------------------------------------
        
        bool active() const override;

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
        StereoEqualizer<4, float, Kaixo::Math::Fast, false> m_Filter{};
        std::vector<float> m_Samples{};
        std::size_t m_Write = 0;
        std::size_t m_Counter = 0;
        std::size_t m_SamplesSilence = 0;
        Tempo m_Tempo = Tempo::T1_6;
        float m_Delay = 0;
        float m_Feedback = 0.5;
        float m_Mix = true;
        float m_RandomFrequency = 0;
        bool m_PingPong = false;
        bool m_Synced = false;

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------
