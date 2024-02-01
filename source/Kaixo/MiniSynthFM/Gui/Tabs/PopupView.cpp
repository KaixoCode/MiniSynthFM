
// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Gui/Tabs/PopupView.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    PopupView::PopupView(Context c)
        : View(c)
    {

        // ------------------------------------------------

        add<ImageView>({ .image = T.display.popup.background });

        // ------------------------------------------------

        m_BackButton = &add<Button>({ 80, 100, 96, 20 }, {
            .callback = [this](bool) {
                if (m_Callback) m_Callback(false);
                if (--m_Requests == 0) setVisible(false);
            },
            .graphics = T.display.popup.backButton,
        });

        add<Button>({ 178, 100, 96, 20 }, {
            .callback = [this](bool) {
                if (m_Callback) m_Callback(true);
                if (--m_Requests == 0) setVisible(false);
            },
            .graphics = T.display.popup.confirmButton
        });

        m_Message = &add<TextView>({ 80, 60, 194, 38 }, {
            .graphics = T.display.popup.message,
            .padding = { 4, 3 },
            .multiline = true,
            .editable = false,
            .lineHeight = 16,
        });

        // ------------------------------------------------

        setVisible(false);

        // ------------------------------------------------

    }

    // ------------------------------------------------

    void PopupView::open(auto c, std::string_view text, bool withBack) {
        m_Callback = c;
        m_Message->setText(text);
        m_Requests++;
        m_BackButton->setVisible(withBack);
        setVisible(true);
    }

    // ------------------------------------------------

}

// ------------------------------------------------
