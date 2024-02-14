#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Processing/Interface.hpp"

// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Controller.hpp"
#include "Kaixo/MiniSynthFM/Gui/Piano.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------
    
    class SampleRateInterface : public Interface {
    public:

        // ------------------------------------------------

        float sampleRate();

        // ------------------------------------------------

    };
    
    // ------------------------------------------------
    
    class TimerInterface : public Interface {
    public:

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

        void noteOn(Note note) override;
        void noteOff(Note note) override;

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
