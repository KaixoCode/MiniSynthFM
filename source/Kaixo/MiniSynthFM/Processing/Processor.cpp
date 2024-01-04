
// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Processing/Processor.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    MiniSynthFMProcessor::MiniSynthFMProcessor() {
        registerModule(parameters);
    }

    // ------------------------------------------------

    void MiniSynthFMProcessor::process() {
        for (std::size_t i = 0; i < inputBuffer().size(); ++i) {
            parameters.process();

            // outputBuffer()[i] = ...;
        }
    }

    // ------------------------------------------------

    Processor* createProcessor() { return new MiniSynthFMProcessor(); }

    // ------------------------------------------------

}
    
// ------------------------------------------------
