#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Gui/View.hpp"
#include "Kaixo/Core/Gui/TabControl.hpp"

// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Controller.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    class MainView : public View {
    public:

        // ------------------------------------------------

        MainView(Context c);

        // ------------------------------------------------
        
        TabControl lfoFrequencyTempo[Lfos]{};

        // ------------------------------------------------

    };
    
    // ------------------------------------------------

}
    
// ------------------------------------------------
