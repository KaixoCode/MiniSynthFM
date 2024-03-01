
// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Processing/Arpeggiator.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    Arpeggiator::Arpeggiator(VoiceBank<VoiceBankVoice, Voices>& bank)
        : m_Bank(bank)
    {}

    // ------------------------------------------------

    void Arpeggiator::process() {
        if (m_NotesDown.empty()) return;

        std::size_t samples = samplesBetweenNotes();
        std::size_t gate = m_GatePercent * samples;

        if (m_Timestamp % samples == 0) {
            m_NoteTrigger %= m_NotesDown.size();
            triggerNote(m_NoteTrigger);
            m_NoteTrigger++;
        }

        m_NoteRelease %= m_NotesDown.size();
        if (inSequence(m_NoteRelease).lastTrigger + gate < m_Timestamp) {
            bool removed = releaseNote(m_NoteRelease);
            if (!removed) m_NoteRelease++;
        }

        m_Timestamp += 1;
    }

    // ------------------------------------------------

    void Arpeggiator::noteOn(Note note, double velocity, int channel) {
        if (!m_Enabled) {
            m_Bank.noteOn(note, velocity, channel);
            return;
        }

        if (m_NotesDown.empty()) { // First note, reset timestamp
            m_Timestamp = 0;
            m_NoteTrigger = 0;
            m_NoteRelease = 0;
        }
        // Note was already pressed, but inactive, activate again
        for (auto& pressed : m_NotesDown) {
            if (pressed.note == note) {
                pressed.active = true;
                return;
            }
        }
        // Note cache is full, remove oldest
        if (m_NotesDown.full()) {
            m_NotesDown.pop_front();
        }
        // Add pressed note to cache
        m_NotesDown.push_back({
            .velocity = velocity,
            .note = note,
            .channel = channel,
        });

        createNoteOrder();
    }

    void Arpeggiator::noteOff(Note note, double velocity, int channel) {
        if (!m_Enabled) {
            m_Bank.noteOff(note, velocity, channel);
        }

        for (auto& pressed : m_NotesDown) {
            if (pressed.note == note) {
                pressed.active = false;
                return;
            }
        }
    }

    // ------------------------------------------------

    void Arpeggiator::triggerNote(std::size_t noteIndex) {
        auto& note = inSequence(noteIndex);
        note.lastTrigger = m_Timestamp;
        m_Bank.noteOn(note.note, note.velocity, note.channel);
    }

    bool Arpeggiator::releaseNote(std::size_t noteIndex) {
        auto& note = inSequence(noteIndex);
        m_Bank.noteOff(note.note, note.velocity, note.channel);

        if (!note.active) {
            m_NotesDown.erase_index(m_NoteOrder[noteIndex]);
            createNoteOrder();
            return true;
        }

        return false;
    }

    // ------------------------------------------------

    std::size_t Arpeggiator::samplesBetweenNotes() const {
        auto bars = [&]() {
            switch (m_Tempo) {
            case Tempo::T1_1:   return 1.f;
            case Tempo::T1_2:   return 0.5f;
            case Tempo::T1_4:   return 0.25f;
            case Tempo::T1_6:   return 0.1875f;
            case Tempo::T1_8:   return 0.125f;
            case Tempo::T1_16:  return 0.0625f;
            case Tempo::T1_32:  return 0.03125f;
            case Tempo::T1_64:  return 0.015625f;
            }
            };

        float time = m_Time;
        if (m_Synced) {
            float nmrBarsForTempo = bars();
            float beatsPerSecond = bpm() / 60;
            float beatsPerBar = timeSignature().numerator;
            float secondsPerBar = beatsPerBar / beatsPerSecond;
            float seconds = nmrBarsForTempo * secondsPerBar;
            time = seconds * 1000;
        }

        return sampleRate() * time / 1000.;
    }

    // ------------------------------------------------

    void Arpeggiator::mode(Mode val) { m_Mode = val; }
    void Arpeggiator::mode(float val) { mode(normalToIndex(val, Mode::Amount)); }

    void Arpeggiator::tempo(Tempo val) { m_Tempo = val; }
    void Arpeggiator::tempo(float val) { tempo(normalToIndex(val, Tempo::Amount)); }

    void Arpeggiator::time(float ms) { m_Time = ms; }
    void Arpeggiator::gate(float percent) { m_GatePercent = percent; }

    void Arpeggiator::synced(bool sync) { m_Synced = sync; }

    void Arpeggiator::enable(bool enabled) { m_Enabled = enabled; }

    // ------------------------------------------------

    Arpeggiator::PressedNote& Arpeggiator::inSequence(std::size_t index) {
        return m_NotesDown[m_NoteOrder[index]];
    }

    // ------------------------------------------------

    void Arpeggiator::createNoteOrder() {
        m_NoteOrder.clear();
        auto sortUp = [this](auto& values) {
            std::ranges::sort(values, [this](auto a, auto b) -> bool {
                return m_NotesDown[a].note < m_NotesDown[b].note;
            });
        };

        auto sortDown = [this](auto& values) {
            std::ranges::sort(values, [this](auto a, auto b) -> bool {
                return m_NotesDown[a].note > m_NotesDown[b].note;
            });
        };

        switch (m_Mode) {
        case Mode::Up: {
            for (std::size_t i = 0; i < m_NotesDown.size(); ++i)
                m_NoteOrder.push_back(i);

            sortUp(m_NoteOrder);
            break;
        }
        case Mode::Down: {
            for (std::size_t i = 0; i < m_NotesDown.size(); ++i)
                m_NoteOrder.push_back(i);

            sortDown(m_NoteOrder);
            break;
        }
        case Mode::UpDown: {
            decltype(m_NoteOrder) temp{};
            for (std::size_t i = 0; i < m_NotesDown.size(); ++i)
                temp.push_back(i);

            sortUp(temp);
            m_NoteOrder.push_back_all(temp);
            temp.erase(temp.end() - 1); // Remove duplicate
            sortDown(temp);
            m_NoteOrder.push_back_all(temp);
            break;
        }
        case Mode::DownUp: {
            decltype(m_NoteOrder) temp{};
            for (std::size_t i = 0; i < m_NotesDown.size(); ++i)
                temp.push_back(i);

            sortDown(temp);
            m_NoteOrder.push_back_all(temp);
            temp.erase(temp.end() - 1); // Remove duplicate
            sortUp(temp);
            m_NoteOrder.push_back_all(temp);
            break;
        }        
        case Mode::UpAndDown: {
            decltype(m_NoteOrder) temp{};
            for (std::size_t i = 0; i < m_NotesDown.size(); ++i)
                temp.push_back(i);

            sortUp(temp);
            m_NoteOrder.push_back_all(temp);
            sortDown(temp);
            m_NoteOrder.push_back_all(temp);
            break;
        }
        case Mode::DownAndUp: {
            decltype(m_NoteOrder) temp{};
            for (std::size_t i = 0; i < m_NotesDown.size(); ++i)
                temp.push_back(i);

            sortDown(temp);
            m_NoteOrder.push_back_all(temp);
            sortUp(temp);
            m_NoteOrder.push_back_all(temp);
            break;
        }
        }
    }
}