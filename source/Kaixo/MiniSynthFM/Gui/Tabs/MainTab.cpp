
// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Gui/Tabs/MainTab.hpp"

// ------------------------------------------------

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

    void MainTab::onIdle() {
        float value = context.interface<Processing::TimerInterface>()();
        timer->settings.text = std::format("{:.2f} %", value);
        timer->repaint();
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

        presetName = &add<TextView>({ 6, 83, 256, 20 }, {
            .graphics = T.display.main.presetName,
            .padding = { 4, 3 },
            .multiline = false,
            .editable = false,
            .lineHeight = 14,
        });

        add<Button>({ 264, 83, 20, 20 }, {
            .callback = [&](bool) {
                context.controller<MiniSynthFMController>().presetDatabase.loadPreviousPreset();
            },
            .graphics = T.display.main.previousPreset,
        });

        add<Button>({ 286, 83, 20, 20 }, {
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

        timer = &add<TextView>({ 6, 6, 100, 20 }, {
            .graphics = T.display.main.presetName,
            .padding = { 4, 3 },
            .multiline = false,
            .editable = false,
            .lineHeight = 14,
        });

        // ------------------------------------------------

        add<ImageView>({ .image = T.display.main.foreground, .enableMouse = false });

        // ------------------------------------------------
        
        // TODO: toggle to view advanced information like
        //  - CPU usage
        //  - Which SIMD types are used
        //  - etc.

        // ------------------------------------------------

    }

    // ------------------------------------------------

}

// ------------------------------------------------
