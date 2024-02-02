
// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Gui/Panels/LfoPanel.hpp"

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

    LfoPanel::LfoPanel(Context c, Settings s)
        : View(c), settings(std::move(s)) 
    {

        // ------------------------------------------------

        add<ImageView>({ .image = T.lfo.background });

        // ------------------------------------------------
            
        std::size_t i = settings.index;

        // ------------------------------------------------
            
        add<Knob>({ 272, 9, 64, 64 }, {
            .graphics = T.lfo.parameters.depth,
            .tooltipName = false,
            .tooltipValue = false,
            .param = Synth.lfo[i].depth
        });
            
        frequencyTempo.add(0, add<Knob>({ 67, 9, 64, 64 }, {
            .graphics = T.lfo.parameters.frequency,
            .tooltipName = false,
            .tooltipValue = false,
            .param = Synth.lfo[i].frequency
        }));

        frequencyTempo.add(1, add<Knob>({ 67, 9, 64, 64 }, {
            .graphics = T.lfo.parameters.tempo,
            .tooltipName = false,
            .tooltipValue = false,
            .param = Synth.lfo[i].tempo
        }));

        frequencyTempo.select(1);

        add<Knob>({ 137, 9, 64, 64 }, {
            .graphics = T.lfo.parameters.waveform,
            .tooltipName = false,
            .tooltipValue = false,
            .param = Synth.lfo[i].waveform
        });

        // ------------------------------------------------

        add<Button>({ 2, 27, 64, 46 }, {
            .callback = [&, i](bool val) {
                frequencyTempo.select(val);
            },
            .graphics = T.lfo.parameters.synced,
            .behaviour = Button::Behaviour::Toggle,
            .param = Synth.lfo[i].synced
        });

        // ------------------------------------------------

        add<Jack>({ 339, 21, 64, 52 }, {
            .graphics = T.lfo.jacks.output,
            .type = Jack::Type::Output,
            .patchBay = settings.patchBay,
            .source = ModSource::LFO
        });
            
        add<Jack>({ 207, 21, 64, 52 }, {
            .graphics = T.lfo.jacks.depth,
            .type = Jack::Type::Input,
            .patchBay = settings.patchBay,
            .destination = ModDestination::LfoDepth,
            .name = "Depth",
        });

        // ------------------------------------------------
            
        add<Led>({ 385, 8, 13, 13 }, {
            .graphics = T.lfo.led,
            .value = context.interface<Processing::LfoInterface>({ .index = i })
        });

        // ------------------------------------------------

        add<ImageView>({ .image = T.lfo.foreground, .enableMouse = false });

        // ------------------------------------------------

    }

    // ------------------------------------------------

}

// ------------------------------------------------