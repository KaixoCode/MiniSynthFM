
// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Gui/Piano.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------
    
    int noteToOctave(Note fnote) {
        int note = static_cast<int>(Math::floor(fnote));
        return (note - 24) / 12;
    }
    
    int noteInOctave(Note fnote) {
        int note = static_cast<int>(Math::floor(fnote));
        return note % 12;
    }

    constexpr std::string_view noteNames[12]{
        "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
    };

    std::string fullNoteName(Note fnote) {
        int note = noteInOctave(fnote);
        int octave = noteToOctave(fnote);
        return std::string(noteNames[note]) + std::to_string(octave);
    }
    
    std::string noteName(Note fnote) {
        int note = noteInOctave(fnote);
        return std::string(noteNames[note]);
    }

    // ------------------------------------------------

    Piano::Key::Key(Context c, Settings s)
        : View(c), settings(std::move(s)), 
        m_FullName(fullNoteName(settings.note)),
        m_NoteName(noteName(settings.note)),
        m_Octave(std::to_string(noteToOctave(settings.note)))
    {
        animation(settings.graphics);
        wantsIdle(true);
    }

    // ------------------------------------------------

    void Piano::Key::paint(juce::Graphics& g) {
        settings.graphics.draw({
            .graphics = g,
            .bounds = localDimensions(),
            .text = { 
                { "$note", m_NoteName }, 
                { "$octave", m_Octave },
                { "$full-note", m_FullName },
            },
            .state = state(),
        });
    }

    // ------------------------------------------------

    void Piano::Key::mouseDown(const juce::MouseEvent& event) {
        float velocity = Math::clamp1(0.1 + 0.9 * static_cast<float>(event.y - y()) / height());
        m_NoteID = settings.piano.settings.interface->noteOn(settings.note, velocity);
    }
    
    void Piano::Key::mouseDrag(const juce::MouseEvent& event) {
        float delta = localDimensions().centerX() - event.x;
        float offset = Math::sign(delta) * Math::max(Math::abs(delta) / (width() + settings.piano.settings.spacing) - 0.5, 0.f) / 48.;
        float note = 0.5 - offset;
        settings.piano.settings.interface->pitchBend(m_NoteID, note);
    }

    void Piano::Key::mouseUp(const juce::MouseEvent& event) {
        float velocity = Math::clamp1(0.1 + 0.9 * static_cast<float>(event.y - y()) / height());
        settings.piano.settings.interface->pitchBend(m_NoteID, 0.5);
        settings.piano.settings.interface->noteOff(settings.note, velocity);
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
