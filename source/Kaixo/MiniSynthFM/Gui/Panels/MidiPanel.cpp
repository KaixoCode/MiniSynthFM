
// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Gui/Panels/MidiPanel.hpp"

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

    MidiPanel::MidiPanel(Context c, Settings s)
        : View(c), settings(std::move(s))
    {

        // ------------------------------------------------
        
        add<Knob>({ 2, 9, 64, 64 }, {
            .graphics = T.midi.parameters.modWheelAmount,
            .tooltipName = false,
            .tooltipValue = false,
            .param = Synth.modWheelAmount
        });
        
        add<Jack>({ 67, 21, 64, 52 }, {
            .graphics = T.midi.jacks.modWheel,
            .type = Jack::Type::Output,
            .patchBay = settings.patchBay,
            .source = ModSource::ModWheel,
            .name = "Mod",
        }).description("Route this output jack to any input jack to use the mod wheel as a modulation source.");
        
        add<Led>({ 115, 8, 13, 13 }, {
            .graphics = T.envelope.led,
            .value = context.interface<Processing::ModWheelInterface>()
        });

        add<Knob>({ 134, 9, 64, 64 }, {
            .graphics = T.midi.parameters.velocityAmount,
            .tooltipName = false,
            .tooltipValue = false,
            .param = Synth.velocityAmount
        });
        
        add<Jack>({ 199, 21, 64, 52 }, {
            .graphics = T.midi.jacks.velocity,
            .type = Jack::Type::Output,
            .patchBay = settings.patchBay,
            .source = ModSource::Velocity,
            .name = "Vel",
        }).description("Route this output jack to any input jack to use the velocity as a modulation source.");
        
        add<Led>({ 247, 8, 13, 13 }, {
            .graphics = T.envelope.led,
            .value = context.interface<Processing::VelocityInterface>()
        });

        add<Knob>({ 266, 9, 64, 64 }, {
            .graphics = T.midi.parameters.randomAmount,
            .tooltipName = false,
            .tooltipValue = false,
            .param = Synth.randomAmount
        });
        
        add<Jack>({ 331, 21, 64, 52 }, {
            .graphics = T.midi.jacks.random,
            .type = Jack::Type::Output,
            .patchBay = settings.patchBay,
            .source = ModSource::Random,
            .name = "Rand",
        }).description("This output jack will generate a random output every key press to be used as a modulation source.");
        
        add<Led>({ 379, 8, 13, 13 }, {
            .graphics = T.envelope.led,
            .value = context.interface<Processing::RandomInterface>()
        });

        // ------------------------------------------------

    }

    // ------------------------------------------------

}

// ------------------------------------------------
