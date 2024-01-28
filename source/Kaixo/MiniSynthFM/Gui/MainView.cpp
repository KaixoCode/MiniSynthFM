
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

    class PopupView : public View {
    public:

        // ------------------------------------------------

        PopupView(Context c)
            : View(c)
        {

            // ------------------------------------------------

            add<ImageView>({ .image = T.display.popup.background });

            // ------------------------------------------------

            m_BackButton = &add<Button>({ 80, 100, 96, 20 }, {
                .callback = [this](bool) {
                    if (m_Callback) m_Callback(false);
                    if (--m_Requests == 0) setVisible(false);
                },
                .graphics = T.display.popup.backButton,
            });

            add<Button>({ 178, 100, 96, 20 }, {
                .callback = [this](bool) {
                    if (m_Callback) m_Callback(true);
                    if (--m_Requests == 0) setVisible(false);
                },
                .graphics = T.display.popup.confirmButton
            });

            m_Message = &add<TextView>({ 80, 60, 194, 38 }, {
                .graphics = T.display.popup.message,
                .padding = { 4, 3 },
                .multiline = true,
                .editable = false,
                .lineHeight = 16,
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
    
    class LoadPresetTab : public View {
    public:

        // ------------------------------------------------
        
        class Bank : public View {
        public:

            // ------------------------------------------------
            
            struct Settings {

                LoadPresetTab& self;
                std::string name;
                std::filesystem::path folder{};
                bool isFactory = false;

            } settings;

            // ------------------------------------------------
            
            Theme::Drawable graphics;

            // ------------------------------------------------

            Bank(Context c, Settings s)
                : View(c), settings(std::move(s)) 
            {
                graphics = T.display.loadPreset.bank;
                animation(graphics);
            }

            // ------------------------------------------------
            
            void mouseDown(const juce::MouseEvent& e) override {
                settings.self.select(*this);
            }

            // ------------------------------------------------

            void paint(juce::Graphics& g) override {
                graphics.draw({
                    .graphics = g,
                    .bounds = localDimensions(),
                    .text = settings.name,
                    .state = state()
                });
            }

            // ------------------------------------------------

        };

        // ------------------------------------------------
        
        class Preset : public View {
        public:

            // ------------------------------------------------

            struct Settings {

                LoadPresetTab& self;
                std::filesystem::path path{};
                std::string factoryName{};
                bool isFactory = false;
                bool isInit = false;

            } settings;

            // ------------------------------------------------
            
            Theme::Drawable graphics;
            PresetData presetData;

            // ------------------------------------------------

            Preset(Context c, Settings s)
                : View(c), settings(std::move(s))
            {
                graphics = T.display.loadPreset.preset;

                if (settings.isFactory) {
                    presetData.name = settings.factoryName;
                } else {
                    if (auto json = basic_json::parse(file_to_string(settings.path))) {
                        auto pdata = typeid(PresetData).name();
                        if (json->contains(pdata)) {
                            presetData.deserialize(json.value()[pdata]);
                        }
                    }
                }

                animation(graphics);
            }

            // ------------------------------------------------

            void mouseDown(const juce::MouseEvent& e) override {
                if (settings.isInit) {
                    context.initPreset();
                } else if (settings.isFactory) {
                    // TODO:
                } else {
                    context.loadPreset(settings.path);
                }
            }

            // ------------------------------------------------
            
            void paint(juce::Graphics& g) override {
                graphics.draw({
                    .graphics = g,
                    .bounds = localDimensions(),
                    .text = presetData.name,
                    .state = state()
                });
            }

            // ------------------------------------------------

        };

        // ------------------------------------------------

        struct Settings {

            // ------------------------------------------------

            PopupView& popup;

            // ------------------------------------------------

        } settings;

        // ------------------------------------------------
        
        ScrollView* m_Banks;
        ScrollView* m_Presets;

        // ------------------------------------------------

        LoadPresetTab(Context c, Settings s) 
            : View(c), settings(s) 
        {
            // ------------------------------------------------

            add<ImageView>({ .image = T.display.loadPreset.background });

            // ------------------------------------------------

            m_Banks = &add<ScrollView>({ 0, 0, 110, Height }, {
                .scrollbar = T.scrollbar
            });
            
            m_Presets = &add<ScrollView>({ 110, 0, 220, Height }, {
                .scrollbar = T.scrollbar
            });

            reloadBanks();
        }

        // ------------------------------------------------
        
        void reloadBanks() {
            m_Banks->clear();

            select(m_Banks->add<Bank>({ Width, 20 }, {
                .self = *this,
                .name = "Factory",
                .isFactory = true
            }));

            if (auto optPath = Storage::get<std::string>(PresetPath)) {
                std::filesystem::path path = optPath.value();

                if (!std::filesystem::exists(path)) return;

                m_Banks->add<Bank>({ Width, 20 }, {
                    .self = *this,
                    .name = "User",
                    .folder = path
                });

                for (auto& entry : std::filesystem::directory_iterator(path)) {
                    if (entry.is_directory()) {
                        m_Banks->add<Bank>({ Width, 20 }, {
                            .self = *this,
                            .name = entry.path().stem().string(),
                            .folder = entry
                        });
                    }
                }
            }
        }

        // ------------------------------------------------
        
        void select(Bank& bank) {
            m_Presets->clear();

            for (auto& b : m_Banks->views()) {
                b->selected(false);
            }
            bank.selected(true);

            if (bank.settings.isFactory) {
                m_Presets->add<Preset>({ Width, 20 }, {
                    .self = *this,
                    .factoryName = "Init",
                    .isFactory = true,
                    .isInit = true,
                });
            } else {

                if (!std::filesystem::exists(bank.settings.folder)) return;

                for (auto& entry : std::filesystem::directory_iterator(bank.settings.folder)) {
                    if (entry.is_regular_file()) {
                        m_Presets->add<Preset>({ Width, 20 }, {
                            .self = *this,
                            .path = entry
                        });
                    }
                }
            }
        }

        // ------------------------------------------------

    };

    // ------------------------------------------------
    
    class SettingsTab : public View {
    public:

        // ------------------------------------------------

        struct Settings {

            // ------------------------------------------------

            PopupView& popup;

            // ------------------------------------------------

        } settings;

        // ------------------------------------------------

        TabControl zoom;

        // ------------------------------------------------

        Button* presetPath;

        // ------------------------------------------------

        juce::FileChooser themeChooser{ "Choose a MiniFM theme.", {}, "*.minifmtheme" };
        juce::FileChooser presetPathChooser{ "Choose a MiniFM preset folder." };

        // ------------------------------------------------

        SettingsTab(Context c, Settings s)
            : View(c), settings(s)
        {
            // ------------------------------------------------

            add<ImageView>({ .image = T.display.settings.background });

            // ------------------------------------------------

            //add(new TimerValue{ context }, Dimensions{ 10, 210, 10 + 200, 210 + 100 });

            zoom.addButton(0, add<Button>({ 5 + 0 * 48, 5, 48, 20 }, { .graphics = T.display.settings.zoomButton[0] }));
            zoom.addButton(1, add<Button>({ 5 + 1 * 48, 5, 48, 20 }, { .graphics = T.display.settings.zoomButton[1] }));
            zoom.addButton(2, add<Button>({ 5 + 2 * 48, 5, 48, 20 }, { .graphics = T.display.settings.zoomButton[2] }));
            
            zoom.tab(0).addCallback([&](bool v) { if (v) context.scale(0.5); });
            zoom.tab(1).addCallback([&](bool v) { if (v) context.scale(0.7); });
            zoom.tab(2).addCallback([&](bool v) { if (v) context.scale(1.0); });
            
            auto zoomFactor = context.scale();
            if (zoomFactor == 0.5) zoom.select(0);
            else if (zoomFactor == 0.7) zoom.select(1);
            else if (zoomFactor == 1.0) zoom.select(2);
            else zoom.select(2);

            Button& themePath = add<Button>({ 5, 25, 320, 20 }, {
                .callback = [&](bool) {
                    themeChooser.launchAsync(
                          juce::FileBrowserComponent::openMode
                        | juce::FileBrowserComponent::canSelectFiles,
                    [&](const juce::FileChooser& choose) 
                    {
                        auto file = choose.getResult();
                        auto filepath = file.getFullPathName().toStdString();

                        if (!T.open(filepath)) return; // Try open theme

                        themePath.settings.text = T.name();
                        context.repaint();
                        Storage::set<std::string>(ThemePath, filepath);
                    });
                },
                .graphics = T.display.settings.themePath,
                .text = std::string{ T.name() },
            });

            add<Button>({ 5, 45, 320, 20 }, {
                .callback = [&](bool) {
                    Storage::set<std::string>(ThemePath, Theme::Default);
                    T.openDefault();
                    themePath.settings.text = Theme::Default;
                    context.repaint();
                },
                .graphics = T.display.settings.defaultTheme
            });

            add<Button>({ 5, 85, 320, 20 }, {
                .callback = [&](bool) {
                    T.reopen();
                    themePath.settings.text = T.name();
                    context.repaint();
                },
                .graphics = T.display.settings.reloadTheme
            });

            std::string storedPresetPath = Storage::getOrDefault<std::string>(PresetPath, "No Path Selected");

            presetPath = &add<Button>({ 5, 105, 320, 20 }, {
                .callback = [this](bool) {
                    themeChooser.launchAsync(
                          juce::FileBrowserComponent::openMode
                        | juce::FileBrowserComponent::canSelectDirectories,
                    [this](const juce::FileChooser& choose)
                    {
                        auto file = choose.getResult();
                        if (!file.exists()) return;
                        auto filepath = file.getFullPathName().toStdString();

                        presetPath->settings.text = filepath;
                        presetPath->repaint();
                        Storage::set<std::string>(PresetPath, filepath);
                    });
                },
                .graphics = T.display.settings.presetPath,
                .text = storedPresetPath
            });

            add<Button>({ 5, 145, 320, 20 }, {
                .callback = [&](bool state) {
                    Storage::set<bool>(Setting::TouchMode, state);
                },
                .graphics = T.display.settings.touchMode,
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
        
        struct Settings {

            // ------------------------------------------------

            PopupView& popup;

            // ------------------------------------------------

        } settings;

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

        // ------------------------------------------------

        PresetTab(Context c, Settings s)
            : View(c), settings(s)
        {

            // ------------------------------------------------
            
            add<ImageView>({ .image = T.display.savePreset.background });

            // ------------------------------------------------

            name = &add<TextView>({ 6, 6, 312, 20 }, {
                .graphics = T.ledText,
                .padding = { 4, 3 },
                .multiline = false,
                .editable = true,
                .lineHeight = 14,
                .maxSize = 32,
                .placeholder = "Name"
            });

            author = &add<TextView>({ 6, 28, 312, 20 }, {
                .graphics = T.ledText,
                .padding = { 4, 3 },
                .multiline = false,
                .editable = true,
                .lineHeight = 14,
                .maxSize = 32,
                .placeholder = "Author"
            });

            type = &add<TextView>({ 6, 50, 312, 20 }, {
                .graphics = T.ledText,
                .padding = { 4, 3 },
                .multiline = false,
                .editable = true,
                .lineHeight = 14,
                .maxSize = 32,
                .placeholder = "Type"
            });

            description = &add<TextView>({ 6, 72, 312, 65 }, {
                .graphics = T.ledText,
                .padding = { 4, 3 },
                .multiline = true,
                .editable = true,
                .lineHeight = 16,
                .maxSize = 255,
                .placeholder = "Description"
            });

            add<Button>({ 6, 139, 312, 20 }, {
                .callback = [&](bool) {
                    if (name->empty()) settings.popup.open([](bool) {}, "You cannot leave the preset name blank.", false);
                    else savePreset(false);
                },
                .graphics = T.display.savePreset.saveButton,
            });

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
                auto file = presetPath / (name->content() + ".minifm");

                resultHandler(context.savePreset(file, force));
            } else {
                settings.popup.open([this](bool v) {
                    //context.tabControl(MidTabs).select(4);
                }, "No preset path is specified, please select one.", false);
            }
        }

        // ------------------------------------------------
    
        void resultHandler(SaveResult result) {
            switch (result) {
            case SaveResult::Success: return; // Success
            case SaveResult::AlreadyExists:
                return settings.popup.open([this](bool v) {
                    if (!v) return;
                    savePreset(true);
                    }, "File already exists, do you wish to overwrite?");
            case SaveResult::CannotWrite:
                return settings.popup.open([](bool) {}, "Cannot write to file, check folder permissions.", false);
            case SaveResult::InvalidPath:
                return settings.popup.open([](bool) {}, "Cannot save preset.", false);
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
        
        Processing::InterfaceStorage<float()> timerValue;
        TextView* timer = nullptr;
        TextView* description = nullptr;

        // ------------------------------------------------

        struct Settings {

            // ------------------------------------------------

            PopupView& popup;

            // ------------------------------------------------

        } settings;

        // ------------------------------------------------

        MainTab(Context c, Settings s)
            : View(c), settings(s)
        {
            // ------------------------------------------------

            add<ImageView>({ .image = T.display.main.background });

            // ------------------------------------------------

            wantsIdle(true);

            timerValue = context.interface<Processing::TimerInterface>();

            description = &add<TextView>({ 5, 35, 320, 125 }, {
                .graphics = T.ledText,
                .multiline = true,
                .editable = false,
            });

            timer = &add<TextView>({ 5, 5, 320, 20 }, {
                .graphics = T.ledText,
                .multiline = false,
                .editable = false,
            });
        }

        // ------------------------------------------------
    
        void onIdle() override {
            timer->setText(std::format("{:.4f} %", timerValue()));
            timer->repaint();
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

            auto& popup = add<PopupView>();

            tabs.add(0, add<MainTab>({ .popup = popup }));
            tabs.add(1, add<PresetTab>({ .popup = popup }));
            tabs.add(2, add<LoadPresetTab>({ .popup = popup }));
            tabs.add(3, add<SettingsTab>({ .popup = popup }));

            tabs.addButton(0, add<Button>({ 314, 1, 40, 40 }, {
                .graphics = T.display.main.button
            }));

            tabs.addButton(1, add<Button>({ 314, 42, 40, 40 }, {
                .graphics = T.display.savePreset.button
            }));

            tabs.addButton(2, add<Button>({ 314, 83, 40, 40 }, {
                .graphics = T.display.loadPreset.button
            }));

            tabs.addButton(3, add<Button>({ 314, 124, 40, 40 }, {
                .graphics = T.display.settings.button
            }));

            tabs.select(0);

            // Move popup to end of views, so it draws on top
            removeChildComponent(&popup);
            addChildComponent(&popup);
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
            .graphics = T.filter.parameters.enable,
            .behaviour = Button::Behaviour::Toggle,
            .param = Synth.filter.enable
        });

        add<Knob>({ 353, 64, 64, 64 }, {
            .graphics = T.filter.parameters.cutoff,
            .tooltipName = false,
            .tooltipValue = false,
            .param = Synth.filter.cutoff
        });

        add<Jack>({ 353, 140, 64, 52 }, {
            .graphics = T.filter.jacks.cutoff,
            .type = Jack::Type::Input,
            .patchBay = patchBay,
            .destination = ModDestination::FilterFreq,
            .name = "Cutoff"
        });

        add<Knob>({ 422, 64, 64, 64 }, {
            .graphics = T.filter.parameters.resonance,
            .tooltipName = false,
            .tooltipValue = false,
            .param = Synth.filter.resonance
        });
        
        add<Knob>({ 422, 128, 64, 64 }, {
            .graphics = T.filter.parameters.drive,
            .tooltipName = false,
            .tooltipValue = false,
            .param = Synth.filter.drive
        });

        // ------------------------------------------------

        for (std::size_t i = 0; i < Oscillators; ++i) {

            // ------------------------------------------------

            add<Knob>({ 37 + i * 280, 244, 64, 64 }, {
                .graphics = T.oscillator.parameters.volume,
                .tooltipName = false,
                .tooltipValue = false,
                .param = Synth.oscillator[i].volume
            });
            
            add<Knob>({ 37 + i * 280, 308, 64, 64 }, {
                .graphics = T.oscillator.parameters.waveform,
                .tooltipName = false,
                .tooltipValue = false,
                .param = Synth.oscillator[i].waveform
            });

            add<Knob>({ 103 + i * 280, 308, 64, 64 }, {
                .graphics = T.oscillator.parameters.fm,
                .tooltipName = false,
                .tooltipValue = false,
                .param = Synth.oscillator[i].fm
            });
            
            add<Knob>({ 103 + i * 280, 244, 64, 64 }, {
                .graphics = T.oscillator.parameters.tune,
                .tooltipName = false,
                .tooltipValue = false,
                .param = Synth.oscillator[i].tune
            });
            
            add<Jack>({ 169 + i * 280, 256, 64, 52 }, {
                .graphics = T.oscillator.jacks.fm,
                .type = Jack::Type::Input,
                .patchBay = patchBay,
                .destination = i == 0 ? ModDestination::Op1FM
                             : i == 1 ? ModDestination::Op2FM
                             : ModDestination::Op3FM,
                .name = "FM"
            });
            
            add<Jack>({ 169 + i * 280, 320, 64, 52 }, {
                .graphics = T.oscillator.jacks.amount,
                .type = Jack::Type::Input,
                .patchBay = patchBay,
                .destination = i == 0 ? ModDestination::Op1Amount
                             : i == 1 ? ModDestination::Op2Amount
                             :          ModDestination::Op3Amount,
                .name = "Amount"
            });

            add<Jack>({ 235 + i * 280, 256, 64, 52 }, {
                .graphics = T.oscillator.jacks.sync,
                .type = Jack::Type::Input,
                .patchBay = patchBay,
                .destination = i == 0 ? ModDestination::Op1Sync
                             : i == 1 ? ModDestination::Op2Sync
                             : ModDestination::Op3Sync,
                .name = "Sync"
            });
            
            add<Jack>({ 235 + i * 280, 320, 64, 52 }, {
                .graphics = T.oscillator.jacks.output,
                .type = Jack::Type::Output,
                .patchBay = patchBay,
                .source = i == 0 ? ModSource::Op1
                        : i == 1 ? ModSource::Op2
                        :          ModSource::Op3
            });

            // ------------------------------------------------
            
            add<Button>({ 215 + i * 280, 213, 80, 23 }, {
                .graphics = T.oscillator.parameters.output,
                .behaviour = Button::Behaviour::Toggle,
                .param = Synth.oscillator[i].output
            });

            // ------------------------------------------------
            
            add<Knob>({ 92 + i * 280, 213, 80, 23 }, {
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

            add<Knob>({ 195, 64, 64, 64 }, {
                .graphics = T.lfo.parameters.depth,
                .tooltipName = false,
                .tooltipValue = false,
                .param = Synth.lfo[i].depth
            });
            
            lfoFrequencyTempo[i].add(0, add<Knob>({264, 64, 64, 64}, {
                .graphics = T.lfo.parameters.frequency,
                .tooltipName = false,
                .tooltipValue = false,
                .param = Synth.lfo[i].frequency
            }));

            lfoFrequencyTempo[i].add(1, add<Knob>({264, 64, 64, 64}, {
                .graphics = T.lfo.parameters.tempo,
                .tooltipName = false,
                .tooltipValue = false,
                .param = Synth.lfo[i].tempo
            }));

            lfoFrequencyTempo[i].select(1);

            add<Knob>({ 195, 128, 64, 64 }, {
                .graphics = T.lfo.parameters.waveform,
                .tooltipName = false,
                .tooltipValue = false,
                .param = Synth.lfo[i].waveform
            });

            // ------------------------------------------------

            add<Button>({ 239, 33, 75, 23 }, {
                .callback = [&, i](bool val) {
                    lfoFrequencyTempo[i].select(val);
                },
                .graphics = T.lfo.parameters.synced,
                .behaviour = Button::Behaviour::Toggle,
                .param = Synth.lfo[i].synced
            });

            // ------------------------------------------------

            add<Jack>({ 264, 140, 64, 52 }, {
                .graphics = T.lfo.jacks.output,
                .type = Jack::Type::Output,
                .patchBay = patchBay,
                .source = ModSource::LFO
            });

            // ------------------------------------------------
            
            add<Led>({ 315, 38, 13, 13 }, {
                .graphics = T.lfo.led,
                .value = context.interface<Processing::LfoInterface>({ .index = i })
            });

            // ------------------------------------------------

        }

        // ------------------------------------------------
        
        for (std::size_t i = 0; i < ADSREnvelopes; ++i) {
            add<Knob>({ 880, 71 + i * 180, 64, 64 }, {
                .graphics = T.envelope.parameters.level,
                .tooltipName = false,
                .tooltipValue = false,
                .param = Synth.envelope[i].level
            });
                        
            add<Knob>({ 957, 52 + i * 180, 60, 140 }, {
                .graphics = T.envelope.parameters.attack,
                .tooltipName = false,
                .tooltipValue = false,
                .param = Synth.envelope[i].attack
            });

            add<Knob>({ 1017, 52 + i * 180, 60, 140 }, {
                .graphics = T.envelope.parameters.decay,
                .tooltipName = false,
                .tooltipValue = false,
                .param = Synth.envelope[i].decay
            });

            add<Knob>({ 1077, 52 + i * 180, 60, 140 }, {
                .graphics = T.envelope.parameters.sustain,
                .tooltipName = false,
                .tooltipValue = false,
                .param = Synth.envelope[i].sustain
            });

            add<Knob>({ 1137, 52 + i * 180, 60, 140 }, {
                .graphics = T.envelope.parameters.release,
                .tooltipName = false,
                .tooltipValue = false,
                .param = Synth.envelope[i].release
            });
            
            // ------------------------------------------------
            
            add<Jack>({ 880, 140 + i * 180, 64, 52 }, {
                .graphics = T.envelope.jacks.output,
                .type = Jack::Type::Output,
                .patchBay = patchBay,
                .source = i == 0 ? ModSource::Envelope1 : ModSource::Envelope2
            });
            
            // ------------------------------------------------
            
            add<Button>({ 942, 33 + i * 180, 80, 23 }, {
                .graphics = T.envelope.parameters.loop,
                .behaviour = Button::Behaviour::Toggle,
                .param = Synth.envelope[i].loop
            });
            
            // ------------------------------------------------

            add<Led>({ 1182, 38 + i * 180, 13, 13 }, {
                .graphics = T.envelope.led,
                .value = context.interface<Processing::EnvelopeInterface>({ .index = i })
            });

            // ------------------------------------------------

        }

        // ------------------------------------------------

        add<Knob>({ 941, 396, 64, 64 }, {
            .graphics = T.gain.parameters.level,
            .tooltipName = false,
            .tooltipValue = false,
            .param = Synth.gain.level
        });
                        
        add<Knob>({ 1006, 396, 64, 64 }, {
            .graphics = T.gain.parameters.attack,
            .tooltipName = false,
            .tooltipValue = false,
            .param = Synth.gain.attack
        });

        add<Knob>({ 1071, 396, 64, 64 }, {
            .graphics = T.gain.parameters.decay,
            .tooltipName = false,
            .tooltipValue = false,
            .param = Synth.gain.decay
        });

        // ------------------------------------------------
            
        add<Jack>({ 1136, 408, 64, 52 }, {
            .graphics = T.gain.jacks.output,
            .type = Jack::Type::Output,
            .patchBay = patchBay,
            .source = ModSource::Envelope3
        });
            
        // ------------------------------------------------
            
        add<Button>({ 876, 415, 64, 45 }, {
            .graphics = T.gain.parameters.gate,
            .behaviour = Button::Behaviour::Toggle,
            .param = Synth.gain.gate
        });
            
        // ------------------------------------------------

        add<Led>({ 1182, 398, 13, 13 }, {
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
        
        add<Knob>({ 460, 396, 64, 64 }, {
            .graphics = T.knob,
            .tooltipName = false,
            .tooltipValue = false,
            .param = Synth.delay.mix
        });

        delayTimeTempo.add(0, add<Knob>({ 524, 396, 64, 64 }, {
            .graphics = T.knob,
            .tooltipName = false,
            .tooltipValue = false,
            .param = Synth.delay.delay
        }));
        
        delayTimeTempo.add(1, add<Knob>({ 524, 396, 64, 64 }, {
            .graphics = T.knob,
            .tooltipName = false,
            .tooltipValue = false,
            .param = Synth.delay.tempo
        }));

        delayTimeTempo.select(1);

        add<Knob>({ 588, 396, 64, 64 }, {
            .graphics = T.knob,
            .tooltipName = false,
            .tooltipValue = false,
            .param = Synth.delay.feedback
        });
        
        add<Button>({ 652, 396, 64, 64 }, {
            .graphics = T.toggle,
            .behaviour = Button::Behaviour::Toggle,
            .param = Synth.delay.pingpong,
        });
        
        add<Button>({ 726, 396, 64, 64 }, {
            .callback = [&](bool v) {
                delayTimeTempo.select(v);
            },
            .graphics = T.toggle,
            .behaviour = Button::Behaviour::Toggle,
            .param = Synth.delay.synced,
        });
        
        add<Knob>({ 790, 396, 64, 64 }, {
            .graphics = T.knob,
            .tooltipName = false,
            .tooltipValue = false,
            .param = Synth.quality
        });
        
        // ------------------------------------------------
        
        add<Piano>({ 180, 493, 1020, 200 }, {
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
