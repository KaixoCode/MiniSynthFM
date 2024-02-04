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

        void tune(Note t) { m_Tune = t; m_FrequencyDirty = true; }
        void octave(int o) { m_Octave = o; m_FrequencyDirty = true; }

        void waveform(Waveform wf) { m_Waveform = wf; }
        void waveform(float val) { waveform(normalToIndex(val, Waveform::Amount)); }

        void quality(Quality val) { m_Quality = val; }
        void quality(float val) { quality(normalToIndex(val, Quality::Amount)); }

        // ------------------------------------------------
        
        float frequencyMultiplier() { return m_FrequencyMultiplier; }

        // ------------------------------------------------

        void updateFrequency() {
            if (!m_FrequencyDirty) return;
            m_FrequencyDirty = false;
            m_FrequencyMultiplier = Math::Fast::exp2(1. / 12. * m_Tune + m_Octave);
        }

        // ------------------------------------------------

        std::size_t oversample() const;

        // ------------------------------------------------

    private:
        Note m_Tune = 0;
        float m_FrequencyMultiplier = 1;
        int m_Octave = 0;
        Waveform m_Waveform = Waveform::Sine;
        Quality m_Quality = Quality::Normal;
        bool m_FrequencyDirty = true;

        // ------------------------------------------------
        
        friend class FMOscillator;
        friend class SimdFMOscillator;

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

        float output[16]{};
        float fmOutput[16]{};

        // ------------------------------------------------

        void trigger();

        // ------------------------------------------------

        void note(float note);
        void fm(float phase, std::size_t os = 0);

        // ------------------------------------------------

        void process() override;

        // ------------------------------------------------
        
        void hardSync(FMOscillator& osc);
        void resetPhase();

        // ------------------------------------------------

    private:
        float m_Phase = 0;
        float m_PhaseModulation[16]{};
        float m_Frequency = 440;
        float m_FrequencyOffset = 440;
        float m_NoteFrequency = 0;
        bool m_DidCycle = false;

        // ------------------------------------------------
        
        Note m_Note = 1;

        // ------------------------------------------------

        void updateFrequency();

        // ------------------------------------------------
        
        template<class SimdType>
        void processImpl();

        // ------------------------------------------------

        template<class SimdType>
        SimdType at(SimdType p);

        template<class SimdType>
        SimdType fmAt(SimdType p);

        // ------------------------------------------------

    };

    // ------------------------------------------------
    
    class SimdFMOscillator : public Module {
    public:

        // ------------------------------------------------

        FMOscillatorParameters& params;

        // ------------------------------------------------

        SimdFMOscillator(FMOscillatorParameters& p)
            : params(p) 
        {}

        // ------------------------------------------------

        float output[MaxOversample][Voices]{};
        float fmOutput[MaxOversample][Voices]{};

        // ------------------------------------------------

        void trigger(std::size_t i) {
            resetPhase(i);
        }

        // ------------------------------------------------

        template<class SimdType>
        KAIXO_INLINE void note(std::size_t i, SimdType note) {
            Kaixo::store<SimdType>(m_Note + i, note);
            Kaixo::store<SimdType>(m_NoteFrequency + i, 440.f * Math::Fast::exp2(((note - 69) / 12.f)));
        }

        template<class SimdType>
        KAIXO_INLINE void fm(std::size_t i, SimdType phase, std::size_t os = 0) {
            auto _phaseModulation = Kaixo::at<SimdType>(m_PhaseModulation[os], i);
            Kaixo::store<SimdType>(m_PhaseModulation[os] + i, phase + _phaseModulation);
        }

        // ------------------------------------------------

        template<class SimdType>
        KAIXO_INLINE void process() {
            constexpr std::size_t Count = sizeof(SimdType) / sizeof(float);
            std::size_t os = params.oversample();

            for (std::size_t i = 0; i < Voices; i += Count) {
                auto _noteFrequency = Kaixo::at<SimdType>(m_NoteFrequency, i);
                auto _frequency = _noteFrequency * params.frequencyMultiplier();
                auto _phase = Kaixo::at<SimdType>(m_Phase, i);

                auto delta = _frequency / sampleRate();

                for (std::size_t j = 0; j < os; ++j) {
                    SimdType _output = Kaixo::at<SimdType>(output[j], i);
                    SimdType _fmOutput = Kaixo::at<SimdType>(fmOutput[j], i);
                    SimdType _phaseMod = Kaixo::at<SimdType>(m_PhaseModulation[j], i);

                    SimdType phase = Math::Fast::fmod1((j * delta / os) + (_phaseMod + (_phase + 10)));
                    _output = this->at<SimdType>(phase, _frequency);
                    _fmOutput = this->fmAt<SimdType>(phase);

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
        void hardSync(std::size_t i, SimdType shouldDo, SimdFMOscillator& osc) {
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

        void resetPhase(std::size_t i) {
            m_Phase[i] = 0;
        }

        // ------------------------------------------------

        float m_Phase[Voices]{};
        float m_PhaseModulation[MaxOversample][Voices]{};
        float m_Frequency[Voices]{};
        float m_FrequencyOffset[Voices]{};
        float m_NoteFrequency[Voices]{};
        float m_DidCycle[Voices]{};
        float m_Note[Voices]{};

        // ------------------------------------------------

        template<class SimdType>
        SimdType at(SimdType p, SimdType freq) {
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
            case Waveform::Sine: return Math::Fast::nsin(0.5 - p);
            case Waveform::Triangle: return 1 - Math::Fast::abs(2 - 4 * p);
            case Waveform::Saw: return saw(p, xd);
            case Waveform::Square: return square(p, xd);
            }
        }

        template<class SimdType>
        SimdType fmAt(SimdType p) {
            // requires 0 <= p <= 1
            switch (params.m_Waveform) {
            case Waveform::Sine: return Math::Fast::nsin(0.5 - p);
            case Waveform::Triangle: return 2 * (2 * p - 1) * ((2 * p - 1) * Math::Fast::sign(0.5 - p) + 1);
            case Waveform::Saw: return 4 * (p - p * p);
            case Waveform::Square: return 1 - Math::Fast::abs(2 - 4 * p);
            }
        }

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------
