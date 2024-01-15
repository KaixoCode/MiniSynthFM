
// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Gui/MainView.hpp"

// ------------------------------------------------

#include "Kaixo/Core/Gui/Views/ImageView.hpp"
#include "Kaixo/Core/Gui/Views/ScrollView.hpp"
#include "Kaixo/Core/Gui/Views/TextView.hpp"
#include "Kaixo/Core/Gui/Views/FilterDisplay.hpp"
#include "Kaixo/Core/Gui/Knob.hpp"
#include "Kaixo/Core/Gui/Button.hpp"
#include "Kaixo/Core/Theme/Theme.hpp"

// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Controller.hpp"
#include "Kaixo/MiniSynthFM/Gui/PatchBay.hpp"
#include "Kaixo/MiniSynthFM/Gui/Led.hpp"
#include "Kaixo/MiniSynthFM/Processing/Processor.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    MainView::MainView(Context c)
        : View(c)
    {
        // ------------------------------------------------

        wantsIdle(true);

        // ------------------------------------------------
        
        context.tooltip().background(T.tooltip.background);
        context.tooltip().font(T.tooltip.font);
        context.tooltip().textColor(T.tooltip.textColor);

        // ------------------------------------------------

        add<ImageView>({ T.background });

        // ------------------------------------------------

        add<TextView>({ 300, 150, 100, 100 }, {
            .graphics = T.text
        });

        // ------------------------------------------------
        
        auto& patchBay = add<PatchBay>();
        
        // ------------------------------------------------

        add<Button>({ 411, 33, 75, 23 }, {
            .graphics = T.toggle,
            .behaviour = Button::Behaviour::Toggle,
            .param = Synth.filter.enable
        });

        add<Knob>({ 353, 64, 64, 64 }, {
            .graphics = T.knob,
            .tooltipName = false,
            .tooltipValue = false,
            .param = Synth.filter.frequency
        });

        add<Jack>({ 353, 140, 64, 52 }, {
            .graphics = T.inputJack,
            .type = Jack::Type::Input,
            .patchBay = patchBay,
            .destination = ModDestination::FilterFreq,
            .name = "Cutoff"
        });

        add<Knob>({ 422, 64, 64, 64 }, {
            .graphics = T.knob,
            .tooltipName = false,
            .tooltipValue = false,
            .param = Synth.filter.resonance
        });
        
        add<Knob>({ 422, 128, 64, 64 }, {
            .graphics = T.knob,
            .tooltipName = false,
            .tooltipValue = false,
            .param = Synth.filter.drive
        });

        // ------------------------------------------------

        for (std::size_t i = 0; i < Oscillators; ++i) {

            // ------------------------------------------------

            add<Knob>({ 37 + i * 280, 244, 64, 64 }, {
                .graphics = T.knob,
                .tooltipName = false,
                .tooltipValue = false,
                .param = Synth.oscillator[i].volume
            });
            
            add<Knob>({ 37 + i * 280, 308, 64, 64 }, {
                .graphics = T.knob,
                .tooltipName = false,
                .tooltipValue = false,
                .param = Synth.oscillator[i].wave
            });

            add<Knob>({ 103 + i * 280, 308, 64, 64 }, {
                .graphics = T.knob,
                .tooltipName = false,
                .tooltipValue = false,
                .param = Synth.oscillator[i].fm
            });
            
            add<Knob>({ 103 + i * 280, 244, 64, 64 }, {
                .graphics = T.knob,
                .tooltipName = false,
                .tooltipValue = false,
                .param = Synth.oscillator[i].tune
            });
            
            add<Jack>({ 169 + i * 280, 256, 64, 52 }, {
                .graphics = T.inputJack,
                .type = Jack::Type::Input,
                .patchBay = patchBay,
                .destination = i == 0 ? ModDestination::Op1FM
                             : i == 1 ? ModDestination::Op2FM
                             : ModDestination::Op3FM,
                .name = "FM"
            });
            
            add<Jack>({ 169 + i * 280, 320, 64, 52 }, {
                .graphics = T.inputJack,
                .type = Jack::Type::Input,
                .patchBay = patchBay,
                .destination = i == 0 ? ModDestination::Op1Amount
                             : i == 1 ? ModDestination::Op2Amount
                             :          ModDestination::Op3Amount,
                .name = "Amount"
            });

            add<Jack>({ 235 + i * 280, 256, 64, 52 }, {
                .graphics = T.inputJack,
                .type = Jack::Type::Input,
                .patchBay = patchBay,
                .destination = i == 0 ? ModDestination::Op1Sync
                             : i == 1 ? ModDestination::Op2Sync
                             : ModDestination::Op3Sync,
                .name = "Sync"
            });
            
            add<Jack>({ 235 + i * 280, 320, 64, 52 }, {
                .graphics = T.outputJack,
                .type = Jack::Type::Output,
                .patchBay = patchBay,
                .source = i == 0 ? ModSource::Op1
                        : i == 1 ? ModSource::Op2
                        :          ModSource::Op3
            });

            // ------------------------------------------------
            
            add<Button>({ 215 + i * 280, 213, 80, 23 }, {
                .graphics = T.toggle,
                .behaviour = Button::Behaviour::Toggle,
                .param = Synth.oscillator[i].output
            });

            // ------------------------------------------------
            
            add<Knob>({ 92 + i * 280, 213, 80, 23 }, {
                .graphics = T.threewayToggle,
                .type = Knob::Type::Both,
                .tooltipName = false,
                .tooltipValue = false,
                .param = Synth.oscillator[i].octave
            });

            // ------------------------------------------------

        }

        // ------------------------------------------------
        
        for (std::size_t i = 0; i < Lfos; ++i) {

            // ------------------------------------------------

            add<Knob>({ 195, 64, 64, 64 }, {
                .graphics = T.knob,
                .tooltipName = false,
                .tooltipValue = false,
                .param = Synth.lfo[i].depth
            });
            
            add<Knob>({ 264, 64, 64, 64 }, {
                .graphics = T.knob,
                .tooltipName = false,
                .tooltipValue = false,
                .param = Synth.lfo[i].time
            });

            add<Knob>({ 195, 128, 64, 64 }, {
                .graphics = T.knob,
                .tooltipName = false,
                .tooltipValue = false,
                .param = Synth.lfo[i].waveform
            });

            // ------------------------------------------------

            add<Button>({ 239, 33, 75, 23 }, {
                .graphics = T.toggle,
                .behaviour = Button::Behaviour::Toggle,
                .param = Synth.lfo[i].synced
            });

            // ------------------------------------------------

            add<Jack>({ 264, 140, 64, 52 }, {
                .graphics = T.outputJack,
                .type = Jack::Type::Output,
                .patchBay = patchBay,
                .source = ModSource::LFO
            });

            // ------------------------------------------------
            
            add<Led>({ 315, 38, 13, 13 }, {
                .graphics = T.led,
                .value = context.interface<Processing::LfoInterface>({ .index = i })
            });

            // ------------------------------------------------

        }

        // ------------------------------------------------
        
        for (std::size_t i = 0; i < Envelopes; ++i) {
            add<Knob>({ 880, 71 + i * 180, 64, 64 }, {
                .graphics = T.knob,
                .tooltipName = false,
                .tooltipValue = false,
                .param = Synth.envelope[i].level
            });
                        
            add<Knob>({ 957, 52 + i * 180, 60, 140 }, {
                .graphics = T.slider,
                .tooltipName = false,
                .tooltipValue = false,
                .param = Synth.envelope[i].attack
            });

            add<Knob>({ 1017, 52 + i * 180, 60, 140 }, {
                .graphics = T.slider,
                .tooltipName = false,
                .tooltipValue = false,
                .param = Synth.envelope[i].decay
            });

            add<Knob>({ 1077, 52 + i * 180, 60, 140 }, {
                .graphics = T.slider,
                .tooltipName = false,
                .tooltipValue = false,
                .param = Synth.envelope[i].sustain
            });

            add<Knob>({ 1137, 52 + i * 180, 60, 140 }, {
                .graphics = T.slider,
                .tooltipName = false,
                .tooltipValue = false,
                .param = Synth.envelope[i].release
            });
            
            // ------------------------------------------------
            
            add<Jack>({ 880, 140 + i * 180, 64, 52 }, {
                .graphics = T.outputJack,
                .type = Jack::Type::Output,
                .patchBay = patchBay,
                .source = i == 0 ? ModSource::Envelope1 : ModSource::Envelope2
            });
            
            // ------------------------------------------------
            
            add<Button>({ 942, 33 + i * 180, 80, 23 }, {
                .graphics = T.toggle,
                .behaviour = Button::Behaviour::Toggle,
                .param = Synth.envelope[i].loop
            });
            
            // ------------------------------------------------

            add<Led>({ 1182, 38 + i * 180, 13, 13 }, {
                .graphics = T.led,
                .value = context.interface<Processing::EnvelopeInterface>({ .index = i })
            });

            // ------------------------------------------------

        }

        // ------------------------------------------------
        
        // Move patch bay to end of views, so it draws on top
        removeChildComponent(&patchBay);
        addChildComponent(&patchBay);

        // ------------------------------------------------
        
    }

    // ------------------------------------------------

    View* createBaseView(Context context) { return new MainView{ context }; }

    // ------------------------------------------------

}
    
// ------------------------------------------------
