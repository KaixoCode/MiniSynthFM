# MiniSynthFM

MiniSynthFM is an intuitive and efficient FM Synthesizer, crafted for musicians and producers who prioritize a fast, creative workflow. 
Despite its simplicity, MiniSynthFM packs a robust set of features that allow for versatile FM sound design without overwhelming complexity.

For more information visit https://minisynthfm.com

![image](https://minisynthfm.com/assets/minisynthfm.png)

# How to build this project?
Make sure you clone recursively to get all sub-modules. And then it's just a simple CMake project. 
It may fail on the first build because JUCE generates a file called 'JuceHeader.h', and this is only generated when you build the project. 
So you may need to rerun cmake after you've built the code once.

The most important projects in this cmake project are:
- MiniSynthFM: All the source code for the synth itself. This includes all code from the SynthCore repository.
- MiniSynthFM_Standalone: JUCE-generated project that builds a standalone executable.
- MiniSynthFM_VST3: JUCE-generated project that builds the VST3 version of the synth.
