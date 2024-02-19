
// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Gui/Panels/DelayPanel.hpp"

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

    DelayPanel::DelayPanel(Context c, Settings s)
        : View(c), settings(std::move(s))
    {

        // ------------------------------------------------
        
        add<ImageView>({ .image = T.delay.background })
            .description("Delay effect");

        // ------------------------------------------------
        
        add<Knob>({ 76, 97, 64, 64 }, {
            .graphics = T.delay.parameters.mix,
            .tooltipName = false,
            .tooltipValue = false,
            .param = Synth.delay.mix
        });

        timeTempo.add(0, add<Knob>({ 4, 32, 64, 64 }, {
            .graphics = T.delay.parameters.time,
            .tooltipName = false,
            .tooltipValue = false,
            .param = Synth.delay.time
        }));
        
        timeTempo.add(1, add<Knob>({ 4, 32, 64, 64 }, {
            .graphics = T.delay.parameters.tempo,
            .tooltipName = false,
            .tooltipValue = false,
            .param = Synth.delay.tempo
        }));

        timeTempo.select(1);

        add<Knob>({ 76, 32, 64, 64 }, {
            .graphics = T.delay.parameters.feedback,
            .tooltipName = false,
            .tooltipValue = false,
            .param = Synth.delay.feedback
        });
        
        add<Button>({ 65, 3, 73, 23 }, {
            .graphics = T.delay.parameters.pingpong,
            .behaviour = Button::Behaviour::Toggle,
            .param = Synth.delay.pingpong,
        });
        
        add<Button>({ 4, 115, 64, 46 }, {
            .callback = [&](bool v) {
                timeTempo.select(v);
            },
            .graphics = T.delay.parameters.synced,
            .behaviour = Button::Behaviour::Toggle,
            .param = Synth.delay.synced,
        });

        // ------------------------------------------------

        add<ImageView>({ .image = T.delay.foreground, .enableMouse = false });

        // ------------------------------------------------

    }

    // ------------------------------------------------

}

// ------------------------------------------------
