
// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Gui/DisplayView.hpp"

// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Gui/Tabs/MainTab.hpp"
#include "Kaixo/MiniSynthFM/Gui/Tabs/SavePresetTab.hpp"
#include "Kaixo/MiniSynthFM/Gui/Tabs/SettingsTab.hpp"
#include "Kaixo/MiniSynthFM/Gui/Tabs/PopupView.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    DisplayView::DisplayView(Context c)
        : View(c)
    {

        // ------------------------------------------------

        add<ImageView>({ .image = T.display.background });

        // ------------------------------------------------

        auto& popup = add<PopupView>();

        // ------------------------------------------------

        auto& tabs = context.tabControl(0);

        tabs.add(0, add<MainTab>({ .popup = popup }));
        tabs.add(1, add<PresetTab>({ .popup = popup }));
        tabs.add(2, *(load = &add<LoadPresetTab>({ .popup = popup })));
        tabs.add(3, add<SettingsTab>({ .popup = popup }));

        // ------------------------------------------------

        tabs.addButton(0, add<Button>({ 313, 6, 35, 35 }, {
            .graphics = T.display.main.button
        }));

        tabs.addButton(1, add<Button>({ 313, 45, 35, 35 }, {
            .graphics = T.display.savePreset.button
        }));

        tabs.addButton(2, add<Button>({ 313, 85, 35, 35 }, {
            .graphics = T.display.loadPreset.button
        }));

        tabs.addButton(3, add<Button>({ 313, 124, 35, 35 }, {
            .graphics = T.display.settings.button
        }));

        // ------------------------------------------------

        tabs.tab(2).addCallback([&](bool v) {
            if (v) {
                load->reload();
            }
        });

        // ------------------------------------------------

        tabs.select(0);

        // ------------------------------------------------

        // Move popup to end of views, so it draws on top
        removeChildComponent(&popup);
        addChildComponent(&popup);

        // ------------------------------------------------

        add<ImageView>({ .image = T.display.foreground, .enableMouse = false });

        // ------------------------------------------------

    }

    // ------------------------------------------------

}

// ------------------------------------------------
