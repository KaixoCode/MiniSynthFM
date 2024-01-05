
// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Processing/Processor.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    MiniSynthFMProcessor::MiniSynthFMProcessor() {
        registerModule(parameters);
        registerModule(voices[0]);
    }

    // ------------------------------------------------

    void MiniSynthFMProcessor::process() {
        for (std::size_t i = 0; i < inputBuffer().size(); ++i) {
            parameters.process();

            voices[0].process();

            outputBuffer()[i] = voices[0].output;
        }
    }

    // ------------------------------------------------

    Processor* createProcessor() { return new MiniSynthFMProcessor(); }

    // ------------------------------------------------

}
    
// ------------------------------------------------
