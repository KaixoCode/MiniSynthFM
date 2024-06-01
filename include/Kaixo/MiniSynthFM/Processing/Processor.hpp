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

        void noteOnMPE(NoteID id, Note note, double velocity, int channel) override;
        void notePitchBendMPE(NoteID id, double value) override;
        void noteOffMPE(NoteID id, Note note, double velocity, int channel) override;

        // ------------------------------------------------
        
        void quality(float val);
        void quality(Quality val);
        
        void exportQuality(float val);
        void exportQuality(Quality val);

        void phaseMode(float val);
        void phaseMode(PhaseMode val);

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
        float timerNanosPerSample = 0;
        float timerPercentMax = 0;
        float timerNanosPerSampleMax = 0;
        std::chrono::time_point<std::chrono::steady_clock> lastMeasure;

        // ------------------------------------------------

    private:
        Quality m_ExportQuality;
        Quality m_LiveQuality;

        // ------------------------------------------------

    };
    
    // ------------------------------------------------

}
    
// ------------------------------------------------
