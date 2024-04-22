
// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Gui/Panels/FilterPanel.hpp"

// ------------------------------------------------

#include "Kaixo/Core/Gui/Views/ImageView.hpp"
#include "Kaixo/Core/Gui/Knob.hpp"
#include "Kaixo/Core/Gui/Button.hpp"
#include "Kaixo/Core/Theme/Theme.hpp"

// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Controller.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    FilterPanel::FilterPanel(Context c, Settings s)
        : View(c), settings(std::move(s))
    {

        // ------------------------------------------------
        
        add<ImageView>({ .image = T.filter.background })
            .description("Lowpass filter with control over the cutoff and resonance. Adjust the drive knob to add some distortion.");

        // ------------------------------------------------

        add<Button>({ 63, 3, 75, 23 }, {
            .graphics = T.filter.parameters.keytrack,
            .behaviour = Button::Behaviour::Toggle,
            .param = Synth.filter.keyTrack
        });

        add<Knob>({ 4, 32, 64, 64 }, {
            .graphics = T.filter.parameters.cutoff,
            .tooltipName = false,
            .tooltipValue = false,
            .param = Synth.filter.cutoff
        });

        add<Jack>({ 4, 109, 64, 52 }, {
            .graphics = T.filter.jacks.cutoff,
            .type = Jack::Type::Input,
            .patchBay = settings.patchBay,
            .destination = ModDestination::FilterFreq,
            .name = "Cutoff"
        }).description("Routing to this input jack will modulate the cutoff frequency of the lowpass filter.");

        add<Knob>({ 76, 32, 64, 64 }, {
            .graphics = T.filter.parameters.resonance,
            .tooltipName = false,
            .tooltipValue = false,
            .param = Synth.filter.resonance
        });
        
        add<Knob>({ 76, 97, 64, 64 }, {
            .graphics = T.filter.parameters.drive,
            .tooltipName = false,
            .tooltipValue = false,
            .param = Synth.filter.drive
        });

        // ------------------------------------------------

        add<ImageView>({ .image = T.filter.foreground, .enableMouse = false });

        // ------------------------------------------------

    }

    // ------------------------------------------------

}

// ------------------------------------------------
