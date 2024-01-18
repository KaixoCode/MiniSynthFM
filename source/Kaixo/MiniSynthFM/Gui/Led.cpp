
// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Gui/Led.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    Led::Led(Context c, Settings settings)
        : View(c), settings(std::move(settings))
    {
        wantsIdle(true);
    }

    // ------------------------------------------------

    void Led::paint(juce::Graphics& g) {

        // ------------------------------------------------

        settings.graphics.draw({
            .graphics = g,
            .bounds = localDimensions(),
            .value = m_Value
        });

        // ------------------------------------------------

    }

    // ------------------------------------------------

    void Led::onIdle() {
        float value = Math::clamp1(settings.value());
        if (m_Value != value) {
            m_Value = value;
            repaint();
        }
    }

    // ------------------------------------------------

}

// ------------------------------------------------
