
// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Processing/Interfaces.hpp"

// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Processing/Processor.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------
    
    std::size_t GeneralInfoInterface::activeVoices() {
        return self<MiniSynthFMProcessor>().bank.activeVoices();
    }
    
    // ------------------------------------------------
    
    float GeneralInfoInterface::sampleRate() {
        return self<MiniSynthFMProcessor>().sampleRate();
    }

    // ------------------------------------------------

    float GeneralInfoInterface::nanosPerSample() {
        return self<MiniSynthFMProcessor>().timerNanosPerSample;
    }

    float GeneralInfoInterface::percent() {
        return self<MiniSynthFMProcessor>().timerPercent;
    }

    // ------------------------------------------------

    float EnvelopeInterface::operator()() {
        std::size_t voice = self<MiniSynthFMProcessor>().bank.lastTriggered().settings.index;
        return self<MiniSynthFMProcessor>().voice.envelope[settings.index].output[voice];
    }

    // ------------------------------------------------

    float LfoInterface::operator()() {
        std::size_t voice = self<MiniSynthFMProcessor>().bank.lastTriggered().settings.index;
        return self<MiniSynthFMProcessor>().voice.lfo[settings.index].output[voice] * 0.5 + 0.5;
    }

    // ------------------------------------------------

    void ModInterface::operator()(ModSource source, ModDestination destination, bool val) {
        self<MiniSynthFMProcessor>().params.routing[(int)destination][(int)source] = val;
    }

    // ------------------------------------------------

    bool PianoInterface::pressed(Note note) {
        for (auto& voice : self<MiniSynthFMProcessor>().bank) {
            if (voice.pressed && voice.note == note) return true;
        }

        return false;
    }

    constexpr NoteID generateNoteId(Note note) {
        return (18ull << 7) + static_cast<int>(Math::clamp(note, 0, 127));
    }

    NoteID PianoInterface::noteOn(Note note, float velocity) {
        NoteID id = generateNoteId(note);
        addAsyncTask([this, note, velocity, id] {
            self<MiniSynthFMProcessor>().noteOnMPE(id, note, velocity, 0);
        });
        return id;
    }

    void PianoInterface::noteOff(Note note, float velocity) {
        NoteID id = generateNoteId(note);
        addAsyncTask([this, note, velocity, id] {
            self<MiniSynthFMProcessor>().noteOffMPE(id, note, velocity, 0);
        });
    }

    void PianoInterface::pitchBend(NoteID id, float amt) {
        addAsyncTask([this, id, amt] {
            self<MiniSynthFMProcessor>().notePitchBendMPE(id, amt);
        });
    }

    // ------------------------------------------------

    float ModWheelInterface::operator()() {
        return self<MiniSynthFMProcessor>().params.modWheel;
    }

    // ------------------------------------------------
    
    float VelocityInterface::operator()() {
        return self<MiniSynthFMProcessor>().bank.lastTriggered().velocity;
    }
    
    // ------------------------------------------------
    
    float RandomInterface::operator()() {
        std::size_t voice = self<MiniSynthFMProcessor>().bank.lastTriggered().settings.index;
        return self<MiniSynthFMProcessor>().voice.randomValue[voice];
    }

    // ------------------------------------------------

}

// ------------------------------------------------
