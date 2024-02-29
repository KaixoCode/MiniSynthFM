
// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Processing/Processor.hpp"

// ------------------------------------------------

#include "Kaixo/Utils/Timer.hpp"

// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Processing/Interfaces.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------
    
    Arpeggiator::Arpeggiator(VoiceBank<VoiceBankVoice, Voices>& bank)
        : m_Bank(bank)
    {}

    // ------------------------------------------------

    void Arpeggiator::process() {
        if (m_NotesDown.empty()) return;

        std::size_t samples = samplesBetweenNotes();
        std::size_t gate = m_GatePercent * samples;

        if (m_Timestamp % samples == 0) {
            triggerNote(m_NoteTrigger);
            m_NoteTrigger = (m_NoteTrigger + 1) % m_NotesDown.size();
        }

        while (m_NotesDown[m_NoteRelease].lastTrigger + gate >= m_Timestamp) {
            bool removed = releaseNote(m_NoteRelease);
            // If note removed, don't increment index, and also double-check trigger
            if (removed) {
                m_NoteTrigger = m_NoteTrigger % m_NotesDown.size();
                m_NoteRelease = m_NoteRelease % m_NotesDown.size();
            } else m_NoteRelease = (m_NoteRelease + 1) % m_NotesDown.size();
        }

        m_Timestamp += 1;
    }

    // ------------------------------------------------
    
    void Arpeggiator::noteOn(Note note, double velocity, int channel) {
        if (m_NotesDown.empty()) { // First note, reset timestamp
            m_Timestamp = 0;
        }
        // Note was already pressed, but inactive, activate again
        for (auto& pressed : m_NotesDown) {
            if (pressed.note == note) {
                pressed.active = true;
                return;
            }
        }
        // Note cache is full, remove oldest
        if (m_NotesDown.full()) {
            m_NotesDown.pop_front();
        }
        // Add pressed note to cache
        m_NotesDown.push_back({
            .velocity = velocity,
            .note = note,
            .channel = channel,
        });
    }

    void Arpeggiator::noteOff(Note note, double velocity, int channel) {
        for (auto& pressed : m_NotesDown) {
            if (pressed.note == note) {
                pressed.active = false;
                return;
            }
        }

    }

    // ------------------------------------------------
    
    void Arpeggiator::triggerNote(std::size_t noteIndex) {
        auto& note = m_NotesDown[noteIndex];
        note.lastTrigger = m_Timestamp;
        m_Bank.noteOn(note.note, note.velocity, note.channel);
    }
    
    bool Arpeggiator::releaseNote(std::size_t noteIndex) {
        auto& note = m_NotesDown[noteIndex];
        m_Bank.noteOff(note.note, note.velocity, note.channel);
    
        if (!note.active) {
            m_NotesDown.erase_index(noteIndex);
            return true;
        }

        return false;
    }

    // ------------------------------------------------

    std::size_t Arpeggiator::samplesBetweenNotes() const {
        return 100; // TODO:
    }


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

        params.quality = offline() ? m_ExportQuality : m_LiveQuality;

        for (std::size_t i = 0; i < outputBuffer().size(); ++i) {
            parameters.process();

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
