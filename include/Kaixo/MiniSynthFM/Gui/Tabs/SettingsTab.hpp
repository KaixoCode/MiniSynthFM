
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

        Button* themePath;
        Button* presetPath;
        Button* cpuUsage = nullptr;
        Button* sampleRate = nullptr;
        Button* activeVoices = nullptr;

        // ------------------------------------------------

        juce::FileChooser themeChooser{ "Choose a MiniFM theme.", {}, "*.minifmtheme" };
        juce::FileChooser presetPathChooser{ "Choose a MiniFM preset folder." };

        // ------------------------------------------------

        void onIdle() override;

        // ------------------------------------------------

        SettingsTab(Context c, Settings s);

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------
