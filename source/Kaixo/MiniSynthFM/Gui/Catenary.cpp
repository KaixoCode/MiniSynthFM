
// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Gui/Catenary.hpp"

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    Catenary::Catenary(float x0, float y0, float x1, float y1, float addLength, int numIterations) {
        assert(x0 != x1);
        assert(addLength > 0.f);
        // make sure x1 is right
        if (x0 > x1) {
            std::swap(x0, x1);
            std::swap(y0, y1);
        }

        auto x_d = x1 - x0;
        auto y_d = y1 - y0;

        auto d = std::hypot(x_d, y_d);
        auto L = d + addLength;

        auto x_f = std::sqrt(L * L - y_d * y_d) / x_d; // always > 1

        // need to solve sinh(xi)/xi - x_f = 0

        //  auto xi_approx = 1.15f*std::log(x_f - 0.8f) + 1.9f; // manual fit for xi>2, guaranteed too low.
        auto xi_approx = 1.16f * std::log(x_f - 0.75f) + 1.9f; // manual fit for large 2<xi<10..
        if (xi_approx < 2.f) {
            // taylor-approx: 1 + xi^2/6 + xi^4/120 + xi^6/5040 - x_f = 0
            // use inverse taylor series for smallish values. Always slightly too high.
            auto tmp = std::cbrt(std::sqrt(15680.f) * std::sqrt(x_f * (405.f * x_f + 198.f) + 62.f) + 2520.f * x_f + 616.f);
            xi_approx = std::sqrt(tmp - 84.f / tmp - 14.f);
        }

        auto xi = xi_approx;
        for (auto n = 0; n < numIterations; ++n) { // newton iterations to improve precision
            auto x = xi;
            auto exp_half = 0.5f * std::exp(x); // sinh and cosh at the same time from exp
            auto iexp_half = 0.25f / exp_half;
            auto sinhx = exp_half - iexp_half;
            auto coshx = exp_half + iexp_half;
            auto val = sinhx / x - x_f;
            auto der = (coshx * x - sinhx) / (x * x);
            xi -= val / der;
        }

        a = 0.5f * x_d / xi;
        inva = 1.f / a;
        b = 0.5f * (x0 + x1) - a * std::asinh(0.5f * y_d / (a * std::sinh(xi)));
        c = y0 - (a * std::cosh((x0 - b) * inva));
    }

    // ------------------------------------------------

    float Catenary::calcY(float x) const {
        auto y = a * std::cosh((x - b) * inva) + c;
        return y;
    }

    // ------------------------------------------------

}

// ------------------------------------------------
