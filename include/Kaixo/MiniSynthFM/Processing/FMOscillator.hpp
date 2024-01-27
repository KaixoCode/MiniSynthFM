#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Processing/Module.hpp"
#include "Kaixo/Core/Processing/Random.hpp"
#include "Kaixo/Core/Processing/Filter.hpp"

// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Controller.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    enum class Waveform { Sine, Triangle, Saw, Square, Amount };

    // ------------------------------------------------
    
    struct FMOscillatorParameters : public Module {

        // ------------------------------------------------

        void tune(Note t) { m_Tune = t; m_FrequencyDirty = true; }
        void octave(int o) { m_Octave = o; m_FrequencyDirty = true; }

        void waveform(Waveform wf) { m_Waveform = wf; }
        void waveform(float val) { waveform(normalToIndex(val, Waveform::Amount)); }

        void quality(Quality val) { m_Quality = val; }
        void quality(float val) { quality(normalToIndex(val, Quality::Amount)); }

        // ------------------------------------------------
        
        float frequencyMultiplier() { return m_FrequencyMultiplier; }

        // ------------------------------------------------

        void updateFrequency() {
            if (!m_FrequencyDirty) return;
            m_FrequencyDirty = false;
            m_FrequencyMultiplier = Math::Fast::exp2(1. / 12. * m_Tune + m_Octave);
        }

        // ------------------------------------------------

    private:
        Note m_Tune = 0;
        float m_FrequencyMultiplier = 1;
        int m_Octave = 0;
        Waveform m_Waveform = Waveform::Sine;
        Quality m_Quality = Quality::Normal;
        bool m_FrequencyDirty = true;

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

        void note(float note);
        void fm(float phase);

        // ------------------------------------------------

        void process() override;

        // ------------------------------------------------
        
        void hardSync(FMOscillator& osc);
        void resetPhase();

        // ------------------------------------------------

    private:
        float m_Phase = 0;
        float m_PhaseModulation = 0;
        float m_PreviousPhaseModulation = 0;
        float m_Frequency = 440;
        float m_FrequencyOffset = 440;
        float m_NoteFrequency = 0;
        bool m_DidCycle = false;

        // ------------------------------------------------
        
        Note m_Note = 1;

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
