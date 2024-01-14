#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Processing/Module.hpp"
#include "Kaixo/Core/Processing/Random.hpp"

// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Controller.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    enum class Waveform { Sine, Triangle, Saw, Square, Amount };

    // ------------------------------------------------
    
    struct FMOscillatorParameters : public Module {

        // ------------------------------------------------

        void tune(Note t) { m_Tune = t; }
        void octave(int o) { m_Octave = o; }

        void waveform(Waveform wf) { m_Waveform = wf; }
        void waveform(float val) { m_Waveform = normalToIndex(val, Waveform::Amount); }

        // ------------------------------------------------
    private:
        Note m_Tune = 0;
        int m_Octave = 0;
        Waveform m_Waveform = Waveform::Sine;

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
        float fmOutput{};

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
        
        Random m_Random;
        float m_RandomTune = 0;
        std::size_t m_Counter = 0;

        // ------------------------------------------------

        void updateFrequency();

        // ------------------------------------------------

        float at(float p);
        float fmAt(float p);

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------
