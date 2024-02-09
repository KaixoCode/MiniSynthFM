
// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Gui/Tabs/SettingsTab.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    SettingsTab::SettingsTab(Context c, Settings s)
        : View(c), settings(s)
    {
        // ------------------------------------------------

        add<ImageView>({ .image = T.display.settings.background });

        // ------------------------------------------------
        
        auto& scrollView = add<ScrollView>({ 6, 6, 300, 153 }, {
            .scrollbar = T.display.loadPreset.scrollbar,
            .margin = { 0, 0, 0, 0 },
            .gap = 2,
            .barThickness = 5,
            .barPadding = { 2, 0, 0, 0 },
            .keepBarSpace = false,
            .alignChildren = Theme::Align::Left
        });

        // ------------------------------------------------

        scrollView.add<ImageView>({ Width, 20 }, {
            .image = T.display.settings.presetTitle,
            .text = "Preset Settings",
        });
        
        // ------------------------------------------------

        scrollView.add<Knob>({ Width, 20 }, {
            .graphics = T.display.settings.parameters.phaseMode,
            .tooltipName = false,
            .tooltipValue = false,
            .param = Synth.phaseMode,
        });

        // ------------------------------------------------

        scrollView.add<Knob>({ Width, 20 }, {
            .graphics = T.display.settings.parameters.quality,
            .tooltipName = false,
            .tooltipValue = false,
            .param = Synth.quality,
        });
        
        scrollView.add<Knob>({ Width, 20 }, {
            .graphics = T.display.settings.parameters.exportQuality,
            .tooltipName = false,
            .tooltipValue = false,
            .param = Synth.exportQuality,
        });

        // ------------------------------------------------

        scrollView.add<ImageView>({ Width, 20 }, {
            .image = T.display.settings.globalTitle,
            .text = "Global Settings",
        });

        // ------------------------------------------------

        themePath = &scrollView.add<Button>({ Width, 20 }, {
            .callback = [this](bool) {
                themeChooser.launchAsync(
                        juce::FileBrowserComponent::openMode
                    | juce::FileBrowserComponent::canSelectFiles,
                [this](const juce::FileChooser& choose)
                {
                    auto file = choose.getResult();
                    auto filepath = file.getFullPathName().toStdString();

                    if (!T.open(filepath)) return; // Try open theme

                    themePath->settings.text = T.name();
                    context.repaint();
                    Storage::set<std::string>(Setting::LoadedTheme, filepath);
                });
            },
            .graphics = T.display.settings.themePath,
            .text = std::string{ T.name() },
        });

        scrollView.add<Button>({ Width, 20 }, {
            .callback = [this](bool) {
                Storage::set<std::string>(Setting::LoadedTheme, Theme::Default);
                T.openDefault();
                themePath->settings.text = Theme::Default;
                context.repaint();
            },
            .graphics = T.display.settings.defaultTheme
        });

        scrollView.add<Button>({ Width, 20 }, {
            .callback = [this](bool) {
                T.reopen();
                themePath->settings.text = T.name();
                context.repaint();
            },
            .graphics = T.display.settings.reloadTheme
        });

        std::string storedPresetPath = Storage::getOrDefault<std::string>(PresetPath, "No Path Selected");

        presetPath = &scrollView.add<Button>({ Width, 20 }, {
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

        scrollView.add<Button>({ Width, 20 }, {
            .callback = [&](bool state) {
                Storage::set<bool>(Setting::TouchMode, state);
            },
            .graphics = T.display.settings.touchMode,
            .behaviour = Button::Behaviour::Toggle,
        }).value(Storage::flag(Setting::TouchMode));

        scrollView.add<Button>({ Width, 20 }, {
            .callback = [&](bool state) {
                Storage::set<bool>(ShowPiano, state);
                if (state) {
                    context.window().setSize(SYNTH_InitialSize);
                } else {
                    context.window().setSize(1205, 476);
                }
                context.repaint();
            },
            .graphics = T.display.settings.showPiano,
            .behaviour = Button::Behaviour::Toggle,
        }).value(Storage::flag(ShowPiano));

        scrollView.updateDimensions();

        // ------------------------------------------------

        constexpr float zoomLevels[5]{ 0.5, 0.7, 1.0, 1.5, 2.0 };

        auto& zoom = scrollView.add<Knob>({ Width, 20 }, {
            .onchange = [&, zoomLevels](ParamValue v) {
                auto value = zoomLevels[normalToIndex(v, 5)];
                auto current = context.scale();
                if (value != current) {
                    context.scale(value);
                    Storage::set<float>(WindowScale, value);
                }
            },
            .graphics = T.display.settings.zoomButton,
            .steps = 5,
            .resetValue = 2. / 4.,
        });

        if (auto zoomFactor = Storage::get<float>(WindowScale)) {
            context.scale(zoomFactor.value());
            for (auto [index, level] : std::views::enumerate(zoomLevels)) {
                if (zoomFactor == level) {
                    zoom.value(index / 4.);
                    break;
                }
            }
        } else {
            context.scale(1);
            zoom.value(2. / 4.);
        }

        // ------------------------------------------------

        add<ImageView>({ .image = T.display.settings.foreground, .enableMouse = false });

        // ------------------------------------------------

    }

    // ------------------------------------------------

}

// ------------------------------------------------
