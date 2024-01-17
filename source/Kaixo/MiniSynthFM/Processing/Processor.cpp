
// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Processing/Processor.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    MiniSynthFMProcessor::MiniSynthFMProcessor() {
        registerModule(parameters);
        registerModule(params);
        registerModule(voices);

        registerInterface<EnvelopeInterface>();
        registerInterface<LfoInterface>();
        registerInterface<ModInterface>();
        registerInterface<PianoInterfaceImpl>();
        registerInterface<PianoPressInterface>();
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

    void MiniSynthFMProcessor::init() {
        std::memset(params.routing, 0, sizeof(params.routing));
    }

    constexpr void forAllModulation(auto lambda) {
        auto doAllS = [&](ModDestination dst) {
            lambda(ModSource::Envelope1, dst);
            lambda(ModSource::Envelope2, dst);
            lambda(ModSource::Envelope3, dst);
            lambda(ModSource::Op1, dst);
            lambda(ModSource::Op2, dst);
            lambda(ModSource::Op3, dst);
            lambda(ModSource::LFO, dst);
        };

        doAllS(ModDestination::FilterFreq);
        doAllS(ModDestination::Op1Amount);
        doAllS(ModDestination::Op2Amount);
        doAllS(ModDestination::Op3Amount);
        doAllS(ModDestination::Op1FM);
        doAllS(ModDestination::Op2FM);
        doAllS(ModDestination::Op3FM);
        doAllS(ModDestination::Op1Sync);
        doAllS(ModDestination::Op2Sync);
        doAllS(ModDestination::Op3Sync);
    }

    basic_json MiniSynthFMProcessor::serialize() {
        basic_json json;

        forAllModulation([&](ModSource src, ModDestination dst) {
            json[toString(src)][toString(dst)] = params.routing[(int)src][(int)dst];
        });

        return json;
    }

    void MiniSynthFMProcessor::deserialize(basic_json& data) {
        forAllModulation([&](ModSource src, ModDestination dst) {
            auto sname = toString(src);
            auto dname = toString(dst);
            if (data.contains(sname, basic_json::Array) &&
                data[sname].contains(dname, basic_json::Boolean))
            {
                params.routing[(int)src][(int)dst] = data[sname][dname].as<bool>();
            }
        });
    }

    // ------------------------------------------------

    Processor* createProcessor() { return new MiniSynthFMProcessor(); }

    // ------------------------------------------------

}
    
// ------------------------------------------------
