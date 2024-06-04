#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Processing/Interface.hpp"

// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Controller.hpp"
#include "Kaixo/MiniSynthFM/Gui/Piano.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {
    
    // ------------------------------------------------
    
    class GeneralInfoInterface : public Interface {
    public:

        // ------------------------------------------------
        
        std::size_t activeVoices();

        // ------------------------------------------------

        float sampleRate();

        // ------------------------------------------------

        float nanosPerSample();
        float percent();

        // ------------------------------------------------

    };

    // ------------------------------------------------

    class EnvelopeInterface : public TypedInterface<float()> {
    public:

        // ------------------------------------------------

        struct Settings {
            std::size_t index;
        } settings;

        // ------------------------------------------------

        float operator()() override;

        // ------------------------------------------------

    };

    // ------------------------------------------------

    class LfoInterface : public TypedInterface<float()> {
    public:

        // ------------------------------------------------

        struct Settings {
            std::size_t index;
        } settings;

        // ------------------------------------------------

        float operator()() override;

        // ------------------------------------------------

    };

    // ------------------------------------------------

    class ModInterface : public TypedInterface<void(ModSource, ModDestination, bool)> {
    public:

        // ------------------------------------------------

        void operator()(ModSource source, ModDestination destination, bool val) override;

        // ------------------------------------------------

    };

    // ------------------------------------------------

    class PianoInterface : public Gui::PianoInterface {
    public:

        // ------------------------------------------------

        bool pressed(Note note) override;

        // ------------------------------------------------

        NoteID noteOn(Note note, float velocity) override;
        void noteOff(Note note, float velocity) override;
        void pitchBend(NoteID id, float amt) override;

        // ------------------------------------------------

    };

    // ------------------------------------------------
    
    class VelocityInterface : public TypedInterface<float()> {
    public:

        // ------------------------------------------------

        float operator()() override;

        // ------------------------------------------------

    };

    // ------------------------------------------------
    
    class ModWheelInterface : public TypedInterface<float()> {
    public:

        // ------------------------------------------------

        float operator()() override;

        // ------------------------------------------------

    };
    
    // ------------------------------------------------
    
    class RandomInterface : public TypedInterface<float()> {
    public:

        // ------------------------------------------------

        float operator()() override;

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------
