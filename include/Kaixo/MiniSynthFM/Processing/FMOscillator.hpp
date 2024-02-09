#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Processing/Module.hpp"
#include "Kaixo/Core/Processing/Random.hpp"
#include "Kaixo/Core/Processing/Filter.hpp"

// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Controller.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    enum class Waveform { Sine, Triangle, Saw, Square, Amount };

    // ------------------------------------------------
    
    struct FMOscillatorParameters : public Module {

        // ------------------------------------------------
        
        FMOscillatorParameters(Quality& q);

        // ------------------------------------------------
        
        Quality& quality;

        // ------------------------------------------------

        void tune(Note t);
        void octave(int o);

        void waveform(Waveform wf);
        void waveform(float val);

        // ------------------------------------------------
        
        float frequencyMultiplier();

        // ------------------------------------------------

        void updateFrequency();

        // ------------------------------------------------

    private:
        Note m_Tune = 0;
        float m_FrequencyMultiplier = 1;
        int m_Octave = 0;
        Waveform m_Waveform = Waveform::Sine;
        bool m_FrequencyDirty = true;

        // ------------------------------------------------
        
        friend class FMOscillator;

        // ------------------------------------------------

    };

    // ------------------------------------------------
    
    class FMOscillator : public Module {
    public:

        // ------------------------------------------------

        FMOscillatorParameters& params;

        // ------------------------------------------------

        FMOscillator(FMOscillatorParameters& p);

        // ------------------------------------------------

        float output[MaxOversample][Voices]{};
        float fmOutput[MaxOversample][Voices]{};

        // ------------------------------------------------

        void trigger(std::size_t i);

        // ------------------------------------------------

        template<class SimdType>
        KAIXO_INLINE void hardSync(std::size_t i, SimdType shouldDo, FMOscillator& osc);

        // ------------------------------------------------

        template<class SimdType>
        KAIXO_INLINE void note(std::size_t i, SimdType note);

        template<class SimdType>
        KAIXO_INLINE void fm(std::size_t i, SimdType phase, std::size_t os = 0);

        // ------------------------------------------------

        template<class SimdType>
        KAIXO_INLINE void process();

        // ------------------------------------------------

    private:
        float m_Phase[Voices]{};
        float m_PhaseModulation[MaxOversample][Voices]{};
        float m_Frequency[Voices]{};
        float m_FrequencyOffset[Voices]{};
        float m_NoteFrequency[Voices]{};
        float m_DidCycle[Voices]{};
        float m_Note[Voices]{};

        // ------------------------------------------------

        template<class SimdType> KAIXO_INLINE std::pair<SimdType, SimdType> at(SimdType p, SimdType freq);

        // ------------------------------------------------

    };

    // ------------------------------------------------
    
    template<class SimdType>
    KAIXO_INLINE void FMOscillator::note(std::size_t i, SimdType note) {
        Kaixo::store<SimdType>(m_Note + i, note);
        Kaixo::store<SimdType>(m_NoteFrequency + i, 440.f * Math::Fast::exp2(((note - 69) / 12.f)));
    }

    template<class SimdType>
    KAIXO_INLINE void FMOscillator::fm(std::size_t i, SimdType phase, std::size_t os) {
        auto _phaseModulation = Kaixo::at<SimdType>(m_PhaseModulation[os], i);
        Kaixo::store<SimdType>(m_PhaseModulation[os] + i, phase + _phaseModulation);
    }

    // ------------------------------------------------
    
    template<class SimdType>
    KAIXO_INLINE void FMOscillator::hardSync(std::size_t i, SimdType shouldDo, FMOscillator& osc) {
        auto _phase = Kaixo::at<SimdType>(m_Phase, i);
        auto _oscphase = Kaixo::at<SimdType>(osc.m_Phase, i);
        auto _frequency = Kaixo::at<SimdType>(m_Frequency, i);
        auto _oscfrequency = Kaixo::at<SimdType>(osc.m_Frequency, i);
        auto _didcycle = Kaixo::conditional<SimdType>(shouldDo, Kaixo::at<SimdType>(osc.m_DidCycle, i));

        _phase = Kaixo::iff<SimdType>(_didcycle,
            [&] { return Math::Fast::fmod1(_oscphase * (_frequency / _oscfrequency)); },
            [&] { return _phase; });

        Kaixo::store<SimdType>(m_Phase + i, _phase);
    }

    // ------------------------------------------------

    template<class SimdType>
    KAIXO_INLINE void FMOscillator::process() {
        constexpr std::size_t Count = sizeof(SimdType) / sizeof(float);
        const auto oversampleAmount = oversampleForQuality(params.quality);

        for (std::size_t i = 0; i < Voices; i += Count) {
            auto _noteFrequency = Kaixo::at<SimdType>(m_NoteFrequency, i);
            auto _frequency = _noteFrequency * params.frequencyMultiplier();
            auto _phase = Kaixo::at<SimdType>(m_Phase, i);

            auto delta = _frequency / sampleRate();

            for (std::size_t j = 0; j < oversampleAmount; ++j) {
                SimdType _phaseMod = Kaixo::at<SimdType>(m_PhaseModulation[j], i);

                SimdType phase = Math::Fast::fmod1((j * delta / oversampleAmount) + (_phaseMod + (_phase + 10)));
                auto [_output, _fmOutput] = this->at<SimdType>(phase, _frequency);

                Kaixo::store<SimdType>(output[j] + i, _output);
                Kaixo::store<SimdType>(fmOutput[j] + i, _fmOutput);
            }
            _phase = Math::Fast::fmod1(_phase + delta);

            Kaixo::store<SimdType>(m_Frequency + i, _frequency);
            Kaixo::store<SimdType>(m_DidCycle + i, _phase < delta);
            Kaixo::store<SimdType>(m_Phase + i, _phase);
        }

        std::memset(m_PhaseModulation, 0, sizeof(m_PhaseModulation));
    }

    // ------------------------------------------------

    template<class SimdType>
    KAIXO_INLINE std::pair<SimdType, SimdType> FMOscillator::at(SimdType p, SimdType freq) {
        constexpr auto g = [](SimdType x) {
            return (x - 1) * x * (2 * x - 1);
        };
        
        constexpr auto saw = [](SimdType x, SimdType nf) {
            SimdType v1 = Math::Fast::fmod1(x - nf + 1);
            SimdType v2 = Math::Fast::fmod1(x + nf);
            SimdType v3 = x;

            return (g(v1) + g(v2) - 2 * g(v3)) / (6.f * nf * nf);
        };
        
        constexpr auto square = [](SimdType x, SimdType nf) {
            SimdType v1 = Math::Fast::fmod1(x - nf + 1);
            SimdType v2 = Math::Fast::fmod1(x + nf);
            SimdType v3 = x;
            SimdType v4 = Math::Fast::fmod1(2.5 - x - nf);
            SimdType v5 = Math::Fast::fmod1(1.5 - x + nf);
            SimdType v6 = Math::Fast::fmod1(1.5 - x);

            return (g(v1) + g(v2) - 2 * g(v3) + g(v4) + g(v5) - 2 * g(v6)) / (6.f * nf * nf);
        };

        // requires 0 <= p <= 1
        auto xd = Math::Fast::max(freq / 35000.f, SimdType(0.002f));
        switch (params.m_Waveform) {
        case Waveform::Sine: {
            auto val = Math::Fast::nsin(0.5 - p);
            return { val, val };
        }
        case Waveform::Triangle: {
            auto denorm = 2 * p - 1;
            return {
                1 - Math::Fast::abs(2 - 4 * p),
                2 * denorm * (denorm * Math::Fast::sign(0.5 - p) + 1)
            };
        }
        case Waveform::Saw: return {
            saw(p, xd),
            4 * (p - p * p)
        };
        case Waveform::Square: return {
            square(p, xd),
            1 - Math::Fast::abs(2 - 4 * p)
        };
        }
    }

    // ------------------------------------------------

}

// ------------------------------------------------
