
// ------------------------------------------------

#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Gui/Views/ImageView.hpp"
#include "Kaixo/Core/Gui/Views/ScrollView.hpp"
#include "Kaixo/Core/Gui/Views/TextView.hpp"
#include "Kaixo/Core/Gui/Knob.hpp"
#include "Kaixo/Core/Gui/Button.hpp"
#include "Kaixo/Core/Gui/TabControl.hpp"
#include "Kaixo/Core/Theme/Theme.hpp"

// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Controller.hpp"
#include "Kaixo/MiniSynthFM/Gui/Tabs/PopupView.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------
    
    constexpr TabControl::Id SavePresetTabControl = 0x131FF0AB;

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

        void presetSaved() override;
        void presetLoaded() override;

        void reloadPresetInformation();

        // ------------------------------------------------

        TextView* name = nullptr;
        TextView* author = nullptr;
        TextView* type = nullptr;
        TextView* description = nullptr;

        // ------------------------------------------------

        PresetTab(Context c, Settings s);

        // ------------------------------------------------

        void savePreset(bool force);

        // ------------------------------------------------

        void resultHandler(SaveResult result);

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------
