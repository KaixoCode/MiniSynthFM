
// ------------------------------------------------

#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Gui/Views/ImageView.hpp"
#include "Kaixo/Core/Gui/Views/TextView.hpp"
#include "Kaixo/Core/Gui/Button.hpp"
#include "Kaixo/Core/Theme/Theme.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    class PopupView : public View {
    public:

        // ------------------------------------------------

        PopupView(Context c);

        // ------------------------------------------------

        void open(std::function<void(bool)> c, std::string_view text, bool withBack = true);

        // ------------------------------------------------

    private:
        std::function<void(bool)> m_Callback;
        TextView* m_Message;
        std::size_t m_Requests = 0;
        Button* m_BackButton;

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------
