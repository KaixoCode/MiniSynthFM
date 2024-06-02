#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Gui/View.hpp"
#include "Kaixo/Core/Gui/TabControl.hpp"
#include "Kaixo/Core/Theme/Theme.hpp"

// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Controller.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    class MainView : public View {
    public:

        // ------------------------------------------------

        MainView(Context c);

        // ------------------------------------------------
        
        void paint(juce::Graphics& g) override;
        void paintOverChildren(juce::Graphics& g) override;

        // ------------------------------------------------
        
        Theme::Drawable background;
        Theme::Drawable backgroundNoPiano;
        Theme::Drawable foreground;
        Theme::Drawable foregroundNoPiano;

        // ------------------------------------------------
        
        Button* hideInfoButton;
        Button* openWebsiteButton;

        // ------------------------------------------------

    };
    
    // ------------------------------------------------

}
    
// ------------------------------------------------
