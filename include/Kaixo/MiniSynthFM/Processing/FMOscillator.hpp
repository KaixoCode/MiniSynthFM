#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Processing/Module.hpp"

// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Controller.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    enum class Waveform { Sine, Triangle, Saw, Square, Amount };

    // ------------------------------------------------
    
    struct FMOscillatorParameters : public Module {

        // ------------------------------------------------

        void volume(float v) { m_Volume = v; }
        void tune(Note t) { m_Tune = t; }

        void waveform(Waveform wf) { m_Waveform = wf; }
        void waveform(float val) { m_Waveform = normalToIndex(val, Waveform::Amount); }

        // ------------------------------------------------
    private:
        float m_Volume;
        Note m_Tune;
        Waveform m_Waveform;

        // ------------------------------------------------
        
        friend class FMOscillator;

        // ------------------------------------------------

    };

    // ------------------------------------------------

    class FMOscillator : public Module {
    public:

        // ------------------------------------------------
        
        FMOscillatorParameters& params;
        
        // ------------------------------------------------
        
        FMOscillator(FMOscillatorParameters& p);

        // ------------------------------------------------

        float output{};

        // ------------------------------------------------

        void trigger();

        // ------------------------------------------------

        void note(float note) { m_Note = note; }
        void fm(float phase) { m_PhaseModulation += phase; }

        // ------------------------------------------------

        void process() override;

        // ------------------------------------------------

    private:
        float m_Phase = 0;
        float m_PhaseModulation = 0;
        float m_Frequency = 440;

        // ------------------------------------------------

        Note m_Note = 0;

        // ------------------------------------------------

        void updateFrequency();

        // ------------------------------------------------

        float at(float p);

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------
