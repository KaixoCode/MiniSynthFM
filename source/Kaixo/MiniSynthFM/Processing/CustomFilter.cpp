
// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Processing/CustomFilter.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------
    
    FilterParameters::FilterParameters(Quality& q)
        : quality(q) 
    {}

    // ------------------------------------------------
    
    bool ParallelAntiAliasFilter::Settings::operator==(const Settings& other) const {
        return sampleRateIn == other.sampleRateIn
            && sampleRateOut == other.sampleRateOut
            && passbandAmplitudedB == other.passbandAmplitudedB
            && stopbandAmplitudedB == other.stopbandAmplitudedB
            && normalisedTransitionWidth == other.normalisedTransitionWidth;
    }

    void ParallelAntiAliasFilter::recalculate() {
        if (m_Settings == settings) return;
        m_Settings = settings;

        float sampleRate = m_Settings.sampleRateIn;
        float filterFrequency = m_Settings.sampleRateOut / 2 - 10; // 10 Hz extra headroom

        auto normalisedFrequency = filterFrequency / sampleRate;

        auto fp = normalisedFrequency - m_Settings.normalisedTransitionWidth / 2;
        auto fs = normalisedFrequency + m_Settings.normalisedTransitionWidth / 2;

        double Ap = m_Settings.passbandAmplitudedB;
        double As = m_Settings.stopbandAmplitudedB;
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

    void ParallelAntiAliasFilter::reset() {
        for (auto& coefficient : coefficients) {
            std::memset(coefficient.state, sizeof(coefficient.state), 0);
        }
    }

    // ------------------------------------------------
    
    void ParallelFilter::finalize() {
        auto backup = m_M2;
        m_M2 = m_M1;
        m_M1 = m_M0;
        m_M0 = backup;
    }

    void ParallelFilter::reset() {
        std::memset(m_X, sizeof(m_X), 0);
        std::memset(m_Y, sizeof(m_Y), 0);
    }

    // ------------------------------------------------

    CustomFilter::CustomFilter(FilterParameters& p)
        : params(p)
    {
        for (auto& f : m_Filter)
            registerModule(f);
    }

    // ------------------------------------------------

    void CustomFilter::prepare(double sampleRate, std::size_t maxBufferSize) {
        m_Ratio = Math::smoothCoef(0.99, 96000 / sampleRate);
    }

    void CustomFilter::reset() {
        ModuleContainer::reset();
        //m_AAF.reset();
    }

    // ------------------------------------------------

}

// ------------------------------------------------
