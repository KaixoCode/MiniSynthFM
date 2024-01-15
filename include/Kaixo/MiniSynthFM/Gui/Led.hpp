#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Gui/View.hpp"
#include "Kaixo/Core/Theme/Theme.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    class Led : public View {
    public:

        // ------------------------------------------------

        struct Settings {

            // ------------------------------------------------

            Theme::Drawable graphics;
            Processing::InterfaceStorage<float()> value;

            // ------------------------------------------------

        } settings;

        // ------------------------------------------------

        Led(Context c, Settings settings);

        // ------------------------------------------------

        void paint(juce::Graphics& g) override;

        // ------------------------------------------------

        void onIdle() override;

        // ------------------------------------------------
    private:
        float m_Value = 0;

        // ------------------------------------------------

    };
    
    // ------------------------------------------------

}
    
// ------------------------------------------------
