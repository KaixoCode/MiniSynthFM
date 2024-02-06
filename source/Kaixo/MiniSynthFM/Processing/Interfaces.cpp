
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
