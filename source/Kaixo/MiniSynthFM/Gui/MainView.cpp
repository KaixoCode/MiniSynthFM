
// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Gui/MainView.hpp"

// ------------------------------------------------

#include "Kaixo/Core/Gui/Views/ImageView.hpp"
#include "Kaixo/Core/Gui/Views/ScrollView.hpp"
#include "Kaixo/Core/Gui/Views/FilterDisplay.hpp"
#include "Kaixo/Core/Gui/Knob.hpp"
#include "Kaixo/Core/Theme/Theme.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    MainView::MainView(Context c)
        : View(c)
    {

        // ------------------------------------------------

        wantsIdle(true);

        // ------------------------------------------------
        
        context.tooltip().background(T.tooltip.background);
        context.tooltip().font(T.tooltip.font);
        context.tooltip().textColor(T.tooltip.textColor);

        // ------------------------------------------------

        add<ImageView>({ T.background });

        // ------------------------------------------------
        
        add<Knob>({ 0, 0, 50, 50 }, {
            .graphics = T.knob,
            .param = Synth.oscillator[0].tune
        });
        
        add<Knob>({ 0, 50, 50, 50 }, {
            .graphics = T.knob,
            .param = Synth.oscillator[1].tune
        });
        
        add<Knob>({ 0, 100, 50, 50 }, {
            .graphics = T.knob,
            .param = Synth.oscillator[2].tune
        });
        
        add<Knob>({ 50, 0, 50, 50 }, {
            .graphics = T.knob,
            .param = Synth.oscillator[0].volume
        });
        
        add<Knob>({ 50, 50, 50, 50 }, {
            .graphics = T.knob,
            .param = Synth.oscillator[1].volume
        });
        
        add<Knob>({ 50, 100, 50, 50 }, {
            .graphics = T.knob,
            .param = Synth.oscillator[2].volume
        });
        
        add<Knob>({ 100, 0, 50, 50 }, {
            .graphics = T.knob,
            .param = Synth.envelope[0].attack
        });
        
        add<Knob>({ 100, 50, 50, 50 }, {
            .graphics = T.knob,
            .param = Synth.envelope[0].decay
        });
        
        add<Knob>({ 100, 100, 50, 50 }, {
            .graphics = T.knob,
            .param = Synth.envelope[0].sustain
        });
        
        add<Knob>({ 100, 150, 50, 50 }, {
            .graphics = T.knob,
            .param = Synth.envelope[0].release
        });

        // ------------------------------------------------
        
    }

    // ------------------------------------------------

    View* createBaseView(Context context) { return new MainView{ context }; }

    // ------------------------------------------------

}
    
// ------------------------------------------------
