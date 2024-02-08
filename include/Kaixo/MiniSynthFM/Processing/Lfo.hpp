#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Processing/Module.hpp"
#include "Kaixo/Core/Processing/Random.hpp"

// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Controller.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    enum class LfoWaveform {
        Sine, Triangle, Saw, Square, Quantized, Noise, Amount
    };

    // ------------------------------------------------

    struct LfoParameters : public Module {

        // ------------------------------------------------

        enum class Tempo {
            T32_1, T16_1, T8_1, T4_1, T2_1, T1_1, T1_2, T1_4, T1_8, T1_16, T1_32, T1_64, Amount
        };

        // ------------------------------------------------

        void frequency(float hz);
        void tempo(float t);
        void tempo(Tempo t);
        void waveform(float w);
        void waveform(LfoWaveform w);
        void synced(bool s);

        // ------------------------------------------------

        float bars();

        // ------------------------------------------------

        float samplesPerOscillation();

        // ------------------------------------------------

    private:
        float m_Frequency = 10; // Hz
        bool m_Synced = false;
        Tempo m_Tempo = Tempo::T1_1;
        LfoWaveform m_Waveform = LfoWaveform::Sine;

        // ------------------------------------------------

        friend class Lfo;

        // ------------------------------------------------

    };

    // ------------------------------------------------
    
    class Lfo : public Module {
    public:
        
        // ------------------------------------------------

        LfoParameters& params;

        // ------------------------------------------------

        Lfo(LfoParameters& p);

        // ------------------------------------------------
        
        float output[Voices]{};

        // ------------------------------------------------
        
        void trigger(std::size_t i);

        // ------------------------------------------------
        
        template<class SimdType>
        KAIXO_INLINE void process();

        // ------------------------------------------------

        template<class SimdType>
        KAIXO_INLINE SimdType at(SimdType x, std::size_t i);

        // ------------------------------------------------

    private:
        float m_Phase[Voices]{};
        float m_Quantized[Voices]{};
        float m_Noise[Voices]{};

        Random m_Random{};

        // ------------------------------------------------

    };

    // ------------------------------------------------
    
    template<class SimdType>
    KAIXO_INLINE void Lfo::process() {
        constexpr std::size_t Count = sizeof(SimdType) / sizeof(float);

        float delta = 1. / params.samplesPerOscillation();
        for (std::size_t i = 0; i < Voices; i += Count) {
            auto phase = Kaixo::at<SimdType>(m_Phase, i);
            Kaixo::store<SimdType>(output + i, this->at<SimdType>(phase, i));
            Kaixo::store<SimdType>(m_Phase + i, Math::Fast::fmod1(phase + delta));
        }

        // TODO: find way to parallelize this
        for (std::size_t i = 0; i < Voices; i += Count) {
            if (m_Phase[i] < delta) {
                m_Quantized[i] = m_Random.next() * 2 - 1;
            }

            // TODO: m_Noise
        }
    }

    // ------------------------------------------------

    template<class SimdType>
    KAIXO_INLINE SimdType Lfo::at(SimdType x, std::size_t i) {
        switch (params.m_Waveform) {
        case LfoWaveform::Sine: return Math::Fast::nsin(0.5 - x);
        case LfoWaveform::Triangle: return 1 - Math::Fast::abs(2 - 4 * x);
        case LfoWaveform::Saw: return 1 - 2 * x;
        case LfoWaveform::Square: return 1 - 2 * (x > 0.5);
        case LfoWaveform::Quantized: return Kaixo::at<SimdType>(m_Quantized, i);
        case LfoWaveform::Noise: return Kaixo::at<SimdType>(m_Noise, i);
        }
    }

    // ------------------------------------------------

}

// ------------------------------------------------
