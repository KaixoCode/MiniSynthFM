
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
#include "Kaixo/MiniSynthFM/Gui/Piano.hpp"
#include "Kaixo/MiniSynthFM/Gui/DisplayView.hpp"
#include "Kaixo/MiniSynthFM/Processing/Interfaces.hpp"

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

        add<DisplayView>({ 486, 30, 355, 165 });

        // ------------------------------------------------
        
        auto& patchBay = add<PatchBay>();
        
        // ------------------------------------------------

        add<Button>({ 253, 33, 75, 23 }, {
            .graphics = T.filter.parameters.keytrack,
            .behaviour = Button::Behaviour::Toggle,
            .param = Synth.filter.keyTrack
        });

        add<Knob>({ 195, 64, 64, 64 }, {
            .graphics = T.filter.parameters.cutoff,
            .tooltipName = false,
            .tooltipValue = false,
            .param = Synth.filter.cutoff
        });

        add<Jack>({ 195, 140, 64, 52 }, {
            .graphics = T.filter.jacks.cutoff,
            .type = Jack::Type::Input,
            .patchBay = patchBay,
            .destination = ModDestination::FilterFreq,
            .name = "Cutoff"
        });

        add<Knob>({ 264, 64, 64, 64 }, {
            .graphics = T.filter.parameters.resonance,
            .tooltipName = false,
            .tooltipValue = false,
            .param = Synth.filter.resonance
        });
        
        add<Knob>({ 264, 128, 64, 64 }, {
            .graphics = T.filter.parameters.drive,
            .tooltipName = false,
            .tooltipValue = false,
            .param = Synth.filter.drive
        });

        // ------------------------------------------------

        for (std::size_t i = 0; i < Oscillators; ++i) {

            // ------------------------------------------------

            add<Knob>({ 37 + i * 270, 234, 64, 64 }, {
                .graphics = T.oscillator.parameters.volume,
                .tooltipName = false,
                .tooltipValue = false,
                .param = Synth.oscillator[i].volume
            });
            
            add<Knob>({ 37 + i * 270, 298, 64, 64 }, {
                .graphics = T.oscillator.parameters.waveform,
                .tooltipName = false,
                .tooltipValue = false,
                .param = Synth.oscillator[i].waveform
            });

            add<Knob>({ 103 + i * 270, 298, 64, 64 }, {
                .graphics = T.oscillator.parameters.fm,
                .tooltipName = false,
                .tooltipValue = false,
                .param = Synth.oscillator[i].fm
            });
            
            add<Knob>({ 103 + i * 270, 234, 64, 64 }, {
                .graphics = T.oscillator.parameters.tune,
                .tooltipName = false,
                .tooltipValue = false,
                .param = Synth.oscillator[i].tune
            });
            
            add<Jack>({ 169 + i * 270, 246, 64, 52 }, {
                .graphics = T.oscillator.jacks.fm,
                .type = Jack::Type::Input,
                .patchBay = patchBay,
                .destination = i == 0 ? ModDestination::Op1FM
                             : i == 1 ? ModDestination::Op2FM
                             : ModDestination::Op3FM,
                .name = "FM"
            });
            
            add<Jack>({ 169 + i * 270, 310, 64, 52 }, {
                .graphics = T.oscillator.jacks.amount,
                .type = Jack::Type::Input,
                .patchBay = patchBay,
                .destination = i == 0 ? ModDestination::Op1Amount
                             : i == 1 ? ModDestination::Op2Amount
                             :          ModDestination::Op3Amount,
                .name = "Amount"
            });

            add<Jack>({ 235 + i * 270, 246, 64, 52 }, {
                .graphics = T.oscillator.jacks.sync,
                .type = Jack::Type::Input,
                .patchBay = patchBay,
                .destination = i == 0 ? ModDestination::Op1Sync
                             : i == 1 ? ModDestination::Op2Sync
                             : ModDestination::Op3Sync,
                .name = "Sync"
            });
            
            add<Jack>({ 235 + i * 270, 310, 64, 52 }, {
                .graphics = T.oscillator.jacks.output,
                .type = Jack::Type::Output,
                .patchBay = patchBay,
                .source = i == 0 ? ModSource::Op1
                        : i == 1 ? ModSource::Op2
                        :          ModSource::Op3
            });

            // ------------------------------------------------
            
            add<Button>({ 215 + i * 270, 203, 80, 23 }, {
                .graphics = T.oscillator.parameters.output,
                .behaviour = Button::Behaviour::Toggle,
                .param = Synth.oscillator[i].output
            });

            // ------------------------------------------------
            
            add<Knob>({ 92 + i * 270, 203, 80, 23 }, {
                .graphics = T.oscillator.parameters.octave,
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

            add<Knob>({ 710, 379, 64, 64 }, {
                .graphics = T.lfo.parameters.depth,
                .tooltipName = false,
                .tooltipValue = false,
                .param = Synth.lfo[i].depth
            });
            
            lfoFrequencyTempo[i].add(0, add<Knob>({ 515, 379, 64, 64 }, {
                .graphics = T.lfo.parameters.frequency,
                .tooltipName = false,
                .tooltipValue = false,
                .param = Synth.lfo[i].frequency
            }));

            lfoFrequencyTempo[i].add(1, add<Knob>({ 515, 379, 64, 64 }, {
                .graphics = T.lfo.parameters.tempo,
                .tooltipName = false,
                .tooltipValue = false,
                .param = Synth.lfo[i].tempo
            }));

            lfoFrequencyTempo[i].select(1);

            add<Knob>({ 580, 379, 64, 64 }, {
                .graphics = T.lfo.parameters.waveform,
                .tooltipName = false,
                .tooltipValue = false,
                .param = Synth.lfo[i].waveform
            });

            // ------------------------------------------------

            add<Button>({ 450, 397, 64, 46 }, {
                .callback = [&, i](bool val) {
                    lfoFrequencyTempo[i].select(val);
                },
                .graphics = T.lfo.parameters.synced,
                .behaviour = Button::Behaviour::Toggle,
                .param = Synth.lfo[i].synced
            });

            // ------------------------------------------------

            add<Jack>({ 775, 391, 64, 52 }, {
                .graphics = T.lfo.jacks.output,
                .type = Jack::Type::Output,
                .patchBay = patchBay,
                .source = ModSource::LFO
            });
            
            add<Jack>({ 645, 391, 64, 52 }, {
                .graphics = T.lfo.jacks.depth,
                .type = Jack::Type::Input,
                .patchBay = patchBay,
                .destination = ModDestination::LfoDepth,
                .name = "Depth",
            });

            // ------------------------------------------------
            
            add<Led>({ 822, 378, 13, 13 }, {
                .graphics = T.lfo.led,
                .value = context.interface<Processing::LfoInterface>({ .index = i })
            });

            // ------------------------------------------------

        }

        // ------------------------------------------------
        
        for (std::size_t i = 0; i < ADSREnvelopes; ++i) {
            add<Knob>({ 850, 71 + i * 170, 64, 64 }, {
                .graphics = T.envelope.parameters.level,
                .tooltipName = false,
                .tooltipValue = false,
                .param = Synth.envelope[i].level
            });
                        
            add<Knob>({ 927, 52 + i * 170, 60, 140 }, {
                .graphics = T.envelope.parameters.attack,
                .tooltipName = false,
                .tooltipValue = false,
                .param = Synth.envelope[i].attack
            });

            add<Knob>({ 987, 52 + i * 170, 60, 140 }, {
                .graphics = T.envelope.parameters.decay,
                .tooltipName = false,
                .tooltipValue = false,
                .param = Synth.envelope[i].decay
            });

            add<Knob>({ 1047, 52 + i * 170, 60, 140 }, {
                .graphics = T.envelope.parameters.sustain,
                .tooltipName = false,
                .tooltipValue = false,
                .param = Synth.envelope[i].sustain
            });

            add<Knob>({ 1107, 52 + i * 170, 60, 140 }, {
                .graphics = T.envelope.parameters.release,
                .tooltipName = false,
                .tooltipValue = false,
                .param = Synth.envelope[i].release
            });
            
            // ------------------------------------------------
            
            add<Jack>({ 850, 140 + i * 170, 64, 52 }, {
                .graphics = T.envelope.jacks.output,
                .type = Jack::Type::Output,
                .patchBay = patchBay,
                .source = i == 0 ? ModSource::Envelope1 : ModSource::Envelope2
            });
            
            // ------------------------------------------------
            
            add<Button>({ 912, 33 + i * 170, 80, 23 }, {
                .graphics = T.envelope.parameters.loop,
                .behaviour = Button::Behaviour::Toggle,
                .param = Synth.envelope[i].loop
            });
            
            // ------------------------------------------------

            add<Led>({ 1152, 38 + i * 170, 13, 13 }, {
                .graphics = T.envelope.led,
                .value = context.interface<Processing::EnvelopeInterface>({ .index = i })
            });

            // ------------------------------------------------

        }

        // ------------------------------------------------

        add<Knob>({ 911, 379, 64, 64 }, {
            .graphics = T.gain.parameters.level,
            .tooltipName = false,
            .tooltipValue = false,
            .param = Synth.gain.level
        });
                        
        add<Knob>({ 976, 379, 64, 64 }, {
            .graphics = T.gain.parameters.attack,
            .tooltipName = false,
            .tooltipValue = false,
            .param = Synth.gain.attack
        });

        add<Knob>({ 1041, 379, 64, 64 }, {
            .graphics = T.gain.parameters.decay,
            .tooltipName = false,
            .tooltipValue = false,
            .param = Synth.gain.decay
        });

        // ------------------------------------------------
            
        add<Jack>({ 1106, 391, 64, 52 }, {
            .graphics = T.gain.jacks.output,
            .type = Jack::Type::Output,
            .patchBay = patchBay,
            .source = ModSource::Envelope3
        });
            
        // ------------------------------------------------
            
        add<Button>({ 846, 397, 64, 46 }, {
            .graphics = T.gain.parameters.gate,
            .behaviour = Button::Behaviour::Toggle,
            .param = Synth.gain.gate
        });
            
        // ------------------------------------------------

        add<Led>({ 1152, 378, 13, 13 }, {
            .graphics = T.gain.led,
            .value = context.interface<Processing::EnvelopeInterface>({ .index = 2 })
        });
            
        // ------------------------------------------------
        
        add<Jack>({ 80, 408, 64, 52 }, {
            .graphics = T.outputJack,
            .type = Jack::Type::Output,
            .patchBay = patchBay,
            .source = ModSource::ModWheel,
            .name = "Mod",
        });

        add<Jack>({ 145, 408, 64, 52 }, {
            .graphics = T.outputJack,
            .type = Jack::Type::Output,
            .patchBay = patchBay,
            .source = ModSource::PitchBend,
            .name = "PB",
        });

        add<Jack>({ 210, 408, 64, 52 }, {
            .graphics = T.outputJack,
            .type = Jack::Type::Output,
            .patchBay = patchBay,
            .source = ModSource::Note,
            .name = "Note",
        });
        
        add<Jack>({ 275, 408, 64, 52 }, {
            .graphics = T.outputJack,
            .type = Jack::Type::Output,
            .patchBay = patchBay,
            .source = ModSource::Velocity,
            .name = "Vel",
        });
        
        add<Jack>({ 340, 408, 64, 52 }, {
            .graphics = T.outputJack,
            .type = Jack::Type::Output,
            .patchBay = patchBay,
            .source = ModSource::Random,
            .name = "Rand",
        });

        // ------------------------------------------------
        
        add<Knob>({ 412, 128, 64, 64 }, {
            .graphics = T.delay.parameters.mix,
            .tooltipName = false,
            .tooltipValue = false,
            .param = Synth.delay.mix
        });

        delayTimeTempo.add(0, add<Knob>({ 343, 64, 64, 64 }, {
            .graphics = T.delay.parameters.time,
            .tooltipName = false,
            .tooltipValue = false,
            .param = Synth.delay.time
        }));
        
        delayTimeTempo.add(1, add<Knob>({ 343, 64, 64, 64 }, {
            .graphics = T.delay.parameters.tempo,
            .tooltipName = false,
            .tooltipValue = false,
            .param = Synth.delay.tempo
        }));

        delayTimeTempo.select(1);

        add<Knob>({ 412, 64, 64, 64 }, {
            .graphics = T.delay.parameters.feedback,
            .tooltipName = false,
            .tooltipValue = false,
            .param = Synth.delay.feedback
        });
        
        add<Button>({ 403, 33, 75, 23 }, {
            .graphics = T.delay.parameters.pingpong,
            .behaviour = Button::Behaviour::Toggle,
            .param = Synth.delay.pingpong,
        });
        
        add<Button>({ 343, 146, 64, 46 }, {
            .callback = [&](bool v) {
                delayTimeTempo.select(v);
            },
            .graphics = T.delay.parameters.synced,
            .behaviour = Button::Behaviour::Toggle,
            .param = Synth.delay.synced,
        });
        
        add<Knob>({ 390, 399, 64, 64 }, {
            .graphics = T.knob,
            .tooltipName = false,
            .tooltipValue = false,
            .param = Synth.quality
        });
        
        // ------------------------------------------------
        
        add<Piano>({ 150, 476, 1020, 200 }, {
            .start = 36,
            .notes = 12 * 4 + 1, // 4 octaves + C
            .interface = context.interface<Processing::PianoInterface>(),
            .white = {
                .size = { 30, 200 },
                .graphics = T.piano.whiteKey
            },
            .black = {
                .size = { 23, 120 },
                .graphics = T.piano.blackKey
            },
            .spacing = 5,
        });

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
