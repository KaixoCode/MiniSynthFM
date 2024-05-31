
// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Gui/Panels/OperatorPanel.hpp"

// ------------------------------------------------

#include "Kaixo/Core/Gui/Views/ImageView.hpp"
#include "Kaixo/Core/Gui/Knob.hpp"
#include "Kaixo/Core/Gui/Button.hpp"
#include "Kaixo/Core/Theme/Theme.hpp"

// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Controller.hpp"
#include "Kaixo/MiniSynthFM/Processing/Interfaces.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    OperatorPanel::OperatorPanel(Context c, Settings s) 
        : View(c), settings(std::move(s))
    {

        // ------------------------------------------------

        add<ImageView>({ .image = T.oscillator.background })
            .description("Operators are the source of the sound, route the output jack to the FM input of another operator to do frequency modulation.");

        // ------------------------------------------------

        std::size_t i = settings.index;

        // ------------------------------------------------

        add<Knob>({ 2, 34, 64, 64 }, {
            .graphics = T.oscillator.parameters.volume,
            .tooltipName = false,
            .tooltipValue = false,
            .param = Synth.oscillator[i].volume
        });
            
        add<Knob>({ 2, 98, 64, 64 }, {
            .graphics = T.oscillator.parameters.waveform,
            .tooltipName = false,
            .tooltipValue = false,
            .param = Synth.oscillator[i].waveform
        });

        add<Knob>({ 68, 98, 64, 64 }, {
            .graphics = T.oscillator.parameters.fm,
            .tooltipName = false,
            .tooltipValue = false,
            .param = Synth.oscillator[i].fm
        });
            
        add<Knob>({ 68, 34, 64, 64 }, {
            .graphics = T.oscillator.parameters.tune,
            .tooltipName = false,
            .tooltipValue = false,
            .param = Synth.oscillator[i].tune
        });
            
        add<Jack>({ 134, 46, 64, 52 }, {
            .graphics = T.oscillator.jacks.fm,
            .type = Jack::Type::Input,
            .patchBay = settings.patchBay,
            .destination = i == 0 ? ModDestination::Op1FM
                         : i == 1 ? ModDestination::Op2FM
                                  : ModDestination::Op3FM,
            .name = "FM"
        }).description("Routing to this input jack will modulate the frequency of the operator.");
            
        add<Jack>({ 134, 110, 64, 52 }, {
            .graphics = T.oscillator.jacks.amount,
            .type = Jack::Type::Input,
            .patchBay = settings.patchBay,
            .destination = i == 0 ? ModDestination::Op1Amount
                         : i == 1 ? ModDestination::Op2Amount
                                  : ModDestination::Op3Amount,
            .name = "Amount"
        }).description("Routing to this input jack will change how much the FM input jack influences the frequency of the operator.");

        add<Jack>({ 200, 46, 64, 52 }, {
            .graphics = T.oscillator.jacks.sync,
            .type = Jack::Type::Input,
            .patchBay = settings.patchBay,
            .destination = i == 0 ? ModDestination::Op1Sync
                         : i == 1 ? ModDestination::Op2Sync
                                  : ModDestination::Op3Sync,
            .name = "Sync"
        }).description("Route another operator to this input jack and play with this operator's pitch to get the classic hard-sync sound.");
            
        add<Jack>({ 200, 110, 64, 52 }, {
            .graphics = T.oscillator.jacks.output,
            .type = Jack::Type::Output,
            .patchBay = settings.patchBay,
            .source = i == 0 ? ModSource::Op1
                    : i == 1 ? ModSource::Op2
                             : ModSource::Op3
        }).description("Route this output jack to any input jack to use this operator's sound as a modulation source.");

        // ------------------------------------------------
        
        add<Button>({ 180, 3, 80, 23 }, {
            .graphics = T.oscillator.parameters.output,
            .behaviour = Button::Behaviour::Toggle,
            .param = Synth.oscillator[i].output
        });

        // ------------------------------------------------
            
        add<Knob>({ 57, 3, 80, 23 }, {
            .graphics = T.oscillator.parameters.octave,
            .type = Knob::Type::Both,
            .speed = 2,
            .tooltipName = false,
            .tooltipValue = false,
            .param = Synth.oscillator[i].octave
        });

        // ------------------------------------------------

        add<ImageView>({ .image = T.oscillator.foreground, .enableMouse = false });

        // ------------------------------------------------

    }

    // ------------------------------------------------

}

// ------------------------------------------------
