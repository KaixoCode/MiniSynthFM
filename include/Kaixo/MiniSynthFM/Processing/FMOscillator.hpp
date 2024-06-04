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
    enum class ModType { Sync, Volume, Amount };

    // ------------------------------------------------
    
    struct FMOscillatorParameters : public Module {

        // ------------------------------------------------
        
        FMOscillatorParameters(Quality& q);

        // ------------------------------------------------
        
        Quality& quality;

        // ------------------------------------------------
        
        PhaseMode phaseMode = PhaseMode::Contiguous;

        // ------------------------------------------------

        void tune(Note t);
        void octave(int o);

        void waveform(Waveform wf);
        void waveform(float val);
        
        void modType(ModType wf);
        void modType(float val);
        ModType modType() const { return m_ModType; }

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
        ModType m_ModType = ModType::Sync;
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

        alignas(sizeof(float) * Voices) float output[MaxOversample][Voices]{};
        alignas(sizeof(float) * Voices) float fmOutput[MaxOversample][Voices]{};

        // ------------------------------------------------

        void trigger(std::size_t i);

        // ------------------------------------------------

        template<class SimdType>
        KAIXO_INLINE void hardSync(std::size_t i, FMOscillator& osc);
        
        template<class SimdType>
        KAIXO_INLINE void ringMod(std::size_t i, SimdType value, std::size_t os = 0);

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
        alignas(sizeof(float) * Voices) float m_Phase[Voices]{};
        alignas(sizeof(float) * Voices) float m_PhaseModulation[MaxOversample][Voices]{};
        alignas(sizeof(float) * Voices) float m_Frequency[Voices]{};
        alignas(sizeof(float) * Voices) float m_FrequencyOffset[Voices]{};
        alignas(sizeof(float) * Voices) float m_NoteFrequency[Voices]{};
        alignas(sizeof(float) * Voices) float m_DidCycle[Voices]{};
        alignas(sizeof(float) * Voices) float m_Note[Voices]{};
        alignas(sizeof(float) * Voices) float m_RingModulation[MaxOversample][Voices]{};

        // ------------------------------------------------

        template<class SimdType> KAIXO_INLINE std::pair<SimdType, SimdType> at(SimdType p, SimdType freq);

        // ------------------------------------------------

    };

    // ------------------------------------------------
    
    template<class SimdType>
    KAIXO_INLINE void FMOscillator::note(std::size_t i, SimdType note) {
        Kaixo::store<SimdType>(m_Note + i, note);
        Kaixo::store<SimdType>(m_NoteFrequency + i, 440.f * Math::Fast::exp2(((note - 69.f) / 12.f)));
    }

    template<class SimdType>
    KAIXO_INLINE void FMOscillator::fm(std::size_t i, SimdType phase, std::size_t os) {
        auto _phaseModulation = Kaixo::load<SimdType>(m_PhaseModulation[os], i);
        Kaixo::store<SimdType>(m_PhaseModulation[os] + i, phase + _phaseModulation);
    }

    // ------------------------------------------------
    
    template<class SimdType>
    KAIXO_INLINE void FMOscillator::hardSync(std::size_t i, FMOscillator& osc) {
        auto _phase = Kaixo::load<SimdType>(m_Phase, i);
        auto _oscphase = Kaixo::load<SimdType>(osc.m_Phase, i);
        auto _frequency = Kaixo::load<SimdType>(m_Frequency, i);
        auto _oscfrequency = Kaixo::load<SimdType>(osc.m_Frequency, i);
        auto _didcycle = Kaixo::load<SimdType>(osc.m_DidCycle, i);

        _phase = Kaixo::iff<SimdType>(_didcycle,
            [&] { return Math::Fast::fmod1(_oscphase * (_frequency / _oscfrequency)); },
            [&] { return _phase; });

        Kaixo::store<SimdType>(m_Phase + i, _phase);
    }
    
    template<class SimdType>
    KAIXO_INLINE void FMOscillator::ringMod(std::size_t i, SimdType value, std::size_t os) {
        Kaixo::store<SimdType>(m_RingModulation[os] + i, value);
    }

    // ------------------------------------------------

    template<class SimdType>
    KAIXO_INLINE void FMOscillator::process() {
        constexpr std::size_t Count = sizeof(SimdType) / sizeof(float);
        const auto oversampleAmount = oversampleForQuality(params.quality);

        for (std::size_t i = 0; i < Voices; i += Count) {
            auto _noteFrequency = Kaixo::load<SimdType>(m_NoteFrequency, i);
            auto _frequency = _noteFrequency * params.frequencyMultiplier();
            auto _phase = Kaixo::load<SimdType>(m_Phase, i);

            auto delta = _frequency / static_cast<float>(sampleRate());

            for (std::size_t j = 0; j < oversampleAmount; ++j) {
                SimdType _phaseMod = Math::Fast::fmod1(Kaixo::load<SimdType>(m_PhaseModulation[j], i));
                SimdType _ringMod = Kaixo::load<SimdType>(m_RingModulation[j], i);

                SimdType phase = Math::Fast::fmod1((static_cast<float>(j) * delta / static_cast<float>(oversampleAmount)) + (_phaseMod + _phase + 10.f));
                auto [_output, _fmOutput] = this->at<SimdType>(phase, _frequency);

                Kaixo::store<SimdType>(output[j] + i, _output * _ringMod);
                Kaixo::store<SimdType>(fmOutput[j] + i, _fmOutput * _ringMod);
            }
            _phase = Math::Fast::fmod1(_phase + delta);

            Kaixo::store<SimdType>(m_Frequency + i, _frequency);
            Kaixo::store<SimdType>(m_DidCycle + i, _phase < delta);
            Kaixo::store<SimdType>(m_Phase + i, _phase);
        }

        std::memset(m_PhaseModulation, 0, sizeof(m_PhaseModulation));
        for (auto& v : m_RingModulation) for (auto& a : v) a = 1.f;
    }

    // ------------------------------------------------

    template<class SimdType>
    KAIXO_INLINE std::pair<SimdType, SimdType> FMOscillator::at(SimdType p, SimdType freq) {
        constexpr auto g = [](SimdType x) {
            return (x - 1.f) * x * (2.f * x - 1.f);
        };
        
        constexpr auto saw = [](SimdType x, SimdType nf) {
            SimdType v1 = Math::Fast::fmod1(x - nf + 1.f);
            SimdType v2 = Math::Fast::fmod1(x + nf);
            SimdType v3 = x;

            return (g(v1) + g(v2) - 2.f * g(v3)) / (6.f * nf * nf);
        };
        
        constexpr auto square = [](SimdType x, SimdType nf) {
            SimdType v1 = Math::Fast::fmod1(x - nf + 1.f);
            SimdType v2 = Math::Fast::fmod1(x + nf);
            SimdType v3 = x;
            SimdType v4 = Math::Fast::fmod1(2.5f - x - nf);
            SimdType v5 = Math::Fast::fmod1(1.5f - x + nf);
            SimdType v6 = Math::Fast::fmod1(1.5f - x);

            return (g(v1) + g(v2) - 2.f * g(v3) + g(v4) + g(v5) - 2.f * g(v6)) / (6.f * nf * nf);
        };

        // requires 0 <= p <= 1
        auto xd = Math::Fast::max(freq / 35000.f, SimdType(0.002f));
        switch (params.m_Waveform) {
        case Waveform::Sine: {
            auto val = Math::Fast::nsin(0.5f - p);
            return { val, val };
        }
        case Waveform::Triangle: {
            auto denorm = 2.f * p - 1.f;
            return {
                1.f - Math::Fast::abs(2.f - 4.f * p),
                2.f * denorm * (denorm * Math::Fast::sign(0.5f - p) + 1.f)
            };
        }
        case Waveform::Saw: return {
            saw(p, xd),
            4.f * (p - p * p)
        };
        case Waveform::Square: return {
            square(p, xd),
            1.f - Math::Fast::abs(2.f - 4.f * p)
        };
        }
    }

    // ------------------------------------------------

}

// ------------------------------------------------
