
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

        add<ImageView>({ .image = T.lfo.background })
            .description("LFO with control over basic waveform, including (quantized) noise, tempo (can be synced to BPM), and depth.");

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
        
        watch<bool>([&, i] { 
            return context.param(Synth.lfo[i].synced) > 0.5;
        }, [&](bool val) {
            frequencyTempo.select(val);
        });

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
        }).description("Route this output jack to any input jack to use the LFO as a modulation source.");
            
        add<Jack>({ 207, 21, 64, 52 }, {
            .graphics = T.lfo.jacks.depth,
            .type = Jack::Type::Input,
            .patchBay = settings.patchBay,
            .destination = ModDestination::LfoDepth,
            .name = "Depth",
        }).description("Routing to this input jack will modulate the depth of the LFO.");

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
