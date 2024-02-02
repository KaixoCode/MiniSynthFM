#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Gui/View.hpp"

// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Gui/PatchBay.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    class MidiPanel : public View {
    public:

        // ------------------------------------------------

        struct Settings {

            // ------------------------------------------------

            PatchBay& patchBay;

            // ------------------------------------------------

        } settings;

        // ------------------------------------------------

        MidiPanel(Context c, Settings s);

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------