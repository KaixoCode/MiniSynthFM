
// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Processing/CustomFilter.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    std::size_t FilterParameters::oversample() const {
        switch (quality) {
        case Quality::Low: return 1;
        case Quality::Normal: return 2;
        case Quality::High: return 4;
        case Quality::Ultra: return 8;
        case Quality::Extreme: return 16;
        default: return 1;
        }
    }

    // ------------------------------------------------
    
    void ParallelFilter::finalize() {
        auto backup = m_M2;
        m_M2 = m_M1;
        m_M1 = m_M0;
        m_M0 = backup;
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
