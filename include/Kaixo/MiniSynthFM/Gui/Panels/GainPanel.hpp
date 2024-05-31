
// ------------------------------------------------

#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Gui/View.hpp"

// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Gui/PatchBay.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    class GainPanel : public View {
    public:

        // ------------------------------------------------

        struct Settings {

            // ------------------------------------------------

            PatchBay& patchBay;

            // ------------------------------------------------

        } settings;

        // ------------------------------------------------
        
        GainPanel(Context c, Settings s);

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------
