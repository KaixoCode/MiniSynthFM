
// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Gui/PatchBay.hpp"

// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Gui/Catenary.hpp"
#include "Kaixo/MiniSynthFM/Processing/Processor.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    PatchBay::Jack::Jack(Context c, Settings s)
        : View(c), settings(std::move(s))
    {
        m_Id = settings.patchBay.addJack(*this);
    }

    // ------------------------------------------------

    void PatchBay::Jack::paint(juce::Graphics& g) {
        settings.graphics.draw({
            .graphics = g,
            .bounds = localDimensions(),
            .text = settings.name
        });
    }

    // ------------------------------------------------

    Point<> PatchBay::Jack::holePosition() const {
        return position() + Point{ 32, 15 };
    }

    // ------------------------------------------------

    void PatchBay::Jack::mouseDrag(const juce::MouseEvent& event) {
        auto e = event.getEventRelativeTo(&settings.patchBay);
        settings.patchBay.moveCable({ e.x, e.y });
    }

    void PatchBay::Jack::mouseDown(const juce::MouseEvent& event) {
        if (event.mods.isLeftButtonDown()) {
            settings.patchBay.beginCable(m_Id);
        }

        if (event.mods.isRightButtonDown()) {
            settings.patchBay.removeCable(m_Id);
        }

        auto e = event.getEventRelativeTo(&settings.patchBay);
        settings.patchBay.moveCable({ e.x, e.y });
    }

    void PatchBay::Jack::mouseUp(const juce::MouseEvent& event) {
        settings.patchBay.finishCable();
    }

    // ------------------------------------------------

    bool PatchBay::Connection::operator==(const Connection& other) const {
        return other.begin == begin && other.end == end ||
               other.begin == end && other.end == begin;
    }

    // ------------------------------------------------

    void PatchBay::Connection::reset() {
        begin = npos;
        end = npos;

        for (auto& segment : m_Segments) {
            segment.y = 0;
            segment.vy = 0;
        }

        m_Color = (m_Color + 1) % 5;
    }

    // ------------------------------------------------

    void PatchBay::Connection::paint(juce::Graphics& g, Point<> mouse, PatchBay& self) {
        if (begin == npos && end == npos) return; // Nothing to draw

        Point<float> a = self.m_Jacks[begin]->holePosition();
        Point<float> b = mouse;

        if (end != npos) {
            b = self.m_Jacks[end]->holePosition();
        }

        if (a.x() > b.x()) std::swap(a, b);
        if (a.x() == b.x()) a.x(b.x() - 1); // make sure x is never the same

        float currentDistance = a.getDistanceFrom(b);
        float deltaDistance = currentDistance - m_Distance;
        m_DeltaDistance = deltaDistance * 0.05 + m_DeltaDistance * 0.95;
        m_Distance = currentDistance;
        float extra = Math::clamp(100 - m_DeltaDistance * 5, 10, 200);

        Catenary catenary{ a.x(), self.height() - a.y(), b.x(), self.height() - b.y(), extra, 5 };

        juce::Path path;
        path.startNewSubPath(a);

        bool first = true;
        float dx = (b.x() - a.x()) / (Segments + 1);
        for (std::size_t i = 0; i < Segments; ++i) {
            auto& segment = m_Segments[i];
            float x = a.x() + dx * (i + 1);
            float y = self.height() - catenary.calcY(x);

            float half = Segments / 2.f;
            float damp = 0.6 + 0.3 * (1 - ((i - half) * (i - half)) / (half * half));

            segment.vy += (y - segment.y) * 0.15;
            segment.vy *= damp;
            segment.vy = std::clamp(segment.vy, -50.f, 50.f);
            segment.y += segment.vy;
            segment.y = std::clamp(segment.y, 0.f, (float)self.height());

            path.lineTo({ x, segment.y });
        }

        path.lineTo(b);

        self.m_CableGraphics[m_Color].end.draw({
            .graphics = g,
            .bounds = { a.x() - 13, a.y() - 13, 26, 26 },
        });

        self.m_CableGraphics[m_Color].end.draw({
            .graphics = g,
            .bounds = { b.x() - 13, b.y() - 13, 26, 26 },
        });

        g.setColour(self.m_CableGraphics[m_Color].color);
        g.strokePath(path, PathStrokeType{ 8.f, PathStrokeType::curved, PathStrokeType::rounded });
    }

    // ------------------------------------------------

    PatchBay::PatchBay(Context c)
        : View(c)
    {   // Patch bay is just functionality and visuals, 
        // no UI interactions. That happens through the Jacks.
        setInterceptsMouseClicks(false, false);
        wantsIdle(true);

        m_CableGraphics[0] = { T.cable1.end, T.cable1.color };
        m_CableGraphics[1] = { T.cable2.end, T.cable2.color };
        m_CableGraphics[2] = { T.cable3.end, T.cable3.color };
        m_CableGraphics[3] = { T.cable4.end, T.cable4.color };
        m_CableGraphics[4] = { T.cable5.end, T.cable5.color };
    }

    // ------------------------------------------------

    void PatchBay::onIdle() {
        repaint();
    }

    // ------------------------------------------------

    void PatchBay::paint(juce::Graphics& g) {
        for (auto& connection : m_Connections) {
            connection.paint(g, m_LastMousePosition, *this);
        }
        m_CurrentConnection.paint(g, m_LastMousePosition, *this);
    }

    // ------------------------------------------------

    void PatchBay::beginCable(JackId id) {
        m_CurrentConnection.begin = id;
        m_CurrentConnection.end = npos;
    }

    void PatchBay::removeCable(JackId id) {
        for (std::size_t i = m_Connections.size(); i > 0; --i) {
            auto& connection = m_Connections[i - 1];
            bool isBegin = connection.begin == id;
            bool isEnd = connection.end == id;
            if (isBegin || isEnd) {
                m_CurrentConnection = connection;
                m_CurrentConnection.begin = isBegin ? connection.end : connection.begin;
                m_CurrentConnection.end = npos;
                removeConnection(connection);
                m_Connections.erase(m_Connections.begin() + (i - 1));
                break;
            }
        }
    }

    void PatchBay::moveCable(Point<> to) {
        m_LastMousePosition = to;
    }

    bool PatchBay::finishCable() {
        if (m_CurrentConnection.begin == npos) return false;
        // Find which jack hovering over
        for (JackId id = 0; id < m_Jacks.size(); ++id) {
            auto& jack = m_Jacks[id];
            auto& begin = m_Jacks[m_CurrentConnection.begin];
            if (id != m_CurrentConnection.begin && // Not same jack
                jack->input() != begin->input() && // Not both same type
                jack->output() != begin->output() &&
                jack->dimensions().contains(m_LastMousePosition)) // Hover
            {
                m_CurrentConnection.end = id;
                // Check for existing connection
                for (auto& connection : m_Connections) {
                    if (connection == m_CurrentConnection) {
                        m_CurrentConnection.reset();
                        return false; // Connection already exists, do not add it again
                    }
                }

                addCurrentConnection(); // Finalize connection
                return true;
            }
        }

        m_CurrentConnection.reset(); // No connection added, so reset
        return false;
    }

    // ------------------------------------------------

    void PatchBay::addCurrentConnection() {
        m_Connections.push_back(m_CurrentConnection);
        addConnection(m_CurrentConnection);
        m_CurrentConnection.reset();
    }

    void PatchBay::addConnection(Connection& con) {
        modifyConnection(con, true);
    }

    void PatchBay::removeConnection(Connection& con) {
        modifyConnection(con, false);
    }

    void PatchBay::modifyConnection(Connection& con, bool enable) {
        auto& a = m_Jacks[con.begin];
        auto& b = m_Jacks[con.end];

        ModSource source = a->output() ? a->settings.source : b->settings.source;
        ModDestination destination = a->input() ? a->settings.destination : b->settings.destination;

        if (source == ModSource::None || destination == ModDestination::None) return;

        context.interface<Processing::ModInterface>()->call(source, destination, enable);
    }

    // ------------------------------------------------

    PatchBay::JackId PatchBay::addJack(Jack& jack) {
        m_Jacks.push_back(&jack);
        return m_Jacks.size() - 1;
    }

    // ------------------------------------------------

}
    
// ------------------------------------------------