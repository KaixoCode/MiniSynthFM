#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Processing/Module.hpp"
#include "Kaixo/Core/Processing/Filter.hpp"
#include "Kaixo/Core/Processing/Random.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    struct FilterParameters : public Module {

        // ------------------------------------------------

        float frequency = 0;
        float resonance = 0;
        float drive = 0;
        bool enable = true;

        // ------------------------------------------------

    };

    // ------------------------------------------------

    class CustomFilter : public ModuleContainer {
    public:

        // ------------------------------------------------

        FilterParameters& params;

        // ------------------------------------------------

        CustomFilter(FilterParameters& p);

        // ------------------------------------------------

        float input = 0;
        float output = 0;

        // ------------------------------------------------
        
        void process() override;
        void prepare(double sampleRate, std::size_t maxBufferSize) override;

        // ------------------------------------------------
        
        float frequencyModulation = 0;

        // ------------------------------------------------

    private:
        StereoEqualizer<3, float, Kaixo::Math::Fast, false> m_Filter{};

        Random m_Random{};
        std::size_t m_Counter = 0;
        float m_RandomFreq{};
        float m_FrequencyModulation{};
        float m_Ratio = 0.99;

        // ------------------------------------------------

    };
    
    // ------------------------------------------------

}

// ------------------------------------------------
