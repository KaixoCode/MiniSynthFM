
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
        registerInterface<GeneralInfoInterface>();
        registerInterface<ModWheelInterface>();
        registerInterface<VelocityInterface>();
        registerInterface<RandomInterface>();
    }

    // ------------------------------------------------

    void MiniSynthFMProcessor::process() {
        Timer timer{};

        if constexpr (versionType == VersionType::Demo) {
            if (offline()) return; // Do not allow export in demo mode
            params.quality = Quality::Low;
        } else {
            params.quality = offline() ? m_ExportQuality : m_LiveQuality;
        }

        for (std::size_t i = 0; i < outputBuffer().size(); ++i) {
            parameters.process();
            bank.updateLastNote();

            for (auto& osc : params.oscillator)
                osc.updateFrequency();

            if (voice.ModuleContainer::active()) {
                simd_path::execute([this]<class SimdType>() {
                    voice.process<SimdType>();
                });
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

        // Since the filter can become unstable when modulated at high speeds
        // this makes sure when an invalid state occurs, it gets reset back to a valid state.
        if (voice.ModuleContainer::active()) {
            constexpr float maxPossibleLevel = 20; // Level probably can't get above ~3, but some room for error
            bool hasInvalidState = std::isnan(delay.output.l) 
                || delay.output.l >  maxPossibleLevel
                || delay.output.l < -maxPossibleLevel;

            if (hasInvalidState) {
                reset();
                for (auto& frame : outputBuffer()) frame = { 0, 0 };
            }
        }

        double nanos = timer.time<std::chrono::nanoseconds>();
        double nanosUsedPerSample = nanos / outputBuffer().size();
        double availableNanosPerSample = 1e9 / sampleRate();
        double percentUsed = 100 * nanosUsedPerSample / availableNanosPerSample;

        timerPercentMax = timerPercentMax * 0.99 + 0.01 * percentUsed;
        timerNanosPerSampleMax = timerNanosPerSampleMax * 0.99 + 0.01 * nanosUsedPerSample;

        auto now = std::chrono::steady_clock::now();
        if (now - lastMeasure >= std::chrono::milliseconds(250)) {
            lastMeasure = now;
            timerPercent = timerPercentMax;
            timerNanosPerSample = timerNanosPerSampleMax;
        }
    }

    // ------------------------------------------------

    void MiniSynthFMProcessor::noteOnMPE(NoteID id, Note note, double velocity, int channel) {
        bank.noteOnMPE(id, note, velocity, channel);
        bank.notePitchBendMPE(id, 0.5);
    }

    void MiniSynthFMProcessor::notePitchBendMPE(NoteID id, double value) {
        bank.notePitchBendMPE(id, value);
    }

    void MiniSynthFMProcessor::noteOffMPE(NoteID id, Note note, double velocity, int channel) {
        bank.noteOffMPE(id, note, velocity, channel);
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
