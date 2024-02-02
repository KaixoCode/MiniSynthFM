
// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Gui/Panels/EnvelopePanel.hpp"

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

    EnvelopePanel::EnvelopePanel(Context c, Settings s)
        : View(c), settings(std::move(s)) 
    {

        // ------------------------------------------------

        add<ImageView>({ .image = T.envelope.background });

        // ------------------------------------------------
            
        std::size_t i = settings.index;

        // ------------------------------------------------

        add<Knob>({ 5, 41, 64, 64 }, {
            .graphics = T.envelope.parameters.level,
            .tooltipName = false,
            .tooltipValue = false,
            .param = Synth.envelope[i].level
        });
            
        add<Knob>({ 82, 22, 60, 140 }, {
            .graphics = T.envelope.parameters.attack,
            .tooltipName = false,
            .tooltipValue = false,
            .param = Synth.envelope[i].attack
        });

        add<Knob>({ 142, 22, 60, 140 }, {
            .graphics = T.envelope.parameters.decay,
            .tooltipName = false,
            .tooltipValue = false,
            .param = Synth.envelope[i].decay
        });

        add<Knob>({ 202, 22, 60, 140 }, {
            .graphics = T.envelope.parameters.sustain,
            .tooltipName = false,
            .tooltipValue = false,
            .param = Synth.envelope[i].sustain
        });

        add<Knob>({ 262, 22, 60, 140 }, {
            .graphics = T.envelope.parameters.release,
            .tooltipName = false,
            .tooltipValue = false,
            .param = Synth.envelope[i].release
        });
            
        // ------------------------------------------------
            
        add<Jack>({ 5, 110, 64, 52 }, {
            .graphics = T.envelope.jacks.output,
            .type = Jack::Type::Output,
            .patchBay = settings.patchBay,
            .source = i == 0 ? ModSource::Envelope1 : ModSource::Envelope2
        });
            
        // ------------------------------------------------
            
        add<Button>({ 67, 3, 80, 23 }, {
            .graphics = T.envelope.parameters.loop,
            .behaviour = Button::Behaviour::Toggle,
            .param = Synth.envelope[i].loop
        });
            
        // ------------------------------------------------

        add<Led>({ 307, 8, 13, 13 }, {
            .graphics = T.envelope.led,
            .value = context.interface<Processing::EnvelopeInterface>({ .index = i })
        });

        // ------------------------------------------------

        add<ImageView>({ .image = T.envelope.foreground, .enableMouse = false });

        // ------------------------------------------------

    }

    // ------------------------------------------------

}

// ------------------------------------------------
