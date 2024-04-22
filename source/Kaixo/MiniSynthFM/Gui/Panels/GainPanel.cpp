
// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Gui/Panels/GainPanel.hpp"

// ------------------------------------------------

#include "Kaixo/Core/Gui/Views/ImageView.hpp"
#include "Kaixo/Core/Gui/Knob.hpp"
#include "Kaixo/Core/Gui/Button.hpp"
#include "Kaixo/Core/Theme/Theme.hpp"

// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Controller.hpp"
#include "Kaixo/MiniSynthFM/Gui/Led.hpp"
#include "Kaixo/MiniSynthFM/Processing/Interfaces.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    GainPanel::GainPanel(Context c, Settings s)
        : View(c), settings(std::move(s))
    {

        // ------------------------------------------------

        add<ImageView>({ .image = T.gain.background })
            .description("AD envelope that controls the main output level of MiniFM. Can also be used as a modulation source.");

        // ------------------------------------------------

        add<Knob>({ 66, 9, 64, 64 }, {
            .graphics = T.gain.parameters.level,
            .tooltipName = false,
            .tooltipValue = false,
            .param = Synth.gain.level
        });
                        
        add<Knob>({ 131, 9, 64, 64 }, {
            .graphics = T.gain.parameters.attack,
            .tooltipName = false,
            .tooltipValue = false,
            .param = Synth.gain.attack
        });

        add<Knob>({ 196, 9, 64, 64 }, {
            .graphics = T.gain.parameters.decay,
            .tooltipName = false,
            .tooltipValue = false,
            .param = Synth.gain.decay
        });

        // ------------------------------------------------
            
        add<Jack>({ 261, 21, 64, 52 }, {
            .graphics = T.gain.jacks.output,
            .type = Jack::Type::Output,
            .patchBay = settings.patchBay,
            .source = ModSource::Envelope3
        }).description("Route this output jack to any input jack to use this envelope as a modulation source.");
            
        // ------------------------------------------------
            
        add<Button>({ 1, 27, 64, 46 }, {
            .graphics = T.gain.parameters.gate,
            .behaviour = Button::Behaviour::Toggle,
            .param = Synth.gain.gate
        });
            
        // ------------------------------------------------

        add<Led>({ 307, 8, 13, 13 }, {
            .graphics = T.gain.led,
            .value = context.interface<Processing::EnvelopeInterface>({ .index = 2 })
        });

        // ------------------------------------------------

        add<ImageView>({ .image = T.gain.foreground, .enableMouse = false });

        // ------------------------------------------------

    }

    // ------------------------------------------------

}

// ------------------------------------------------
