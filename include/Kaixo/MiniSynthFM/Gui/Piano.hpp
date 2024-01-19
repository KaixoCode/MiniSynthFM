#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Gui/View.hpp"
#include "Kaixo/Core/Theme/Theme.hpp"
#include "Kaixo/Core/Processing/Interface.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    class PianoInterface : public Processing::Interface {
    public:

        // ------------------------------------------------

        virtual void noteOn(Note note) = 0;
        virtual void noteOff(Note note) = 0;

        // ------------------------------------------------

        virtual bool pressed(Note note) = 0;

        // ------------------------------------------------

    };

    // ------------------------------------------------

    class Piano : public View {
    public:

        // ------------------------------------------------

        struct Settings {

            // ------------------------------------------------

            Note start = 1; // Starting note
            std::size_t notes = 0;

            // ------------------------------------------------

            Processing::InterfaceStorage<PianoInterface> interface;

            // ------------------------------------------------

            struct Key {
                Point<> size = { 30, 100 };
                Theme::DrawableElement& graphics;
            };

            // ------------------------------------------------

            Key white;
            Key black;

            // ------------------------------------------------

            Coord spacing = 5;

            // ------------------------------------------------

        } settings;

        // ------------------------------------------------

        class Key : public View {
        public:

            // ------------------------------------------------

            enum class Type { White, Black };

            // ------------------------------------------------

            struct Settings {

                // ------------------------------------------------

                Piano& piano;
                Type type;

                // ------------------------------------------------

                Theme::Drawable graphics;

                // ------------------------------------------------

                Note note;

                // ------------------------------------------------

            } settings;

            // ------------------------------------------------

            Key(Context c, Settings s);

            // ------------------------------------------------

            void paint(juce::Graphics& g) override;

            // ------------------------------------------------

            void mouseDown(const juce::MouseEvent& event) override;
            void mouseUp(const juce::MouseEvent& event) override;

            // ------------------------------------------------

            void onIdle() override;

            // ------------------------------------------------

        };

        // ------------------------------------------------

        Piano(Context c, Settings s);

        // ------------------------------------------------

    private:

        // ------------------------------------------------

        Note inOctave(Note note) const;
        bool isBlack(Note note) const;
        bool isWhite(int note) const;

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------
