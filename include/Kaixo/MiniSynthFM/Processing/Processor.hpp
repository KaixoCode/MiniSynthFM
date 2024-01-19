#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Processing/Processor.hpp"
#include "Kaixo/Core/Processing/VoiceBank.hpp"
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
        
        void init() override;
        basic_json serialize() override;
        void deserialize(basic_json& data) override;

        // ------------------------------------------------

    };
    
    // ------------------------------------------------

}
    
// ------------------------------------------------
