#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Gui/View.hpp"
#include "Kaixo/Core/Gui/TabControl.hpp"

// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Gui/PatchBay.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    class LfoPanel : public View {
    public:

        // ------------------------------------------------

        struct Settings {

            // ------------------------------------------------

            std::size_t index;
            PatchBay& patchBay;

            // ------------------------------------------------

        } settings;

        // ------------------------------------------------
        
        TabControl frequencyTempo;

        // ------------------------------------------------

        LfoPanel(Context c, Settings s);

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------
