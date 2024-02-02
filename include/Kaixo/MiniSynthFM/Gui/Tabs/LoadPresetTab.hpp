
// ------------------------------------------------

#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Gui/Views/ImageView.hpp"
#include "Kaixo/Core/Gui/Views/ScrollView.hpp"
#include "Kaixo/Core/Gui/Views/TextView.hpp"
#include "Kaixo/Core/Gui/Knob.hpp"
#include "Kaixo/Core/Gui/Button.hpp"
#include "Kaixo/Core/Theme/Theme.hpp"

// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Controller.hpp"
#include "Kaixo/MiniSynthFM/Gui/Tabs/PopupView.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    class LoadPresetTab : public View {
    public:

        // ------------------------------------------------

        class Bank : public View {
        public:

            // ------------------------------------------------

            struct Settings {

                LoadPresetTab& self;
                std::size_t bankIndex;

            } settings;

            // ------------------------------------------------

            Theme::Drawable graphics;

            // ------------------------------------------------

            Bank(Context c, Settings s);

            // ------------------------------------------------

            void mouseDown(const juce::MouseEvent& e) override;

            // ------------------------------------------------

            void paint(juce::Graphics& g) override;

            // ------------------------------------------------

        };

        // ------------------------------------------------

        class Preset : public View {
        public:

            // ------------------------------------------------

            struct Settings {

                // ------------------------------------------------

                LoadPresetTab& self;
                std::size_t bankIndex;
                std::size_t presetIndex;

                // ------------------------------------------------

            } settings;

            // ------------------------------------------------

            Theme::Drawable graphics;
            std::string displayName;

            // ------------------------------------------------

            Preset(Context c, Settings s);

            // ------------------------------------------------

            void mouseDown(const juce::MouseEvent& e) override;

            // ------------------------------------------------

            void paint(juce::Graphics& g) override;

            // ------------------------------------------------

            PresetData presetData();

            void load();

            // ------------------------------------------------

            void reloadDisplayName();

            // ------------------------------------------------

            bool matchesSearch(std::string_view search);

            // ------------------------------------------------

        };

        // ------------------------------------------------

        struct Settings {

            // ------------------------------------------------

            PopupView& popup;

            // ------------------------------------------------

        } settings;

        // ------------------------------------------------
        
        struct State {

            // ------------------------------------------------

            std::string bank;
            float scrolled;

            // ------------------------------------------------

        } m_State;

        // ------------------------------------------------

        ScrollView* m_Banks;
        ScrollView* m_Presets;
        TextView* m_Search;

        // ------------------------------------------------

        LoadPresetTab(Context c, Settings s);

        // ------------------------------------------------

        void reloadBanks();

        // ------------------------------------------------

        void select(Bank& bank);

        // ------------------------------------------------
        
        void saveState();
        void loadState();

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------
