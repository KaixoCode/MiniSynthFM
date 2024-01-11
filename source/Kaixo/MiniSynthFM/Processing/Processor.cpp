
// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Processing/Processor.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    MiniSynthFMProcessor::MiniSynthFMProcessor() {
        registerModule(parameters);
        registerModule(params);
        registerModule(voices);
    }

    // ------------------------------------------------

    void MiniSynthFMProcessor::process() {
        voices.process();
    }

    // ------------------------------------------------

    void MiniSynthFMProcessor::noteOn(Note note, double velocity, int channel) {
        voices.noteOn(note, velocity, channel);
    }

    void MiniSynthFMProcessor::noteOff(Note note, double velocity, int channel) {
        voices.noteOff(note, velocity, channel);
    }

    // ------------------------------------------------

    Processor* createProcessor() { return new MiniSynthFMProcessor(); }

    // ------------------------------------------------

}
    
// ------------------------------------------------
