
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
        auto interface = context.interface<Processing::TimerInterface>();
        auto percent = interface->percent();
        cpuUsage->settings.text = std::format("{:.2f} %", percent);
        cpuUsage->repaint();
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
            .callback = [&](bool val) {
                advancedInfo.select(val);
            },
            .graphics = T.display.main.advancedInfo.button,
            .behaviour = Button::Behaviour::Toggle
        });

        // ------------------------------------------------

        add<ImageView>({ .image = T.display.main.foreground, .enableMouse = false });

        // ------------------------------------------------
        
        auto& advanced = add<View>({ 6, 6, 300, 74 });

        // ------------------------------------------------

        advanced.add<ImageView>({ .image = T.display.main.advancedInfo.background });

        cpuUsage = &advanced.add<Button>({ 0, 0, 300, 20 }, {
            .graphics = T.display.main.advancedInfo.cpuUsage,
        });
        
        std::string optimizations = "No SIMD registers available";
        switch (simd_path::path) {
        case simd_path::P0: optimizations = "No SIMD registers available"; break;
        case simd_path::P1: optimizations = "SSE/SSE2"; break;
        case simd_path::P2: optimizations = "SSE/SSE2/3/4.1"; break;
        case simd_path::P3: optimizations = "SSE/SSE2/3/4.1 AVX/AVX2"; break;
        }

        simdOptimizations = &advanced.add<Button>({ 0, 22, 300, 20 }, {
            .graphics = T.display.main.advancedInfo.simdOptimizations,
            .text = optimizations,
        });

        advanced.add<ImageView>({ .image = T.display.main.advancedInfo.foreground, .enableMouse = false });

        // ------------------------------------------------

        advancedInfo.add(1, advanced);
        advancedInfo.select(0);

        // ------------------------------------------------

    }

    // ------------------------------------------------

}

// ------------------------------------------------
