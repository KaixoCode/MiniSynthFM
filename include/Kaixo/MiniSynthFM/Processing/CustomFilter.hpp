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

        FilterParameters(Quality& q);

        // ------------------------------------------------

        Quality& quality;

        // ------------------------------------------------

        float frequency = 0;
        float resonance = 0;
        float drive = 0;
        bool keytrack = false;
        
        // ------------------------------------------------

    };

    // ------------------------------------------------
    
    class ParallelAntiAliasFilter : public Module {
    public:

        // ------------------------------------------------

        struct Settings {

            // ------------------------------------------------

            float sampleRateIn = 96000; // SampleRate going into the filter
            float sampleRateOut = 48000; // SampleRate to filter for

            // ------------------------------------------------

            float passbandAmplitudedB = -1;  // Gain in dB of passband
            float stopbandAmplitudedB = -80; // Gain in dB of stopband
            float normalisedTransitionWidth = 0.001; // Transition width of band

            // ------------------------------------------------
            
            bool operator==(const Settings& other) const;

            // ------------------------------------------------

        } settings;

        // ------------------------------------------------

        void recalculate();

        // ------------------------------------------------
        
        struct Coefficient {
            int order = 1;
            float b[4];
            float a[4];

            float state[4][Voices];
        };

        // ------------------------------------------------

        std::vector<Coefficient> coefficients{};

        // ------------------------------------------------
        
        template<class SimdType>
        KAIXO_INLINE SimdType process(SimdType input, std::size_t i);

        // ------------------------------------------------
        
    private:
        Settings m_Settings;

        // ------------------------------------------------

    };

    // ------------------------------------------------
    
    class ParallelFilter : public Module {
    public:

        // ------------------------------------------------

        template<class SimdType> 
        KAIXO_INLINE SimdType processLowpass(SimdType input, SimdType frequency, SimdType q, std::size_t i);

        template<class SimdType> 
        KAIXO_INLINE SimdType processPeaking(SimdType input, SimdType frequency, SimdType q, SimdType gain, std::size_t i);

        void finalize();

        // ------------------------------------------------

    private:
        float m_X[3][Voices]{};
        float m_Y[3][Voices]{};

        // ------------------------------------------------

        std::size_t m_M0 = 0;
        std::size_t m_M1 = 1;
        std::size_t m_M2 = 2;

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
        
        float note[Voices]{};
        float frequencyModulation[Voices]{};
        float input[MaxOversample][Voices]{};
        float output[Voices]{};

        // ------------------------------------------------

        template<class SimdType>
        void process();
        void prepare(double sampleRate, std::size_t maxBufferSize) override;
        void reset() override;
        
        // ------------------------------------------------

    private:
        std::size_t m_Counter = 0;
        float m_RandomFrequency = 0;

        // ------------------------------------------------

        ParallelFilter m_Filter[3]{};

        // ------------------------------------------------
        
        ParallelAntiAliasFilter m_AAF{};

        // ------------------------------------------------

        float m_Ratio = 0.99;
        float m_FrequencyModulation[Voices]{};

        // ------------------------------------------------

    };

    // ------------------------------------------------
    
    template<class SimdType>
    KAIXO_INLINE SimdType ParallelAntiAliasFilter::process(SimdType input, std::size_t i) {
        SimdType res = input;
        for (auto& c : coefficients) {
            auto output = (c.b[0] * res) + Kaixo::at<SimdType>(c.state[0], i);

            for (int j = 0; j < c.order - 1; ++j) {
                Kaixo::store<SimdType>(c.state[j] + i, (c.b[j + 1] * res) - (c.a[j + 1] * output) + Kaixo::at<SimdType>(c.state[j + 1], i));
            }

            Kaixo::store<SimdType>(c.state[c.order - 1] + i, (c.b[c.order] * res) - (c.a[c.order] * output));
            res = output;
        }
        return res;
    }

    // ------------------------------------------------
    
    template<class SimdType>
    KAIXO_INLINE SimdType ParallelFilter::processLowpass(SimdType input, SimdType frequency, SimdType q, std::size_t i) {
        auto freq = Math::Fast::clamp(frequency / static_cast<float>(sampleRate()), SimdType(0.f), SimdType(0.5f));
        auto omega = 2.f * std::numbers::pi_v<float> * freq;
        auto cosOmega = 0.f - Math::Fast::nsin(freq - 0.25f);
        auto sinOmega = Math::Fast::nsin(freq);
        auto qValue = Math::Fast::powN<4>(q);
        auto alpha = sinOmega / (qValue * 12.f + 0.8f);

        auto a0 = +1.0f + alpha;
        auto a1 = -2.0f * cosOmega;
        auto a2 = +1.0f - alpha;
        auto b0 = (1.0f - cosOmega) / 2.0f;
        auto b1 = (1.0f - cosOmega);
        auto b2 = (1.0f - cosOmega) / 2.0f;

        auto b0a0 = b0 / a0;
        auto b1a0 = b1 / a0;
        auto b2a0 = b2 / a0;
        auto a1a0 = a1 / a0;
        auto a2a0 = a2 / a0;

        auto x0 = input;
        auto x1 = Kaixo::at<SimdType>(m_X[m_M1], i);
        auto x2 = Kaixo::at<SimdType>(m_X[m_M2], i);
        auto y1 = Kaixo::at<SimdType>(m_Y[m_M1], i);
        auto y2 = Kaixo::at<SimdType>(m_Y[m_M2], i);

        auto result = b0a0 * x0 + b1a0 * x1 + b2a0 * x2
                                - a1a0 * y1 - a2a0 * y2;

        Kaixo::store<SimdType>(m_Y[m_M0] + i, result);
        Kaixo::store<SimdType>(m_X[m_M0] + i, input);

        return result;
    }

    template<class SimdType>
    KAIXO_INLINE SimdType ParallelFilter::processPeaking(SimdType input, SimdType frequency, SimdType q, SimdType gain, std::size_t i) {
        constexpr float log10_2 = std::numbers::ln2 / std::numbers::ln10;
        auto freq = Math::Fast::clamp(frequency / static_cast<float>(sampleRate()), SimdType(0.f), SimdType(0.5f));
        auto omega = 2.f * std::numbers::pi_v<float> * freq;
        auto cosOmega = 0.f - Math::Fast::nsin(freq - 0.25f);
        auto sinOmega = Math::Fast::nsin(freq);
        auto qValue = 2.f * q + 0.2f;

        auto A = Math::pow(SimdType(10.f), gain / 40.0f);
        auto alpha = sinOmega * Math::Fast::sinh((log10_2 / 2.0f) * (qValue * 4.f + 0.2f) * (omega / sinOmega));

        auto a0 = +1.0f + alpha / A;
        auto a1 = -2.0f * cosOmega;
        auto a2 = +1.0f - alpha / A;
        auto b0 = +1.0f + alpha * A;
        auto b1 = -2.0f * cosOmega;
        auto b2 = +1.0f - alpha * A;

        auto b0a0 = b0 / a0;
        auto b1a0 = b1 / a0;
        auto b2a0 = b2 / a0;
        auto a1a0 = a1 / a0;
        auto a2a0 = a2 / a0;

        auto x0 = input;
        auto x1 = Kaixo::at<SimdType>(m_X[m_M1], i);
        auto x2 = Kaixo::at<SimdType>(m_X[m_M2], i);
        auto y1 = Kaixo::at<SimdType>(m_Y[m_M1], i);
        auto y2 = Kaixo::at<SimdType>(m_Y[m_M2], i);

        auto result = b0a0 * x0 + b1a0 * x1 + b2a0 * x2
                                - a1a0 * y1 - a2a0 * y2;

        Kaixo::store<SimdType>(m_Y[m_M0] + i, result);
        Kaixo::store<SimdType>(m_X[m_M0] + i, input);

        return result;
    }

    // ------------------------------------------------
    
    template<class SimdType>
    void CustomFilter::process() {
        constexpr std::size_t Count = sizeof(SimdType) / sizeof(float);

        const auto oversampleAmount = oversampleForQuality(params.quality);

        m_AAF.settings.sampleRateIn = oversampleAmount * sampleRate();
        m_AAF.settings.sampleRateOut = sampleRate();
        m_AAF.settings.passbandAmplitudedB = -1;
        m_AAF.settings.stopbandAmplitudedB = -80;
        m_AAF.settings.normalisedTransitionWidth = 0.001;
        m_AAF.recalculate();
        
        // every 2 ms
        float timer = 2 * sampleRate() / 1000.;
        if (m_Counter++ > timer) {
            m_RandomFrequency = Random::next();
            m_Counter = 0;
        }

        for (std::size_t i = 0; i < Voices; i += Count) {
            auto noteValue = Kaixo::at<SimdType>(note, i);
            auto freqMod = Kaixo::at<SimdType>(m_FrequencyModulation, i) * m_Ratio + 
                            Kaixo::at<SimdType>(frequencyModulation, i) * (1 - m_Ratio);
            Kaixo::store(m_FrequencyModulation + i, freqMod);

            auto freqValue = Math::Fast::magnitude_to_log(params.frequency + freqMod, SimdType(16.f), SimdType(16000.f));

            if (params.keytrack) {
                freqValue = Math::Fast::clamp(freqValue * Math::Fast::exp2((noteValue - 60.f) / 12.f), SimdType(16.f), SimdType(16000.f));
            } else {
                freqValue = Math::Fast::clamp(freqValue, SimdType(16.f), SimdType(16000.f));
            }

            auto nfreq = (freqValue / 16000.f);
            auto randRange = 24.f * (1 - (1 - params.drive) * (1 - params.drive)) + 6.f * (1.f - nfreq * nfreq);
            auto frequency = Math::Fast::clamp(freqValue + (m_RandomFrequency * 2 - 1) * randRange, SimdType(16.f), SimdType(16000.f));
            auto resonance = params.resonance * (1.f - nfreq * nfreq * nfreq * nfreq);

            // Less resonance when low frequency
            resonance = Kaixo::iff<SimdType>(nfreq < SimdType(0.01f),
                [&] { return resonance * (0.2f + 0.8f * (nfreq / 0.01f)); },
                [&] { return resonance; });

            auto drive = Math::Fast::db_to_magnitude(params.drive * 12);

            SimdType res{};
            if (oversampleAmount == 1) {
                auto inputValue = Kaixo::at<SimdType>(input[0], i);
                res = params.drive * Math::Fast::tanh_like(inputValue * drive) + inputValue * (1 - params.drive);
            } else {
                for (std::size_t j = 0; j < oversampleAmount; ++j) {
                    auto inputValue = Kaixo::at<SimdType>(input[j], i);
                    inputValue = params.drive * Math::Fast::tanh_like(inputValue * drive) + inputValue * (1 - params.drive);
                    res = m_AAF.process<SimdType>(inputValue, i);
                }
            }

            auto filterOutput = m_Filter[0].processLowpass<SimdType>(res, frequency, resonance, i);
            filterOutput = m_Filter[1].processPeaking<SimdType>(filterOutput, frequency * 0.9f, resonance * 0.2f + 0.2f, params.drive * 12.f - resonance * 15.f, i);
            filterOutput = m_Filter[2].processPeaking<SimdType>(filterOutput, frequency * 1.1f, 0.2f - resonance * 0.2f, resonance * 15.f - params.drive * 12.f, i);
            filterOutput = params.drive * Math::Fast::tanh_like(1.115f * filterOutput) + filterOutput * (1.f - 0.9f * params.drive);
            
            Kaixo::store<SimdType>(output + i, filterOutput);
        }

        m_Filter[0].finalize();
        m_Filter[1].finalize();
        m_Filter[2].finalize();
    }

    // ------------------------------------------------

}

// ------------------------------------------------
