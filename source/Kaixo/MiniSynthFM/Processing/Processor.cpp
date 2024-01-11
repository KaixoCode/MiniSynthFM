
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
        for (std::size_t i = 0; i < outputBuffer().size(); ++i) {
            parameters.process();
            for (auto& voice : voices) {
                voice.process();
                outputBuffer()[i] += voice.result;
            }
        }
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
