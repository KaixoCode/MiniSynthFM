
// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Processing/Interfaces.hpp"

// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Processing/Processor.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    float TimerInterface::operator()() {
        return self<MiniSynthFMProcessor>().timerPercent;
    }

    // ------------------------------------------------

    float EnvelopeInterface::operator()() {
        return self<MiniSynthFMProcessor>().voices.lastTriggered().envelope[settings.index].output;
    }

    // ------------------------------------------------

    float LfoInterface::operator()() {
        return self<MiniSynthFMProcessor>().voices.lastTriggered().lfo[settings.index].output * 0.5 + 0.5;
    }

    // ------------------------------------------------

    void ModInterface::operator()(ModSource source, ModDestination destination, bool val) {
        self<MiniSynthFMProcessor>().params.routing[(int)source][(int)destination] = val;
    }

    // ------------------------------------------------

    bool PianoInterface::pressed(Note note) {
        for (auto& voice : self<MiniSynthFMProcessor>().voices) {
            if (voice.pressed && voice.note == note) return true;
        }

        return false;
    }

    void PianoInterface::noteOn(Note note) {
        addAsyncTask([this, note] {
            self<MiniSynthFMProcessor>().noteOn(note, 1, 0);
        });
    }

    void PianoInterface::noteOff(Note note) {
        addAsyncTask([this, note] {
            self<MiniSynthFMProcessor>().noteOff(note, 1, 0);
        });
    }

    // ------------------------------------------------

}

// ------------------------------------------------
