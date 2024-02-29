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
    
    class Arpeggiator : public Module {
    public:

        // ------------------------------------------------

        Arpeggiator(VoiceBank<VoiceBankVoice, Voices>& bank);

        // ------------------------------------------------

        void process() override;

        // ------------------------------------------------

        void noteOn(Note note, double velocity, int channel);
        void noteOff(Note note, double velocity, int channel);

        // ------------------------------------------------

        void triggerNote(std::size_t noteIndex);
        bool releaseNote(std::size_t noteIndex);

        // ------------------------------------------------
        
        std::size_t samplesBetweenNotes() const;

        // ------------------------------------------------

    private:
        VoiceBank<VoiceBankVoice, Voices>& m_Bank;

        // ------------------------------------------------
        
        float m_GatePercent = 0.5;

        // ------------------------------------------------

        std::int64_t m_Timestamp = 0;
        std::size_t m_NoteTrigger = 0;
        std::size_t m_NoteRelease = 0;

        struct PressedNote {
            std::size_t lastTrigger = 0;
            double velocity = 0;
            Note note = 0;
            int channel = 0;
            bool active = true;
        };

        Vector<PressedNote, 32> m_NotesDown{};

        // ------------------------------------------------

    };

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
