#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Gui/View.hpp"

// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Gui/PatchBay.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    class OperatorPanel : public View {
    public:

        // ------------------------------------------------

        struct Settings {

            // ------------------------------------------------

            std::size_t index;
            PatchBay& patchBay;

            // ------------------------------------------------

        } settings;

        // ------------------------------------------------

        OperatorPanel(Context c, Settings s);

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------