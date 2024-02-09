#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Processing/Processor.hpp"
#include "Kaixo/Core/Processing/VoiceBank.hpp"
#include "Kaixo/Core/Processing/ParameterDatabase.hpp"

// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Controller.hpp"
#include "Kaixo/MiniSynthFM/Processing/Voice.hpp"
#include "Kaixo/MiniSynthFM/Processing/Delay.hpp"

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
        
        void quality(float val);
        void quality(Quality val);
        
        void exportQuality(float val);
        void exportQquality(Quality val);

        // ------------------------------------------------

        ParameterDatabase<MiniSynthFMProcessor> parameters{ this };
        VoiceParameters params{};
        MiniSynthFMVoice voice{ params };
        Delay delay{};

        // ------------------------------------------------
        
        VoiceBank<VoiceBankVoice, Voices> bank{
            VoiceBankVoice::Settings{ voice, 0ull }, 
            VoiceBankVoice::Settings{ voice, 1ull }, 
            VoiceBankVoice::Settings{ voice, 2ull }, 
            VoiceBankVoice::Settings{ voice, 3ull }, 
            VoiceBankVoice::Settings{ voice, 4ull }, 
            VoiceBankVoice::Settings{ voice, 5ull }, 
            VoiceBankVoice::Settings{ voice, 6ull }, 
            VoiceBankVoice::Settings{ voice, 7ull }, 
        };

        // ------------------------------------------------
        
        void init() override;
        basic_json serialize() override;
        void deserialize(basic_json& data) override;

        // ------------------------------------------------
        
        float timerPercent = 0;

        // ------------------------------------------------

    };
    
    // ------------------------------------------------

}
    
// ------------------------------------------------
