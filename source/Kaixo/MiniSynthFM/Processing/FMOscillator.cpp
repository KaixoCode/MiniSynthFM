
// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Processing/FMOscillator.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    std::size_t FMOscillatorParameters::oversample() const {
        switch (m_Quality) {
        case Quality::Low: return 1;
        case Quality::Normal: return 2;
        case Quality::High: return 4;
        case Quality::Ultra: return 8;
        case Quality::Extreme: return 16;
        default: return 1;
        }
    }

    // ------------------------------------------------

    FMOscillator::FMOscillator(FMOscillatorParameters& p) 
        : params(p) 
    {}

    // ------------------------------------------------
    
    void FMOscillator::trigger() {
        m_Phase = 0;
    }

    // ------------------------------------------------
    
    void FMOscillator::note(float note) { 
        if (m_Note != note) {
            m_Note = note;
            m_NoteFrequency = noteToFreq(m_Note);
        }
    }

    void FMOscillator::fm(float phase, std::size_t os) { m_PhaseModulation[os] += phase; }

    // ------------------------------------------------

    void FMOscillator::process() {
        updateFrequency();

        auto os = params.oversample();
        
        switch (simd_path::path) {
        case simd_path::s512:
            switch (os) {
            case 16: return processImpl<simd<float, 512>>();
            case  8: return processImpl<simd<float, 256>>();
            case  4: 
            case  2: return processImpl<simd<float, 128>>();
            default: return processImpl<float>();
            }
        case simd_path::s256:
            switch (os) {
            case 16: 
            case  8: return processImpl<simd<float, 256>>();
            case  4:
            case  2: return processImpl<simd<float, 128>>();
            default: return processImpl<float>();
            }
        case simd_path::s128:
            switch (os) {
            case 16:
            case  8: 
            case  4:
            case  2: return processImpl<simd<float, 128>>();
            default: return processImpl<float>();
            }
        case simd_path::s0: return processImpl<float>();
        }
    }

    // ------------------------------------------------
    
    void FMOscillator::hardSync(FMOscillator& osc) {
        if (osc.m_DidCycle) {
            float ratio = m_Frequency / osc.m_Frequency;
            m_Phase = Math::Fast::fmod1(osc.m_Phase * ratio);
        }
    }

    // ------------------------------------------------
    
    void FMOscillator::resetPhase() {
        m_Phase = 0;
    }

    // ------------------------------------------------

    void FMOscillator::updateFrequency() {
        m_Frequency = m_NoteFrequency * params.frequencyMultiplier();
    }

    // ------------------------------------------------

    template<std::size_t N>
    __forceinline constexpr auto ct_foreach(auto lambda) {
        return[&]<std::size_t ...Is>(std::index_sequence<Is...>) {
            return lambda.template operator() < Is... > ();
        }(std::make_index_sequence<N>{});
    }

    // ------------------------------------------------

    template<class SimdType>
    void FMOscillator::processImpl() {
        constexpr std::size_t Count = sizeof(SimdType) / sizeof(float);
        float delta = m_Frequency / sampleRate();
        std::size_t os = params.oversample();

        for (std::size_t i = 0; i < os; i += Count) {
            const SimdType offset = ct_foreach<Count>([&]<std::size_t ...Is> {
                return SimdType{ float(i + Is)... };
            });

            SimdType _output = Kaixo::at<SimdType>(output, i);
            SimdType _fmOutput = Kaixo::at<SimdType>(fmOutput, i);
            SimdType _phaseMod = Kaixo::at<SimdType>(m_PhaseModulation, i);

            SimdType phase = Math::Fast::fmod1((offset * (delta / os)) + (_phaseMod + (m_Phase + 10)));
            _output = this->at<SimdType>(phase);
            _fmOutput = this->fmAt<SimdType>(phase);

            Kaixo::store(output + i, _output);
            Kaixo::store(fmOutput + i, _fmOutput);
        }

        std::memset(m_PhaseModulation, 0, sizeof(m_PhaseModulation));
        m_Phase = Math::Fast::fmod1(m_Phase + delta);

        m_DidCycle = m_Phase < delta;
    }

    // ------------------------------------------------

    template<class SimdType>
    SimdType FMOscillator::at(SimdType p) {

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
        float xd = Math::Fast::max(m_Frequency / 35000.f, 0.002f);
        switch (params.m_Waveform) {
        case Waveform::Sine: return Math::Fast::nsin(0.5 - p);
        case Waveform::Triangle: return 1 - Math::Fast::abs(2 - 4 * p);
        case Waveform::Saw: return saw(p, xd);
        case Waveform::Square: return square(p, xd);
        }
    }

    template<class SimdType>
    SimdType FMOscillator::fmAt(SimdType p) {
        // requires 0 <= p <= 1
        switch (params.m_Waveform) {
        case Waveform::Sine: return Math::Fast::nsin(0.5 - p);
        case Waveform::Triangle: return 2 * (2 * p - 1) * ((2 * p - 1) * Math::Fast::sign(0.5 - p) + 1);
        case Waveform::Saw: return 4 * (p - p * p);
        case Waveform::Square: return 1 - Math::Fast::abs(2 - 4 * p);
        }
    }

    // ------------------------------------------------

}

// ------------------------------------------------
