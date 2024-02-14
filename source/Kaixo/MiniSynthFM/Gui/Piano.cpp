
// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Gui/Piano.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------
    
    std::string noteToName(Note fnote) {
        constexpr std::string_view noteNames[12]{
            "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
        };

        int note = static_cast<int>(Math::floor(fnote));
        int noteInOctave = note % 12;
        int octave = note / 12;

        return std::string(noteNames[noteInOctave]) + std::to_string(octave);
    }

    // ------------------------------------------------

    Piano::Key::Key(Context c, Settings s)
        : View(c), settings(std::move(s)), m_Name(noteToName(s.note))
    {
        animation(settings.graphics);
        wantsIdle(true);
    }

    // ------------------------------------------------

    void Piano::Key::paint(juce::Graphics& g) {
        settings.graphics.draw({
            .graphics = g,
            .bounds = localDimensions(),
            .text = m_Name,
            .state = state(),
        });
    }

    // ------------------------------------------------

    void Piano::Key::mouseDown(const juce::MouseEvent& event) {
        settings.piano.settings.interface->noteOn(settings.note);
    }
    
    void Piano::Key::mouseDrag(const juce::MouseEvent& event) {
        float delta = localDimensions().centerX() - event.x;
        float offset = Math::sign(delta) * Math::max(Math::abs(delta) / (width() + settings.piano.settings.spacing) - 0.5, 0.f) / 12.;
        float note = 0.5 - offset;
        context.performEdit(Synth.pitchBendParameter, note);
    }

    void Piano::Key::mouseUp(const juce::MouseEvent& event) {
        context.performEdit(Synth.pitchBendParameter, 0.5);
        settings.piano.settings.interface->noteOff(settings.note);
    }

    // ------------------------------------------------

    void Piano::Key::onIdle() {
        View::onIdle();
        bool _pressed = settings.piano.settings.interface->pressed(settings.note);
        if (pressed() != _pressed) {
            pressed(_pressed);
            repaint();
        }
    }

    // ------------------------------------------------

    Piano::Piano(Context c, Settings s)
        : View(c), settings(std::move(s))
    {
        Coord x = 0;
        for (std::size_t i = 0; i < settings.notes; ++i) {
            Note note = settings.start + i;

            if (isWhite(note)) {
                add<Key>({ x, 0, settings.white.size.x(), settings.white.size.y() }, {
                    .piano = *this,
                    .type = Key::Type::White,
                    .graphics = settings.white.graphics,
                    .note = note
                    });

                x += settings.white.size.x() + settings.spacing;
            }
        }
        x = 0;
        for (std::size_t i = 0; i < settings.notes; ++i) {
            Note note = settings.start + i;

            if (isBlack(note)) {
                Coord offset = settings.spacing / 2 + settings.black.size.x() / 2;
                add<Key>({ x - offset, 0, settings.black.size.x(), settings.black.size.y() }, {
                    .piano = *this,
                    .type = Key::Type::Black,
                    .graphics = settings.black.graphics,
                    .note = note
                    });
            } else {
                x += settings.white.size.x() + settings.spacing;
            }
        }
    }

    // ------------------------------------------------

    Note Piano::inOctave(Note note) const { return Math::Fast::fmod(note, 12); }

    bool Piano::isBlack(Note note) const {
        Note x = inOctave(note);
        return int((x < 5 ? -x : x) + (x > 4)) % 2;
    }

    bool Piano::isWhite(int note) const { return !isBlack(note); }

    // ------------------------------------------------

}

// ------------------------------------------------
