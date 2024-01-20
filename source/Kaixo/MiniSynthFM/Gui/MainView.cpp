
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
#include "Kaixo/MiniSynthFM/Processing/Interfaces.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------
    
    class SettingsTab : public View {
    public:

        // ------------------------------------------------

        TabControl zoom;

        // ------------------------------------------------

        juce::FileChooser themeChooser{ "Choose a Yoyijo theme.", {}, "*.yoyijotheme" };
        juce::FileChooser presetPathChooser{ "Choose a Yoyijo preset folder." };

        // ------------------------------------------------

        SettingsTab(Context c)
            : View(c)
        {
            //add(new TimerValue{ context }, Dimensions{ 10, 210, 10 + 200, 210 + 100 });

            zoom.addButton(0, add<Button>({ 5 + 0 * 48, 5, 48, 20 }, { .graphics = T.blackKey }));
            zoom.addButton(1, add<Button>({ 5 + 1 * 48, 5, 48, 20 }, { .graphics = T.blackKey }));
            zoom.addButton(2, add<Button>({ 5 + 2 * 48, 5, 48, 20 }, { .graphics = T.blackKey }));
            
            zoom.tab(0).addCallback([&](bool v) { if (v) context.scale(0.5); });
            zoom.tab(1).addCallback([&](bool v) { if (v) context.scale(0.7); });
            zoom.tab(2).addCallback([&](bool v) { if (v) context.scale(1.0); });
            
            auto zoomFactor = context.scale();
            if (zoomFactor == 0.5) zoom.select(0);
            else if (zoomFactor == 0.7) zoom.select(1);
            else if (zoomFactor == 1.0) zoom.select(2);
            else zoom.select(2);

            auto& text = add<TextView>({ 5, 25, 320, 20 }, {
                .graphics = T.ledText,
                .padding = { 8, 8 },
                .multiline = false,
                .editable = false,
                .text = T.ledText.font.fitWithinWidth(T.name(), 360)
            });

            add<Button>({ 5, 45, 320, 20 }, {
                .callback = [&](bool) {
                    themeChooser.launchAsync(
                          juce::FileBrowserComponent::openMode
                        | juce::FileBrowserComponent::canSelectFiles,
                    [&](const juce::FileChooser& choose) {
                        auto file = choose.getResult();
                        auto filepath = file.getFullPathName().toStdString();

                        if (!T.open(filepath)) return; // Try open theme

                        text.setText(T.ledText.font.fitWithinWidth(T.name(), 360));
                        context.repaint();
                        Storage::set<std::string>(ThemePath, filepath);
                    });
                }
            });

            add<Button>({ 5, 45, 320, 20 }, {
                .callback = [&](bool) {
                    Storage::set<std::string>(ThemePath, Theme::Default);
                    T.openDefault();
                    text.setText(Theme::Default);
                    context.repaint();
                },
                .graphics = T.whiteKey
            });

            add<Button>({ 5, 85, 320, 20 }, {
                .callback = [&](bool) {
                    T.reopen();
                    text.setText(T.ledText.font.fitWithinWidth(T.name(), 360));
                    context.repaint();
                },
                .graphics = T.whiteKey
            });

            std::string storedPresetPath = Storage::getOrDefault<std::string>(PresetPath, "No Path Selected");

            auto& presetPath = add<TextView>({ 5, 105, 320, 32 }, {
                .graphics = T.ledText,
                .padding = { 8, 8 },
                .multiline = false,
                .editable = false,
                .text = T.ledText.font.fitWithinWidth(storedPresetPath, 360)
            });

            add<Button>({ 5, 105, 320, 20 }, {
                .callback = [&](bool) {
                    themeChooser.launchAsync(
                          juce::FileBrowserComponent::openMode
                        | juce::FileBrowserComponent::canSelectDirectories,
                    [&](const juce::FileChooser& choose) {
                        auto file = choose.getResult();
                        if (!file.exists()) return;
                        auto filepath = file.getFullPathName().toStdString();

                        presetPath.setText(T.ledText.font.fitWithinWidth(filepath, 360));
                        presetPath.repaint();
                        Storage::set<std::string>(PresetPath, filepath);
                    });
                }
            });

            add<Button>({ 5, 145, 320, 20 }, {
                .callback = [&](bool state) {
                    Storage::set<bool>(Setting::TouchMode, state);
                },
                .graphics = T.whiteKey,
                .behaviour = Button::Behaviour::Toggle,
            }).selected(Storage::flag(Setting::TouchMode));
        }

        // ------------------------------------------------

        // ------------------------------------------------

    };

    // ------------------------------------------------
    
    class PresetTab : public View, public PresetListener {
    public:

        // ------------------------------------------------

        class PopupView : public View {
        public:

            // ------------------------------------------------

            PopupView(Context c)
                : View(c)
            {
                
                // ------------------------------------------------

                m_BackButton = &add<Button>({ 94, 130, 146, 32 }, {
                    .callback = [this](bool) {
                        if (m_Callback) m_Callback(false);
                        if (--m_Requests == 0) setVisible(false);
                    },
                    .graphics = T.blackKey,
                });

                add<Button>({ 244, 130, 146, 32 }, {
                    .callback = [this](bool) {
                        if (m_Callback) m_Callback(true);
                        if (--m_Requests == 0) setVisible(false);
                    },
                    .graphics = T.whiteKey
                });

                m_Message = &add<TextView>({ 94, 75, 296, 56 }, {
                    .graphics = T.ledText,
                    .padding = { 8, 8 },
                    .multiline = true,
                    .editable = false,
                    .lineHeight = 22,
                });

                // ------------------------------------------------

                setVisible(false);

                // ------------------------------------------------

            }

            // ------------------------------------------------

            void open(auto c, std::string_view text, bool withBack = true) {
                m_Callback = c;
                m_Message->setText(text);
                m_Requests++;
                m_BackButton->setVisible(withBack);
                setVisible(true);
            }

            // ------------------------------------------------
        private:
            std::function<void(bool)> m_Callback;
            TextView* m_Message;
            std::size_t m_Requests = 0;
            Button* m_BackButton;

            // ------------------------------------------------

        };

        // ------------------------------------------------

        void presetSaved() override {
            reloadPresetInformation();
        }

        void presetLoaded() override {
            reloadPresetInformation();
        }

        void reloadPresetInformation() {
            name->setText(context.data<PresetData>().name);
            author->setText(context.data<PresetData>().author);
            type->setText(context.data<PresetData>().type);
            description->setText(context.data<PresetData>().description);
            context.defaultDescription(context.data<PresetData>().description);
            context.clearDescription();
            repaint();
        }

        // ------------------------------------------------
        
        TextView* name = nullptr;
        TextView* author = nullptr;
        TextView* type = nullptr;
        TextView* description = nullptr;

        PopupView* popup = nullptr;

        // ------------------------------------------------

        PresetTab(Context c)
            : View(c)
        {

            // ------------------------------------------------

            name = &add<TextView>({ 5, 5, 320, 20 }, {
                .graphics = T.ledText,
                .multiline = false,
                .editable = true,
                .maxSize = 32,
                .placeholder = "Name"
            });

            author = &add<TextView>({ 5, 25, 320, 20 }, {
                .graphics = T.ledText,
                .multiline = false,
                .editable = true,
                .maxSize = 32,
                .placeholder = "Author"
            });

            type = &add<TextView>({ 5, 45, 320, 20 }, {
                .graphics = T.ledText,
                .multiline = false,
                .editable = true,
                .maxSize = 32,
                .placeholder = "Type"
            });

            description = &add<TextView>({ 5, 65, 320, 75 }, {
                .graphics = T.ledText,
                .multiline = true,
                .editable = true,
                .maxSize = 255,
                .placeholder = "Description"
            });

            add<Button>({ 5, 140, 100, 20 }, {
                .callback = [&](bool) {
                    if (name->empty()) popup->open([](bool) {}, "You cannot leave the preset name blank.", false);
                    else savePreset(false);
                },
                .graphics = T.whiteKey,
            });

            popup = &add<PopupView>();

            // ------------------------------------------------

            reloadPresetInformation();

            // ------------------------------------------------

        }

        // ------------------------------------------------
    
        void savePreset(bool force) {
            context.data<PresetData>().name = name->content();
            context.data<PresetData>().author = author->content();
            context.data<PresetData>().type = type->content();
            context.data<PresetData>().description = description->content();

            context.defaultDescription(description->content());

            std::string path = Storage::getOrDefault<std::string>(PresetPath, "No Preset Path");
            std::filesystem::path presetPath = path;
            if (std::filesystem::exists(presetPath)) {
                auto file = presetPath / (name->content() + ".yoyijo");

                resultHandler(context.savePreset(file, force));
            } else {
                popup->open([this](bool v) {
                    //context.tabControl(MidTabs).select(4);
                }, "No preset path is specified, please select one.", false);
            }
        }

        // ------------------------------------------------
    
        void resultHandler(SaveResult result) {
            switch (result) {
            case SaveResult::Success: return; // Success
            case SaveResult::AlreadyExists:
                return popup->open([this](bool v) {
                    if (!v) return;
                    savePreset(true);
                    }, "File already exists, do you wish to overwrite?");
            case SaveResult::CannotWrite:
                return popup->open([](bool) {}, "Cannot write to file, check folder permissions.", false);
            case SaveResult::InvalidPath:
                return popup->open([](bool) {}, "Cannot save preset.", false);
            }
        }

        // ------------------------------------------------
    
    };

    // ------------------------------------------------
    
    class MainTab : public View, public DescriptionListener {
    public:

        // ------------------------------------------------
        
        void updateDescription(std::string_view descr) {
            description->setText(descr);
            description->repaint();
        }

        // ------------------------------------------------
        
        TextView* description = nullptr;

        // ------------------------------------------------

        MainTab(Context c)
            : View(c) 
        {
            description = &add<TextView>({ 5, 35, 320, 125 }, {
                .graphics = T.ledText,
                .multiline = true,
                .editable = false,
            });
        }

        // ------------------------------------------------
    
    };

    // ------------------------------------------------
    
    class LedScreen : public View {
    public:

        // ------------------------------------------------
        
        TabControl tabs{};

        // ------------------------------------------------

        LedScreen(Context c)
            : View(c)
        {
            tabs.add(0, add<MainTab>({ 0, 0, 330, 165 }));

            tabs.addButton(0, add<Button>({ 330, 0, 25, 25 }, {
                .graphics = T.blackKey
            }));
            
            tabs.add(1, add<PresetTab>({ 0, 0, 330, 165 }));

            tabs.addButton(1, add<Button>({ 330, 25, 25, 25 }, {
                .graphics = T.blackKey
            }));

            tabs.add(2, add<SettingsTab>({ 0, 0, 330, 165 }));

            tabs.addButton(2, add<Button>({ 330, 50, 25, 25 }, {
                .graphics = T.blackKey
            }));

            tabs.select(0);
        }

        // ------------------------------------------------

    };

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
        
        context.defaultDescription("MiniFM is a simple FM synthesizer.");

        // ------------------------------------------------

        add<ImageView>({ T.background });

        // ------------------------------------------------

        add<LedScreen>({ 506, 30, 355, 165 });

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
                .graphics = T.knobBi,
                .tooltipName = false,
                .tooltipValue = false,
                .param = Synth.oscillator[i].volume
            });
            
            add<Knob>({ 37 + i * 280, 308, 64, 64 }, {
                .graphics = T.waveformKnob1,
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
                .graphics = T.knobBi,
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
            
            lfoFrequencyTempo[i].add(0, add<Knob>({264, 64, 64, 64}, {
                .graphics = T.knob,
                .tooltipName = false,
                .tooltipValue = false,
                .param = Synth.lfo[i].frequency
            }));

            lfoFrequencyTempo[i].add(1, add<Knob>({264, 64, 64, 64}, {
                .graphics = T.knob,
                .tooltipName = false,
                .tooltipValue = false,
                .param = Synth.lfo[i].tempo
            }));

            lfoFrequencyTempo[i].select(0);

            add<Knob>({ 195, 128, 64, 64 }, {
                .graphics = T.knob,
                .tooltipName = false,
                .tooltipValue = false,
                .param = Synth.lfo[i].waveform
            });

            // ------------------------------------------------

            add<Button>({ 239, 33, 75, 23 }, {
                .callback = [&, i](bool val) {
                    lfoFrequencyTempo[i].select(val);
                },
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
        
        for (std::size_t i = 0; i < ADSREnvelopes; ++i) {
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

        add<Knob>({ 941, 396, 64, 64 }, {
            .graphics = T.knob,
            .tooltipName = false,
            .tooltipValue = false,
            .param = Synth.gain.level
        });
                        
        add<Knob>({ 1006, 396, 64, 64 }, {
            .graphics = T.knob,
            .tooltipName = false,
            .tooltipValue = false,
            .param = Synth.gain.attack
        });

        add<Knob>({ 1071, 396, 64, 64 }, {
            .graphics = T.knob,
            .tooltipName = false,
            .tooltipValue = false,
            .param = Synth.gain.decay
        });

        // ------------------------------------------------
            
        add<Jack>({ 1136, 408, 64, 52 }, {
            .graphics = T.outputJack,
            .type = Jack::Type::Output,
            .patchBay = patchBay,
            .source = ModSource::Envelope3
        });
            
        // ------------------------------------------------
            
        add<Button>({ 876, 415, 64, 45 }, {
            .graphics = T.toggle,
            .behaviour = Button::Behaviour::Toggle,
            .param = Synth.gain.gate
        });
            
        // ------------------------------------------------

        add<Led>({ 1182, 398, 13, 13 }, {
            .graphics = T.led,
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
        
        add<Piano>({ 180, 493, 1020, 200 }, {
            .start = 36,
            .notes = 12 * 4 + 1, // 4 octaves + C
            .interface = context.interface<Processing::PianoInterface>(),
            .white = {
                .size = { 30, 200 },
                .graphics = T.whiteKey
            },
            .black = {
                .size = { 23, 120 },
                .graphics = T.blackKey
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
