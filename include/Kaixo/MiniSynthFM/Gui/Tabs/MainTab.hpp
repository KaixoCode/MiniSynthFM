
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

    class MainTab : public View, public DescriptionListener, public PresetListener {
    public:

        // ------------------------------------------------

        void updateDescription(std::string_view descr) override;

        // ------------------------------------------------

        void presetSaved() override;
        void presetLoaded() override;

        // ------------------------------------------------

        void reloadPresetName();

        // ------------------------------------------------

        TextView* description = nullptr;
        TextView* presetName = nullptr;
        
        // ------------------------------------------------
        
        TabControl advancedInfo{};

        // ------------------------------------------------

        struct Settings {

            // ------------------------------------------------

            PopupView& popup;

            // ------------------------------------------------

        } settings;

        // ------------------------------------------------

        MainTab(Context c, Settings s);

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------
