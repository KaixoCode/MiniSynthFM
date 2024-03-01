#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Processing/Module.hpp"
#include "Kaixo/Core/Processing/VoiceBank.hpp"

// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Controller.hpp"
#include "Kaixo/MiniSynthFM/Processing/Voice.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    class Arpeggiator : public Module {
    public:

        // ------------------------------------------------

        enum class Mode {
            Up, Down, UpDown, DownUp, UpAndDown, DownAndUp, Amount
        };

        enum class Tempo {
            T1_64, T1_32, T1_16, T1_8, T1_6, T1_4, T1_2, T1_1, Amount
        };

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

        void mode(Mode val);
        void mode(float val);
        void tempo(Tempo val);
        void tempo(float val);
        void time(float ms);
        void gate(float percent);
        void synced(bool sync);
        void enable(bool enabled);

        // ------------------------------------------------

    private:
        VoiceBank<VoiceBankVoice, Voices>& m_Bank;

        // ------------------------------------------------

        float m_GatePercent = 0.5;

        // ------------------------------------------------

        Mode m_Mode{};
        Tempo m_Tempo{};
        float m_Time = 100; // ms
        bool m_Synced = true;
        bool m_Enabled = false;

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
        Vector<std::size_t, 32> m_NoteOrder{};

        PressedNote& inSequence(std::size_t index);

        // ------------------------------------------------

        void createNoteOrder();

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------
