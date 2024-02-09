
// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Processing/Processor.hpp"

// ------------------------------------------------

#include "Kaixo/Utils/Timer.hpp"

// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Processing/Interfaces.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    MiniSynthFMProcessor::MiniSynthFMProcessor() {
        registerModule(parameters);
        registerModule(params);
        registerModule(bank);
        registerModule(voice);
        registerModule(delay);

        registerInterface<EnvelopeInterface>();
        registerInterface<LfoInterface>();
        registerInterface<ModInterface>();
        registerInterface<PianoInterface>();
        registerInterface<TimerInterface>();
        registerInterface<ModWheelInterface>();
        registerInterface<VelocityInterface>();
        registerInterface<RandomInterface>();
    }

    // ------------------------------------------------

    void MiniSynthFMProcessor::process() {
        Timer timer{};

        params.quality = offline() ? m_ExportQuality : m_LiveQuality;

        for (std::size_t i = 0; i < outputBuffer().size(); ++i) {
            parameters.process();

            for (auto& osc : params.oscillator)
                osc.updateFrequency();
            
            if (voice.ModuleContainer::active()) {
                switch (simd_path::path) {
                case simd_path::s512: 
                case simd_path::s256: voice.process<simd<float, 256>>(); break;
                case simd_path::s128: voice.process<simd<float, 128>>(); break;
                case simd_path::s0:   voice.process<float>(); break;
                }
                delay.input = voice.output;
            } else {
                delay.input = 0;
            }

            if (delay.active()) {
                delay.process();
                outputBuffer()[i] = delay.output;
            } else {
                outputBuffer()[i] = { 0, 0 };
            }
        }

        double nanos = timer.time<std::chrono::nanoseconds>();
        double nanosUsedPerSample = nanos / outputBuffer().size();
        double availableNanosPerSample = 1e9 / sampleRate();
        double percentUsed = 100 * nanosUsedPerSample / availableNanosPerSample;
        timerPercent = 0.99 * timerPercent + 0.01 * percentUsed;
    }

    // ------------------------------------------------

    void MiniSynthFMProcessor::noteOn(Note note, double velocity, int channel) {
        bank.noteOn(note, velocity, channel);
    }

    void MiniSynthFMProcessor::noteOff(Note note, double velocity, int channel) {
        bank.noteOff(note, velocity, channel);
    }

    // ------------------------------------------------

    void MiniSynthFMProcessor::quality(float val) {
        quality(normalToIndex(val, Quality::Amount));
    }

    void MiniSynthFMProcessor::quality(Quality val) {
        m_LiveQuality = val;
    }


    void MiniSynthFMProcessor::exportQuality(float val) {
        exportQuality(normalToIndex(val, Quality::Amount));
    }

    void MiniSynthFMProcessor::exportQuality(Quality val) {
        m_ExportQuality = val;
    }

    void MiniSynthFMProcessor::phaseMode(float val) {
        phaseMode(normalToIndex(val, PhaseMode::Amount));
    }

    void MiniSynthFMProcessor::phaseMode(PhaseMode val) {
        for (auto& osc : params.oscillator) {
            osc.phaseMode = val;
        }
    }

    // ------------------------------------------------

    void MiniSynthFMProcessor::init() {
        params.resetRouting();
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
            lambda(ModSource::ModWheel, dst);
            lambda(ModSource::Random, dst);
            lambda(ModSource::Velocity, dst);
        };

        doAllS(ModDestination::LfoDepth);
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
            json[toString(src)][toString(dst)] = (bool)params.routing[(int)dst][(int)src];
        });

        return json;
    }

    void MiniSynthFMProcessor::deserialize(basic_json& data) {
        params.resetRouting();

        forAllModulation([&](ModSource src, ModDestination dst) {
            auto sname = toString(src);
            auto dname = toString(dst);
            if (data.contains(sname, basic_json::Object) &&
                data[sname].contains(dname, basic_json::Boolean))
            {
                params.routing[(int)dst][(int)src] = data[sname][dname].as<bool>();
            }
        });
    }

    // ------------------------------------------------

    Processor* createProcessor() { return new MiniSynthFMProcessor(); }

    // ------------------------------------------------

}
    
// ------------------------------------------------
