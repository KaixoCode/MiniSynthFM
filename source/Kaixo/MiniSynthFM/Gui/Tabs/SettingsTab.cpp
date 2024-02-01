
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

        zoom.addButton(0, add<Button>({ 6 + 0 * 50, 116, 48, 20 }, { .graphics = T.display.settings.zoomButton[0] }));
        zoom.addButton(1, add<Button>({ 6 + 1 * 50, 116, 48, 20 }, { .graphics = T.display.settings.zoomButton[1] }));
        zoom.addButton(2, add<Button>({ 6 + 2 * 50, 116, 48, 20 }, { .graphics = T.display.settings.zoomButton[2] }));

        zoom.tab(0).addCallback([&](bool v) { if (v) context.scale(0.5); });
        zoom.tab(1).addCallback([&](bool v) { if (v) context.scale(0.7); });
        zoom.tab(2).addCallback([&](bool v) { if (v) context.scale(1.0); });

        auto zoomFactor = context.scale();
        if (zoomFactor == 0.5) zoom.select(0);
        else if (zoomFactor == 0.7) zoom.select(1);
        else if (zoomFactor == 1.0) zoom.select(2);
        else zoom.select(2);

        themePath = &add<Button>({ 6, 6, 300, 20 }, {
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

        add<Button>({ 6, 28, 300, 20 }, {
            .callback = [this](bool) {
                Storage::set<std::string>(Setting::LoadedTheme, Theme::Default);
                T.openDefault();
                themePath->settings.text = Theme::Default;
                context.repaint();
            },
            .graphics = T.display.settings.defaultTheme
        });

        add<Button>({ 6, 50, 300, 20 }, {
            .callback = [this](bool) {
                T.reopen();
                themePath->settings.text = T.name();
                context.repaint();
            },
            .graphics = T.display.settings.reloadTheme
        });

        std::string storedPresetPath = Storage::getOrDefault<std::string>(PresetPath, "No Path Selected");

        presetPath = &add<Button>({ 6, 72, 300, 20 }, {
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

        add<Button>({ 6, 94, 300, 20 }, {
            .callback = [&](bool state) {
                Storage::set<bool>(Setting::TouchMode, state);
            },
            .graphics = T.display.settings.touchMode,
            .behaviour = Button::Behaviour::Toggle,
        }).value(Storage::flag(Setting::TouchMode));
    }

    // ------------------------------------------------

}

// ------------------------------------------------
