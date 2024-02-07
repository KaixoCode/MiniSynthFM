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
    
    class ParallelAntiAliasFilter : public Module {
    public:

        // ------------------------------------------------

        float sampleRateIn  = 96000; // SampleRate going into the filter
        float sampleRateOut = 48000; // SampleRate to filter for

        // ------------------------------------------------

        float passbandAmplitudedB = -1;  // Gain in dB of passband
        float stopbandAmplitudedB = -80; // Gain in dB of stopband
        float normalisedTransitionWidth = 0.001; // Transition width of band

        // ------------------------------------------------

        void recalculate() {
            float sampleRate = sampleRateIn;
            float filterFrequency = sampleRateOut / 2 - 10; // 10 Hz extra headroom

            auto normalisedFrequency = filterFrequency / sampleRate;

            auto fp = normalisedFrequency - normalisedTransitionWidth / 2;
            auto fs = normalisedFrequency + normalisedTransitionWidth / 2;

            double Ap = passbandAmplitudedB;
            double As = stopbandAmplitudedB;
            auto Gp = Math::Fast::db_to_magnitude(Ap);
            auto Gs = Math::Fast::db_to_magnitude(As);
            auto epsp = std::sqrt(1.0 / (Gp * Gp) - 1.0);
            auto epss = std::sqrt(1.0 / (Gs * Gs) - 1.0);

            auto omegap = std::tan(std::numbers::pi * fp);
            auto omegas = std::tan(std::numbers::pi * fs);

            auto k = omegap / omegas;
            auto k1 = epsp / epss;

            int N;

            double K, Kp, K1, K1p;

            ellipticIntegralK(k, K, Kp);
            ellipticIntegralK(k1, K1, K1p);

            N = std::round(std::ceil((K1p * K) / (K1 * Kp)));

            const std::size_t r = N % 2;
            const std::size_t L = (N - r) / 2;
            const double H0 = std::pow(Gp, 1.0 - r);

            std::vector<std::complex<double>> pa, za;
            pa.reserve(L);
            za.reserve(L);
            std::complex<double> j(0, 1);

            auto v0 = -j * (asne(j / epsp, k1) / (double)N);

            if (r == 1) pa.push_back(omegap * j * sne(j * v0, k));

            for (std::size_t i = 1; i <= L; ++i) {
                auto ui = (2 * i - 1.0) / (double)N;
                auto zetai = cde(ui, k);

                pa.push_back(omegap * j * cde(ui - j * v0, k));
                za.push_back(omegap * j / (k * zetai));
            }

            std::vector<std::complex<double>> p, z, g;
            p.reserve(L + 1);
            z.reserve(L + 1);
            g.reserve(L + 1);

            if (r == 1) {
                p.push_back((1.0 + pa[0]) / (1.0 - pa[0]));
                g.push_back(0.5 * (1.0 - p[0]));
            }

            for (std::size_t i = 0; i < L; ++i) {
                p.push_back((1.0 + pa[i + r]) / (1.0 - pa[i + r]));
                z.push_back(za.size() == 0 ? -1.0 : (1.0 + za[i]) / (1.0 - za[i]));
                g.push_back((1.0 - p[i + r]) / (1.0 - z[i]));
            }

            coefficients.clear();

            if (r == 1) {
                auto b0 = static_cast<float> (H0 * std::real(g[0]));
                auto b1 = b0;
                auto a1 = static_cast<float> (-std::real(p[0]));

                coefficients.push_back({ .order = 1, .b = { b0, b1 }, .a = { 1.0, a1 } });
            }

            for (std::size_t i = 0; i < L; ++i) {
                auto gain = std::pow(std::abs(g[i + r]), 2.0);

                auto b0 = static_cast<float> (gain);
                auto b1 = static_cast<float> (std::real(-z[i] - std::conj(z[i])) * gain);
                auto b2 = static_cast<float> (std::real(z[i] * std::conj(z[i])) * gain);

                auto a1 = static_cast<float> (std::real(-p[i + r] - std::conj(p[i + r])));
                auto a2 = static_cast<float> (std::real(p[i + r] * std::conj(p[i + r])));

                coefficients.push_back({ .order = 2, .b = { b0, b1, b2 }, .a = { 1.0, a1, a2 } });
            }
        }

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
        SimdType process(SimdType input, std::size_t i) {
            float res = s;
            for (auto& c : coefficients) {
                auto output = (c.b[0] * res) + c.state[0].l;

                for (int j = 0; j < c.order - 1; ++j)
                    c.state[j] = (c.b[j + 1] * res) - (c.a[j + 1] * output) + c.state[j + 1];

                c.state[c.order - 1] = (c.b[c.order] * res) - (c.a[c.order] * output);
                res = output;
            }

            return res;
        }

        // ------------------------------------------------

    };

    // ------------------------------------------------
    
    class ParallelFilter : public Module {
    public:

        // ------------------------------------------------

        float frequency[Voices];
        float q[Voices];
        float gain[Voices];

        // ------------------------------------------------

        template<class SimdType>
        SimdType process(SimdType input, std::size_t i);

        void finalize();

        // ------------------------------------------------

        template<class SimdType> void calculateLowpass();
        template<class SimdType> void calculatePeaking();

        // ------------------------------------------------

    private:
        float m_Ca1a0[Voices];
        float m_Ca2a0[Voices];
        float m_Cb0a0[Voices];
        float m_Cb1a0[Voices];
        float m_Cb2a0[Voices];

        // ------------------------------------------------

        float m_X[3][Voices];
        float m_Y[3][Voices];

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
        Random m_Random{};
        std::size_t m_Counter = 0;
        float m_RandomFrequency = 0;

        // ------------------------------------------------

        ParallelFilter m_Filter[3]{};

        // ------------------------------------------------

        float m_Ratio = 0.99;
        float m_FrequencyModulation[Voices];

        // ------------------------------------------------

    };

    // ------------------------------------------------
    
    template<class SimdType>
    SimdType ParallelFilter::process(SimdType input, std::size_t i) {
        auto x0 = input;
        auto x1 = Kaixo::at<SimdType>(m_X[m_M1], i);
        auto x2 = Kaixo::at<SimdType>(m_X[m_M2], i);
        auto y1 = Kaixo::at<SimdType>(m_Y[m_M1], i);
        auto y2 = Kaixo::at<SimdType>(m_Y[m_M2], i);

        auto c0 = Kaixo::at<SimdType>(m_Cb0a0, i);
        auto c1 = Kaixo::at<SimdType>(m_Cb1a0, i);
        auto c2 = Kaixo::at<SimdType>(m_Cb2a0, i);
        auto c3 = Kaixo::at<SimdType>(m_Ca1a0, i);
        auto c4 = Kaixo::at<SimdType>(m_Ca2a0, i);

        auto result = c0 * x0 + c1 * x1 + c2 * x2
                                - c3 * y1 - c4 * y2;

        Kaixo::store<SimdType>(m_Y[m_M0] + i, result);
        Kaixo::store<SimdType>(m_X[m_M0] + i, input);
            
        return result;
    }

    // ------------------------------------------------
    
    template<class SimdType>
    void ParallelFilter::calculateLowpass() {
        constexpr std::size_t Count = sizeof(SimdType) / sizeof(float);
            
        for (std::size_t i = 0; i < Voices; i += Count) {
            auto freq = Math::clamp(Kaixo::at<SimdType>(frequency, i) / sampleRate(), SimdType(0.f), SimdType(0.5f));
            auto omega = 2 * std::numbers::pi * freq;
            auto cosOmega = Math::ncos(freq);
            auto sinOmega = Math::nsin(freq);
            auto qValue = Math::Fast::powN<4>(Kaixo::at<SimdType>(q, i));
            auto alpha = sinOmega / (qValue * 12 + 0.8);

            auto a0 = 1.0 + alpha;
            auto a1 = -2.0 * cosOmega;
            auto a2 = 1.0 - alpha;
            auto b0 = (1.0 - cosOmega) / 2.0;
            auto b1 = (1.0 - cosOmega);
            auto b2 = (1.0 - cosOmega) / 2.0;

            Kaixo::store<SimdType>(m_Ca1a0 + i, a1 / a0);
            Kaixo::store<SimdType>(m_Ca2a0 + i, a2 / a0);
            Kaixo::store<SimdType>(m_Cb0a0 + i, b0 / a0);
            Kaixo::store<SimdType>(m_Cb1a0 + i, b1 / a0);
            Kaixo::store<SimdType>(m_Cb2a0 + i, b2 / a0);
        }
    }

    template<class SimdType>
    void ParallelFilter::calculatePeaking() {
        constexpr std::size_t Count = sizeof(SimdType) / sizeof(float);
        constexpr float log10_2 = std::numbers::ln2 / std::numbers::ln10;

        for (std::size_t i = 0; i < Voices; i += Count) {
            auto freq = Math::clamp(Kaixo::at<SimdType>(frequency, i) / sampleRate(), SimdType(0.f), SimdType(0.5f));
            auto omega = 2 * std::numbers::pi * freq;
            auto cosOmega = Math::Fast::ncos(freq);
            auto sinOmega = Math::Fast::nsin(freq);
            auto qValue = Math::Fast::powN<4>(Kaixo::at<SimdType>(q, i));
            auto gainValue = Kaixo::at<SimdType>(gain, i);

            auto A = Math::Fast::pow(SimdType(10.f), gainValue / 40.0f);
            auto  alpha = sinOmega * Math::sinh((log10_2 / 2.0) * (qValue * 4 + 0.2) * (omega / sinOmega));

            auto a0 = 1.0 + alpha / A;
            auto a1 = -2.0 * cosOmega;
            auto a2 = 1.0 - alpha / A;
            auto b0 = 1.0 + alpha * A;
            auto b1 = -2.0 * cosOmega;
            auto b2 = 1.0 - alpha * A;

            // TODO: can probably cancel some stuff out here
            Kaixo::store<SimdType>(m_Ca1a0 + i, a1 / a0);
            Kaixo::store<SimdType>(m_Ca2a0 + i, a2 / a0);
            Kaixo::store<SimdType>(m_Cb0a0 + i, b0 / a0);
            Kaixo::store<SimdType>(m_Cb1a0 + i, b1 / a0);
            Kaixo::store<SimdType>(m_Cb2a0 + i, b2 / a0);
        }
    }

    // ------------------------------------------------
    
    template<class SimdType>
    void CustomFilter::process() {
        constexpr std::size_t Count = sizeof(SimdType) / sizeof(float);

        // every 2 ms
        float timer = 2 * sampleRate() / 1000.;
        if (m_Counter++ > timer) {
            m_RandomFrequency = m_Random.next();
            m_Counter = 0;
        }

        for (std::size_t i = 0; i < Voices; i += Count) {
            auto noteValue = Kaixo::at<SimdType>(note, i);
            auto freqMod = Kaixo::at<SimdType>(m_FrequencyModulation, i) * m_Ratio + 
                            Kaixo::at<SimdType>(frequencyModulation, i) * (1 - m_Ratio);
            Kaixo::store(m_FrequencyModulation + i, freqMod);

            auto freqValue = Math::Fast::magnitude_to_log(params.frequency + freqMod, SimdType(16.f), SimdType(16000.f));

            if (params.keytrack) {
                freqValue = Math::Fast::clamp(freqValue * Math::Fast::exp2((noteValue - 60) / 12.), SimdType(16.f), SimdType(16000.f));
            } else {
                freqValue = Math::Fast::clamp(freqValue, SimdType(16.f), SimdType(16000.f));
            }

            auto nfreq = (freqValue / 16000);
            auto randRange = 24 * (1 - (1 - params.drive) * (1 - params.drive)) + 6 * (1 - nfreq * nfreq);
            auto frequency = Math::Fast::clamp(freqValue + m_RandomFrequency * randRange * 2 - randRange, SimdType(16.f), SimdType(16000.f));
            auto resonance = params.resonance * (1 - nfreq * nfreq * nfreq * nfreq);

            // Less resonance when low frequency
            resonance = Kaixo::iff<SimdType>(SimdType(0.01f) < nfreq, 
                [&] { return resonance * 0.2f + 0.8f * (nfreq / 0.01f); },
                [&] { return resonance; });

            Kaixo::store<SimdType>(m_Filter[0].frequency + i, frequency);
            Kaixo::store<SimdType>(m_Filter[0].q, resonance);
            Kaixo::store<SimdType>(m_Filter[1].frequency, frequency * 0.9);
            Kaixo::store<SimdType>(m_Filter[1].q, resonance * 0.2 + 0.2);
            Kaixo::store<SimdType>(m_Filter[1].gain, params.drive * 12 - resonance * 15);
            Kaixo::store<SimdType>(m_Filter[2].frequency, frequency * 1.1);
            Kaixo::store<SimdType>(m_Filter[2].q, 0.2 - resonance * 0.2);
            Kaixo::store<SimdType>(m_Filter[2].gain, resonance * 15 - params.drive * 12);

            m_Filter[0].calculateLowpass<SimdType>();
            m_Filter[1].calculatePeaking<SimdType>();
            m_Filter[2].calculatePeaking<SimdType>();

            auto drive = Math::Fast::db_to_magnitude(params.drive * 12);

            SimdType res{};
            if (params.oversample() == 1) {
                auto inputValue = Kaixo::at<SimdType>(input[0], i);
                res = params.drive * Math::Fast::tanh_like(inputValue * drive) + inputValue * (1 - params.drive);
            } else {
                // TODO: aaf
                //m_AAF.sampleRateIn = sampleRate() * params.oversample();
                //m_AAF.sampleRateOut = sampleRate();

                for (std::size_t j = 0; j < params.oversample(); ++j) {
                    auto inputValue = Kaixo::at<SimdType>(input[j], i);
                    inputValue = params.drive * Math::Fast::tanh_like(inputValue * drive) + inputValue * (1 - params.drive);
                    //res = m_AAF.process(input[i]);
                    res = inputValue;
                }
            }

            auto filterOutput = m_Filter[2].process(m_Filter[1].process(m_Filter[0].process(res, i), i), i);
            filterOutput = params.drive * Math::Fast::tanh_like(1.115 * filterOutput) + filterOutput * (1 - 0.9 * params.drive);
            Kaixo::store<SimdType>(output + i, filterOutput);
        }
    }

    // ------------------------------------------------

}

// ------------------------------------------------
