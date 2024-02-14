
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

        class Filter : public View {
        public:

            // ------------------------------------------------
            
            enum class Type { Bank, Type, Author };

            // ------------------------------------------------

            struct Settings {

                // ------------------------------------------------

                LoadPresetTab& self;
                Type type;

                // ------------------------------------------------

                std::size_t bankId; // Only set when type == Bank

                // ------------------------------------------------
                
                std::string value; // Type,Author,or Bank

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
            
            void foreach(std::function<void(const PresetDatabase::Bank::Preset&)> callback);

            // ------------------------------------------------
            
            std::string identifier();

            // ------------------------------------------------

        };

        // ------------------------------------------------

        class Preset : public View {
        public:

            // ------------------------------------------------

            struct Settings {

                // ------------------------------------------------

                LoadPresetTab& self;
                std::size_t presetId;

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

        LoadPresetTab(Context c, Settings s);

        // ------------------------------------------------

        void reloadFilters(Filter::Type type);
        void reload();

        // ------------------------------------------------

        void select(Filter& bank);

        // ------------------------------------------------
        
    private:
        ScrollView* m_Filters;
        ScrollView* m_Presets;
        TextView* m_Search;
        Button* m_SortButton;

        // ------------------------------------------------

        TabControl m_FilterTabs;

        // ------------------------------------------------

        Filter::Type m_CurrentType;

        // ------------------------------------------------

        struct State {

            // ------------------------------------------------

            Filter::Type type;
            std::string value; // Bank,Type,or Author
            float scrolled;
            std::string search;

            // ------------------------------------------------

        } m_State;

        // ------------------------------------------------

        void saveState();
        void loadState();

        // ------------------------------------------------
        
        void applySearch();
        void sortPresets(bool reverse);

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------
