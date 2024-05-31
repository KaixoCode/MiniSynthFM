
// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Gui/Tabs/SettingsTab.hpp"

// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Processing/Interfaces.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    SettingsTab::SettingsTab(Context c, Settings s)
        : View(c), settings(s)
    {
        // ------------------------------------------------
        
        wantsIdle(true);

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
        
        scrollView.add<Button>({ Width, 20 }, {
            .graphics = T.display.settings.parameters.noisyFilter,
            .behaviour = Button::Behaviour::Toggle,
            .param = Synth.noisyFilter
        });

        // ------------------------------------------------

        scrollView.add<Knob>({ Width, 20 }, {
            .graphics = T.display.settings.parameters.delayAlgorithm,
            .tooltipName = false,
            .tooltipValue = false,
            .param = Synth.delayAlgorithm,
        });
        
        // ------------------------------------------------

        scrollView.add<Knob>({ Width, 20 }, {
            .graphics = T.display.settings.parameters.pitchBendRange,
            .tooltipName = false,
            .tooltipValue = false,
            .param = Synth.pitchBendRange,
        });
        
        // ------------------------------------------------

        scrollView.add<Knob>({ Width, 20 }, {
            .graphics = T.display.settings.parameters.phaseMode,
            .tooltipName = false,
            .tooltipValue = false,
            .param = Synth.phaseMode,
        });

        // ------------------------------------------------

        if constexpr (versionType == VersionType::Demo) {
            scrollView.add<Knob>({ Width, 20 }, {
                .onchange = [&](ParamValue val) {
                    context.beginEdit(Synth.quality);
                    context.performEdit(Synth.quality, 0);
                    context.endEdit(Synth.quality);
                    settings.popup.open([](bool) {}, "Cannot change quality in demo mode.", false);
                },
                .graphics = T.display.settings.parameters.quality,
                .tooltipName = false,
                .tooltipValue = false,
                .param = Synth.quality,
            });
        } else {
            scrollView.add<Knob>({ Width, 20 }, {
                .graphics = T.display.settings.parameters.quality,
                .tooltipName = false,
                .tooltipValue = false,
                .param = Synth.quality,
            });
        }
        
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

        Button& themePath = scrollView.add<Button>({ Width, 20 }, {
            .callback = [this, &themePath](bool) {
                themeChooser.launchAsync(
                      juce::FileBrowserComponent::openMode
                    | juce::FileBrowserComponent::canSelectFiles,
                [this, &themePath](const juce::FileChooser& choose)
                {
                    auto file = choose.getResult();
                    auto filepath = file.getFullPathName().toStdString();

                    if (!T.open(filepath)) return; // Try open theme

                    themePath.settings.text = T.name();
                    context.repaint();
                    Storage::set<std::string>(Setting::LoadedTheme, filepath);
                });
            },
            .graphics = T.display.settings.themePath,
            .text = std::string{ T.name() },
        });

        scrollView.add<Button>({ Width, 20 }, {
            .callback = [this, &themePath](bool) {
                Storage::set<std::string>(Setting::LoadedTheme, Theme::Default);
                T.openDefault();
                themePath.settings.text = Theme::Default;
                context.repaint();
            },
            .graphics = T.display.settings.defaultTheme
        });

        scrollView.add<Button>({ Width, 20 }, {
            .callback = [this, &themePath](bool) {
                T.reopen();
                themePath.settings.text = T.name();
                context.repaint();
            },
            .graphics = T.display.settings.reloadTheme
        });

        std::string storedPresetPath = Storage::getOrDefault<std::string>(PresetPath, "No Path Selected");

        Button& presetPath = scrollView.add<Button>({ Width, 20 }, {
            .callback = [this, &presetPath](bool) {
                themeChooser.launchAsync(
                        juce::FileBrowserComponent::openMode
                    | juce::FileBrowserComponent::canSelectDirectories,
                [this, &presetPath](const juce::FileChooser& choose)
                {
                    auto file = choose.getResult();
                    if (!file.exists()) return;
                    auto filepath = file.getFullPathName().toStdString();

                    presetPath.settings.text = filepath;
                    presetPath.repaint();
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
        }).value(Storage::getOrDefault<bool>(ShowPiano, true));
        
        scrollView.add<Button>({ Width, 20 }, {
            .callback = [&](bool state) {
                Storage::set<bool>(CablePhysics, state);
                context.repaint();
            },
            .graphics = T.display.settings.cablePhysics,
            .behaviour = Button::Behaviour::Toggle,
        }).value(Storage::getOrDefault<bool>(CablePhysics, true));


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
        
        scrollView.add<ImageView>({ Width, 20 }, {
            .image = T.display.settings.infoTitle,
            .text = "Info",
        });

        auto& cpuUsage = scrollView.add<Button>({ Width, 20 }, {
            .graphics = T.display.settings.cpuUsage,
        });
        
        watch<float>([interface = context.interface<Processing::GeneralInfoInterface>()] {
            return interface->percent();
        }, [&cpuUsage](float percent) {
            cpuUsage.settings.text = std::format("{:.2f} %", percent);
            cpuUsage.repaint();
        });

        auto& activeVoices = scrollView.add<Button>({ Width, 20 }, {
            .graphics = T.display.settings.activeVoices,
        });

        watch<std::size_t>([interface = context.interface<Processing::GeneralInfoInterface>()] {
            return interface->activeVoices();
        }, [&activeVoices](std::size_t voices) {
            activeVoices.settings.text = std::format("{:d}/8", voices);
            activeVoices.repaint();
        });

        std::string optimizations = "No SIMD registers available";
        switch (simd_path::path) {
        case simd_path::P0: optimizations = "No SIMD registers available"; break;
        case simd_path::P1: optimizations = "SSE/SSE2"; break;
        case simd_path::P2: optimizations = "SSE/SSE2/3/4.1 FMA"; break;
        case simd_path::P3: optimizations = "SSE/SSE2/3/4.1 FMA AVX/AVX2"; break;
        }

        scrollView.add<Button>({ Width, 20 }, {
            .graphics = T.display.settings.simdOptimizations,
            .text = optimizations,
        });

        auto& sampleRate = scrollView.add<Button>({ Width, 20 }, {
            .graphics = T.display.settings.sampleRate,
        });

        watch<float>([interface = context.interface<Processing::GeneralInfoInterface>()] {
            return interface->sampleRate();
        }, [&sampleRate](float srate) {
            sampleRate.settings.text = Formatters::Frequency.format(srate);
            sampleRate.repaint();
        });

        scrollView.add<Button>({ Width, 20 }, {
            .graphics = T.display.settings.version,
            .text = SYNTH_FullVersion,
        });
        
        // ------------------------------------------------

        add<ImageView>({ .image = T.display.settings.foreground, .enableMouse = false });

        // ------------------------------------------------

    }

    // ------------------------------------------------

}

// ------------------------------------------------
