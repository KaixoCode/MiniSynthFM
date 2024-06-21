
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

    class LoadPresetTab : public View {
    public:

        // ------------------------------------------------

        class Preset;
        class Filter : public View {
        public:

            // ------------------------------------------------

            struct Settings {

                // ------------------------------------------------

                LoadPresetTab& self;
                std::function<bool(Preset&)> match;
                std::string name;

                // ------------------------------------------------

            } settings;

            // ------------------------------------------------

            Theme::Drawable graphics;

            // ------------------------------------------------

            Filter(Context c, Settings s);

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
                PresetDatabase::Preset::Interface preset{};
                bool isInit = false;

                // ------------------------------------------------

            } settings;

            // ------------------------------------------------

            Theme::Drawable graphics;
            std::string displayName;

            // ------------------------------------------------

            Preset(Context c, Settings s);

            // ------------------------------------------------

            void mouseDown(const juce::MouseEvent& e) override { settings.preset.load(); }

            // ------------------------------------------------

            void paint(juce::Graphics& g) override;

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

        LoadPresetTab(Context c, Settings s);

        // ------------------------------------------------

        void reload();

        // ------------------------------------------------

        void select(Filter& filter);

        // ------------------------------------------------
        
    private:
        ScrollView* m_Filters;
        ScrollView* m_Presets;
        TextView* m_Search;
        Button* m_SortButton;

        // ------------------------------------------------

        TabControl m_FilterTabs;

        // ------------------------------------------------

        enum class FilterType { Bank, Type, Author } m_CurrentType;

        // ------------------------------------------------

        struct State {

            // ------------------------------------------------

            FilterType type;
            std::string value; // Bank,Type,or Author
            float scrolled;
            std::string search;

            // ------------------------------------------------

        } m_State;

        std::function<bool(Preset&)> m_CurrentFilter{};

        // ------------------------------------------------

        void reloadFilters(FilterType type);
        void reloadPresets();

        // ------------------------------------------------

        void saveState();
        void loadState();

        // ------------------------------------------------
        
        void sortPresets();
        void showFilteredPresets();

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------
