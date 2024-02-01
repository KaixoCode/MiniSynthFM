#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Processing/Module.hpp"
#include "Kaixo/Core/Processing/Filter.hpp"
#include "Kaixo/Core/Processing/Random.hpp"

// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Controller.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    struct FilterParameters : public Module {

        // ------------------------------------------------

        float frequency = 0;
        float resonance = 0;
        float drive = 0;
        bool keytrack = false;
        Quality quality = Quality::Normal;

        // ------------------------------------------------

        std::size_t oversample() const;

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

        float input[16]{};
        float output = 0;

        // ------------------------------------------------
        
        Note note = 1;

        // ------------------------------------------------
        
        void process() override;
        void prepare(double sampleRate, std::size_t maxBufferSize) override;
        void reset() override;

        // ------------------------------------------------
        
        float frequencyModulation = 0;

        // ------------------------------------------------

    private:
        StereoEqualizer<3, float, Kaixo::Math::Fast, false> m_Filter{};

        struct AAFilter {
            double sampleRateIn = 44100;
            double sampleRateOut = 48000;

            AAFilter() {
                params.normalisedTransitionWidth = 0.01;
            }

            float process(float s) {
                params.f0 = 0.5 * sampleRateOut - 2;
                params.sampleRate = sampleRateIn;
                params.recalculateParameters();
                return filter.process(s, params);
            }

            void reset() { params.reset(); }

            EllipticFilter filter;
            EllipticParameters params;
        } m_AAF{};

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
