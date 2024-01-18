
// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Controller.hpp"

// ------------------------------------------------

#include "Kaixo/Core/Theme/Theme.hpp"

// ------------------------------------------------

namespace Kaixo {

    // ------------------------------------------------

    MiniSynthFMController::MiniSynthFMController() {

        // ------------------------------------------------

        Gui::T.initialize();

        // ------------------------------------------------
        
        data<ControllerData>();

        // ------------------------------------------------
        
        linkPitchWheel(Synth.pitchBendParameter);
        linkModWheel(Synth.modWheelParameter);

        // ------------------------------------------------

    }

    // ------------------------------------------------

    Controller* createController() { return new MiniSynthFMController; }

    // ------------------------------------------------

}
    
// ------------------------------------------------
