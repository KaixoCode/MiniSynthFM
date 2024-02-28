
// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Gui/Tabs/MainTab.hpp"

// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Gui/Tabs/SavePresetTab.hpp"
#include "Kaixo/MiniSynthFM/Processing/Interfaces.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    void MainTab::updateDescription(std::string_view descr) {
        description->setText(descr);
        description->repaint();
    }

    // ------------------------------------------------

    void MainTab::presetSaved() { reloadPresetName(); }
    void MainTab::presetLoaded() { reloadPresetName(); }

    // ------------------------------------------------

    void MainTab::reloadPresetName() {
        auto& data = context.data<PresetData>();
        std::string name = data.name;
        if (!data.type.empty()) name += " (" + data.type + ")";
        if (!data.author.empty()) name += " - " + data.author;
        presetName->settings.text = name;
        presetName->repaint();
    }

    // ------------------------------------------------

    MainTab::MainTab(Context c, Settings s)
        : View(c), settings(s)
    {
        // ------------------------------------------------

        wantsIdle(true);

        // ------------------------------------------------

        add<ImageView>({ .image = T.display.main.background });

        // ------------------------------------------------

        presetName = &add<TextView>({ 6, 83, 234, 20 }, {
            .graphics = T.display.main.presetName,
            .padding = { 4, 3 },
            .multiline = false,
            .editable = false,
            .lineHeight = 14,
        });

        add<Button>({ 242, 83, 20, 20 }, {
            .callback = [&](bool) {
                context.controller<MiniSynthFMController>().presetDatabase.loadPreviousPreset();
            },
            .graphics = T.display.main.previousPreset,
        });

        add<Button>({ 264, 83, 20, 20 }, {
            .callback = [&](bool) {
                context.controller<MiniSynthFMController>().presetDatabase.loadNextPreset();
            },
            .graphics = T.display.main.nextPreset,
        });

        description = &add<TextView>({ 6, 105, 300, 54 }, {
            .graphics = T.display.main.description,
            .padding = { 4, 3 },
            .multiline = true,
            .editable = false,
            .lineHeight = 16,
        });

        // ------------------------------------------------

        add<Button>({ 286, 83, 20, 20 }, {
            .callback = [&](bool) {
                context.tabControl(SavePresetTabControl).select(1);
            },
            .graphics = T.display.main.advancedInfo.button
        });

        // ------------------------------------------------
        
        add<ImageView>({ .image = T.display.main.foreground, .enableMouse = false });

        // ------------------------------------------------
        
        context.tabControl(SavePresetTabControl).add(1, add<PresetTab>({ 0, 0, 312, 165 }, {
            .popup = settings.popup
        }));

        context.tabControl(SavePresetTabControl).select(0);

        // ------------------------------------------------

    }

    // ------------------------------------------------

}

// ------------------------------------------------
