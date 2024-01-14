#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Processing/Processor.hpp"
#include "Kaixo/Core/Processing/Voice.hpp"
#include "Kaixo/Core/Processing/VoiceBank.hpp"
#include "Kaixo/Core/Processing/Modules/Envelope.hpp"
#include "Kaixo/Core/Processing/ParameterDatabase.hpp"

// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Controller.hpp"
#include "Kaixo/MiniSynthFM/Processing/Voice.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {
    
    // ------------------------------------------------

    class MiniSynthFMProcessor : public Processor {
    public:

        // ------------------------------------------------
        
        MiniSynthFMProcessor();

        // ------------------------------------------------

        void process() override;
        
        // ------------------------------------------------

        void noteOn(Note note, double velocity, int channel) override;
        void noteOff(Note note, double velocity, int channel) override;

        // ------------------------------------------------

        ParameterDatabase<MiniSynthFMProcessor> parameters{ this };

        VoiceParameters params;
        
        VoiceBank<MiniSynthFMVoice, Voices> voices{ params, params, params, params, params, params, params, params };

        // ------------------------------------------------

    };
    
    // ------------------------------------------------

    class EnvelopeInterface : public SynchronousInterface<float()> {
    public:

        // ------------------------------------------------

        struct Settings {
            std::size_t index;
        } settings;

        // ------------------------------------------------

        float call() override {
            return self<MiniSynthFMProcessor>().voices.lastTriggered().envelope[settings.index].output;
        }

        // ------------------------------------------------

    };

    // ------------------------------------------------
    
    class LfoInterface : public SynchronousInterface<float()> {
    public:

        // ------------------------------------------------

        struct Settings {
            std::size_t index;
        } settings;

        // ------------------------------------------------

        float call() override {
            return self<MiniSynthFMProcessor>().voices.lastTriggered().envelope[settings.index].output;
        }

        // ------------------------------------------------

    };

    // ------------------------------------------------
    
    class ModInterface : public SynchronousInterface<void(ModSource, ModDestination, bool)> {
    public:

        // ------------------------------------------------

        void call(ModSource source, ModDestination destination, bool val) override {
            self<MiniSynthFMProcessor>().params.routing[(int)source][(int)destination] = val;
        }

        // ------------------------------------------------

    };

    // ------------------------------------------------

}
    
// ------------------------------------------------
