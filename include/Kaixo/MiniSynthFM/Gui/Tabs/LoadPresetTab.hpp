
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
        
        enum class FilterType { Bank, Type, Author };

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
            void mouseDrag(const juce::MouseEvent& e) override;
            void mouseUp(const juce::MouseEvent& e) override;

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

            FilterType displayNameType = FilterType::Bank;
            Theme::Drawable graphics;
            std::string displayName;

            // ------------------------------------------------

            Preset(Context c, Settings s);

            // ------------------------------------------------

            void mouseDown(const juce::MouseEvent& e) override;
            void mouseDrag(const juce::MouseEvent& e) override;
            void mouseUp(const juce::MouseEvent& e) override;

            // ------------------------------------------------

            void paint(juce::Graphics& g) override;
            void onIdle() override;

            // ------------------------------------------------

            void reloadDisplayName();
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
        
        void onIdle() override;

        // ------------------------------------------------

        void select(Filter& filter);

        // ------------------------------------------------

        void reload();

        // ------------------------------------------------
        
    private:
        ScrollView* m_Filters;
        ScrollView* m_Presets;
        TextView* m_Search;
        Button* m_SortButton;
        Button* m_ReloadButton;

        // ------------------------------------------------
        
        bool m_Reloaded = false;

        // ------------------------------------------------

        TabControl m_FilterTabs;

        // ------------------------------------------------

        FilterType m_CurrentType;

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

        void createAll();
        void createFilters(FilterType type);
        void createPresets();

        // ------------------------------------------------

        void saveState();
        void loadState();

        // ------------------------------------------------
        
        void sortPresets(bool reverse = false);
        void showFilteredPresets();

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------
