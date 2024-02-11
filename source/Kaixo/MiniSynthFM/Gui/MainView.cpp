
// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Gui/MainView.hpp"

// ------------------------------------------------

#include "Kaixo/Core/Gui/Views/ImageView.hpp"
#include "Kaixo/Core/Theme/Theme.hpp"

// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Controller.hpp"
#include "Kaixo/MiniSynthFM/Gui/PatchBay.hpp"
#include "Kaixo/MiniSynthFM/Gui/Piano.hpp"
#include "Kaixo/MiniSynthFM/Gui/DisplayView.hpp"
#include "Kaixo/MiniSynthFM/Gui/Panels/DelayPanel.hpp"
#include "Kaixo/MiniSynthFM/Gui/Panels/EnvelopePanel.hpp"
#include "Kaixo/MiniSynthFM/Gui/Panels/FilterPanel.hpp"
#include "Kaixo/MiniSynthFM/Gui/Panels/GainPanel.hpp"
#include "Kaixo/MiniSynthFM/Gui/Panels/LfoPanel.hpp"
#include "Kaixo/MiniSynthFM/Gui/Panels/MidiPanel.hpp"
#include "Kaixo/MiniSynthFM/Gui/Panels/OperatorPanel.hpp"
#include "Kaixo/MiniSynthFM/Processing/Interfaces.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    MainView::MainView(Context c)
        : View(c)
    {

        // ------------------------------------------------
 
        background = T.background;
        backgroundNoPiano = T.backgroundNoPiano;
        foreground = T.foreground;
        foregroundNoPiano = T.foregroundNoPiano;

        // ------------------------------------------------
 
        if (Storage::flag(ShowPiano)) {
            context.window().setSize(SYNTH_InitialSize);
        } else {
            context.window().setSize(1205, 476);
        }
        
        // ------------------------------------------------

        wantsIdle(true);

        // ------------------------------------------------

        add<DisplayView>({ 486, 30, 355, 165 });

        // ------------------------------------------------
        
        auto& patchBay = add<PatchBay>();
        
        // ------------------------------------------------
        
        add<FilterPanel>({ 190, 30, 143, 165 }, { 
            .patchBay = patchBay 
        });
        
        // ------------------------------------------------
        
        add<DelayPanel>({ 338, 30, 143, 165 }, {
            .patchBay = patchBay    
        });

        // ------------------------------------------------

        for (std::size_t i = 0; i < Oscillators; ++i) {

            // ------------------------------------------------

            add<OperatorPanel>({ 35 + i * 270, 200, 265, 165 }, { 
                .index = i, 
                .patchBay = patchBay 
            });

            // ------------------------------------------------

        }

        // ------------------------------------------------
        
        for (std::size_t i = 0; i < Lfos; ++i) {

            // ------------------------------------------------
            
            add<LfoPanel>({ 437, 370, 403, 76 }, {
                .index = i,
                .patchBay = patchBay
            });

            // ------------------------------------------------

        }

        // ------------------------------------------------
        
        for (std::size_t i = 0; i < ADSREnvelopes; ++i) {

            // ------------------------------------------------
           
            add<EnvelopePanel>({ 845, 30 + i * 170, 325, 165 }, {
                .index = i,
                .patchBay = patchBay
            });

            // ------------------------------------------------

        }

        // ------------------------------------------------
        
        add<GainPanel>({ 845, 370, 325, 76 }, {
            .patchBay = patchBay    
        });
        
        // ------------------------------------------------
        
        add<MidiPanel>({ 35, 370, 397, 76 }, {
            .patchBay = patchBay    
        });

        // ------------------------------------------------
        
        add<Piano>({ 150, 476, 1020, 200 }, {
            .start = 36,
            .notes = 12 * 4 + 1, // 4 octaves + C
            .interface = context.interface<Processing::PianoInterface>(),
            .white = {
                .size = { 30, 200 },
                .graphics = T.piano.whiteKey
            },
            .black = {
                .size = { 23, 120 },
                .graphics = T.piano.blackKey
            },
            .spacing = 5,
        });

        // ------------------------------------------------
        
        add<Knob>({ 85, 491, 35, 145 }, {
            .graphics = T.piano.modWheel,
            .tooltipName = false,
            .tooltipValue = false,
            .param = Synth.modWheelParameter
        });
        
        add<Knob>({ 35, 491, 35, 145 }, {
            .onchange = [&](ParamValue val) { 
                context.beginEdit(Synth.pitchBendParameter); 
                context.performEdit(Synth.pitchBendParameter, 0.5); 
                context.endEdit(Synth.pitchBendParameter); 
            },
            .graphics = T.piano.pitchWheel,
            .tooltipName = false,
            .tooltipValue = false,
            .param = Synth.pitchBendParameter
        });

        // ------------------------------------------------
        
        // Move patch bay to end of views, so it draws on top
        removeChildComponent(&patchBay);
        addChildComponent(&patchBay);

        // ------------------------------------------------
        
    }

    // ------------------------------------------------
    
    void MainView::paint(juce::Graphics& g) {
        if (Storage::flag(ShowPiano)) {
            background.draw({ .graphics = g, .bounds = getLocalBounds() });
        } else {
            backgroundNoPiano.draw({ .graphics = g, .bounds = getLocalBounds() });
        }
    }
    
    // ------------------------------------------------
    
    void MainView::paintOverChildren(juce::Graphics& g) {
        if (Storage::flag(ShowPiano)) {
            foreground.draw({ .graphics = g, .bounds = getLocalBounds() });
        } else {
            foregroundNoPiano.draw({ .graphics = g, .bounds = getLocalBounds() });
        }
    }

    // ------------------------------------------------

    View* createBaseView(Context context) { return new MainView{ context }; }

    // ------------------------------------------------

}
    
// ------------------------------------------------
