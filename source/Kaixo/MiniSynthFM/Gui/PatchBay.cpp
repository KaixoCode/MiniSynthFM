
// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Gui/PatchBay.hpp"

// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Gui/Catenary.hpp"
#include "Kaixo/MiniSynthFM/Processing/Interfaces.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    constexpr float cableTension = 0.075;
    constexpr float cableDamping = 0.069;
    constexpr float cableGravity = 1.25;

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
            .text = { { "$name", settings.name } }
        });
    }

    // ------------------------------------------------

    Point<> PatchBay::Jack::holePosition() const {
        return settings.patchBay.getLocalPoint(this, Point{ 32, 15 });
    }

    // ------------------------------------------------

    void PatchBay::Jack::mouseDrag(const juce::MouseEvent& event) {
        auto e = event.getEventRelativeTo(&settings.patchBay);

        if (m_Clicked && Storage::flag(Setting::TouchMode)) {
            auto diff = std::chrono::steady_clock::now() - m_ClickedAt;
            auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(diff).count();

            if (millis < 100) {
                settings.patchBay.beginCable(m_Id);
            } else {
                settings.patchBay.removeCable(m_Id);
            }
        }

        m_Clicked = false;

        settings.patchBay.moveCable({ e.x, e.y });
    }

    void PatchBay::Jack::mouseDown(const juce::MouseEvent& event) {
        m_ClickedAt = std::chrono::steady_clock::now(); 
        m_Clicked = true;

        if (!Storage::flag(Setting::TouchMode)) {
            if (event.mods.isLeftButtonDown()) {
                settings.patchBay.beginCable(m_Id);
            }

            if (event.mods.isRightButtonDown()) {
                settings.patchBay.removeCable(m_Id);
            }

            auto e = event.getEventRelativeTo(&settings.patchBay);
            settings.patchBay.moveCable({ e.x, e.y });
        }
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
            segment.position = { 0, 0 };
            segment.velocity = { 0, 0 };
        }

        m_Color = (m_Color + 1) % 5;
    }

    // ------------------------------------------------

    void PatchBay::Connection::initPosition(PatchBay& self) {
        if (begin == npos && end == npos) return;
        auto beginPos = begin == npos ? end : begin;
        auto endPos = end == npos ? begin : end;
        Point<float> a = self.m_Jacks[beginPos]->holePosition();
        Point<float> b = self.m_Jacks[endPos]->holePosition();

        for (std::size_t i = 0; i < Segments; ++i) {
            float ratio = static_cast<float>(i) / Segments;
            auto& segment = m_Segments[i];
            segment.position = a * (1 - ratio) + b * ratio;
            segment.velocity = { 0, 0 };
        }
    }

    // ------------------------------------------------

    void PatchBay::Connection::drawJacks(juce::Graphics& g, Point<float> a, Point<float> b, PatchBay& self) {
        self.m_CableGraphics[m_Color].end.draw({
            .graphics = g,
            .bounds = { a.x() - 13, a.y() - 13, 26, 26 },
        });

        self.m_CableGraphics[m_Color].end.draw({
            .graphics = g,
            .bounds = { b.x() - 13, b.y() - 13, 26, 26 },
        });
    }

    void PatchBay::Connection::drawJacks(juce::Graphics& g, Point<> mouse, PatchBay& self) {
        if (begin == npos && end == npos) return;

        Point<float> a = mouse;
        Point<float> b = mouse;

        if (begin != npos) a = self.m_Jacks[begin]->holePosition();
        if (end != npos) b = self.m_Jacks[end]->holePosition();

        drawJacks(g, a, b, self);
    }

    void PatchBay::Connection::drawCable(juce::Graphics& g, Point<float> a, Point<float> b, PatchBay& self) {

        juce::Path path;

        path.startNewSubPath(a);
        if (!Storage::flag(CablePhysics)) {
            Catenary catenary{ a.x(), self.height() - a.y(), b.x(), self.height() - b.y(), 45, 10 };
            for (std::size_t i = 1; i < Segments - 1; ++i) {
                auto r = (static_cast<float>(i) / (Segments - 1));
                auto x = (b.x() - a.x()) * r + a.x();
                auto y = catenary.calcY(x);
                path.lineTo({ x, self.height() - y});
            }
        } else {
            m_Segments.front().position = a;
            m_Segments.back().position = b;

            for (std::size_t i = 1; i < Segments - 1; ++i) {
                path.lineTo(m_Segments[i].position);
            }
            simulationStep();
        }

        path.lineTo(b);

        path.applyTransform(juce::AffineTransform::translation(0, 3));
        g.setColour(juce::Colour{ 0.f, 0.f, 0.f, 0.2f });
        g.strokePath(path, PathStrokeType{ 8.f, PathStrokeType::curved, PathStrokeType::rounded });

        path.applyTransform(juce::AffineTransform::translation(0, -3));
        g.setColour(self.m_CableGraphics[m_Color].color);
        g.strokePath(path, PathStrokeType{ 8.f, PathStrokeType::curved, PathStrokeType::rounded });
    }

    void PatchBay::Connection::drawCable(juce::Graphics& g, Point<> mouse, PatchBay& self) {
        if (begin == npos && end == npos) return;

        Point<float> a = mouse;
        Point<float> b = mouse;

        if (begin != npos) a = self.m_Jacks[begin]->holePosition();
        if (end != npos) b = self.m_Jacks[end]->holePosition();

        drawCable(g, a, b, self);
    }

    // ------------------------------------------------

    ModSource PatchBay::Connection::source(PatchBay& self) {
        if (self.m_Jacks[begin]->settings.source != ModSource::None) 
            return self.m_Jacks[begin]->settings.source;
        if (self.m_Jacks[end]->settings.source != ModSource::None) 
            return self.m_Jacks[end]->settings.source;
        return ModSource::None;
    }

    ModDestination PatchBay::Connection::destination(PatchBay& self) {
        if (self.m_Jacks[begin]->settings.destination != ModDestination::None) 
            return self.m_Jacks[begin]->settings.destination;
        if (self.m_Jacks[end]->settings.destination != ModDestination::None) 
            return self.m_Jacks[end]->settings.destination;
        return ModDestination::None;
    }

    // ------------------------------------------------

    bool PatchBay::Connection::changing() {
        // If not connected, not changing
        if (end == npos && begin == npos) return false; 
        // If it's currently being moved, it's changing
        if (begin == npos && end != npos || begin != npos && end == npos) return true;
        // Otherwise, if any segment is moving, it's changing
        for (auto& segment : m_Segments) {
            if (segment.velocity.getDistanceFromOrigin() > 0.01)
                return true;
        }
        return false;
    }

    // ------------------------------------------------

    void PatchBay::Connection::simulationStep() {
        for (std::size_t j = 0; j < 2; ++j) {
            for (std::size_t i = 1; i < Segments - 1; ++i) {
                auto& segment = m_Segments[i];

                Point<float> prevPos = m_Segments[i - 1].position;
                Point<float> curPos = m_Segments[i].position;
                Point<float> nextPos = m_Segments[i + 1].position;

                auto acceleration = cableTension * (prevPos - 2.f * curPos + nextPos);

                segment.velocity += acceleration + Point<float>{ 0, cableGravity };
                segment.velocity *= (1.0f - cableDamping); // Apply damping
            }

            for (std::size_t i = 1; i < Segments - 1; ++i) {
                auto& segment = m_Segments[i];
                segment.position += segment.velocity;
            }
        }
    }

    void PatchBay::Connection::simulationStepEndpoints() {

        Point<float> curPos1 = m_Segments.front().position;
        Point<float> nextPos1 = m_Segments[1].position;

        auto acceleration1 = (0.1f * cableTension) * (curPos1 - 2.f * curPos1 + nextPos1);

        // Endpoints just have gravity
        m_Segments.front().velocity += acceleration1 + Point<float>{ 0, cableGravity };
        m_Segments.front().velocity *= (1.0f - 0.5 * cableDamping); // Apply damping
        m_Segments.front().position += m_Segments.front().velocity;

        Point<float> prevPos2 = m_Segments[Segments - 2].position;
        Point<float> curPos2 = m_Segments.back().position;

        auto acceleration2 = (0.1f * cableTension) * (prevPos2 - 2.f * curPos2 + curPos2);

        m_Segments.back().velocity += acceleration2 + Point<float>{ 0, cableGravity };
        m_Segments.back().velocity *= (1.0f - 0.5 * cableDamping); // Apply damping
        m_Segments.back().position += m_Segments.back().velocity;
    }

    // ------------------------------------------------

    PatchBay::PatchBay(Context c)
        : View(c)
    {   // Patch bay is just functionality and visuals, 
        // no UI interactions. That happens through the Jacks.
        setInterceptsMouseClicks(false, false);
        wantsIdle(true);

        m_CableGraphics[0] = { T.cables.cable1.end, T.cables.cable1.color };
        m_CableGraphics[1] = { T.cables.cable2.end, T.cables.cable2.color };
        m_CableGraphics[2] = { T.cables.cable3.end, T.cables.cable3.color };
        m_CableGraphics[3] = { T.cables.cable4.end, T.cables.cable4.color };
        m_CableGraphics[4] = { T.cables.cable5.end, T.cables.cable5.color };
    }

    // ------------------------------------------------

    Rect<int> PatchBay::Connection::bounding(PatchBay& self, Point<> mouse) const {
        auto begin = this->begin == npos ? mouse : self.m_Jacks[this->begin]->holePosition();
        auto end = this->end == npos ? mouse : self.m_Jacks[this->end]->holePosition();
        return bounding(self, begin, end);
    }

    Rect<int> PatchBay::Connection::bounding(PatchBay& self, Point<> a, Point<> b) const {
        auto begin = a;
        auto end = b;
        auto minX = Math::min(begin.x(), end.x());
        auto maxX = Math::max(begin.x(), end.x());
        auto minY = Math::min(begin.y(), end.y());
        auto maxY = Math::max(begin.y(), end.y());

        for (auto& segment : m_Segments) {
            if (segment.position.y() < minY) minY = segment.position.y();
            if (segment.position.y() > maxY) maxY = segment.position.y();
            if (segment.position.x() < minX) minX = segment.position.x();
            if (segment.position.x() > maxX) maxX = segment.position.x();
        }

        return Rect{ minX, minY, maxX - minX, maxY - minY }.expanded(13, 13);
    }


    void PatchBay::onIdle() {
        if (changing()) {
            if (Storage::flag(CablePhysics)) {
                // Find bounding box around all moving connections, so we only repaint the smallest necessary rectangle.
                Rect<int> bounding;
                for (auto& connection : m_Connections) {
                    if (connection.changing()) {
                        bounding = bounding.getUnion(connection.bounding(*this, m_LastMousePosition));
                    }
                }

                for (auto& connection : m_FallingConnections) {
                    bounding = bounding.getUnion(connection.bounding(*this, connection.m_Segments.back().position, connection.m_Segments.front().position));
                }

                if (m_CurrentConnection.begin != npos || m_CurrentConnection.end != npos) {
                    bounding = bounding.getUnion(m_CurrentConnection.bounding(*this, m_LastMousePosition));
                }
                repaint(bounding.getUnion(m_LastBoundingBox)); // Union with previous bounding box to clear previous position
                m_LastBoundingBox = bounding;
            } else {
                repaint();
            }
        }
    }

    // ------------------------------------------------

    void PatchBay::paint(juce::Graphics& g) {
        for (auto& connection : m_Connections) {
            connection.drawJacks(g, m_LastMousePosition, *this);
        }

        for (auto& connection : m_Connections) {
            connection.drawCable(g, m_LastMousePosition, *this);
        }
        
        for (auto& connection : m_FallingConnections) {
            connection.drawJacks(g, connection.m_Segments.front().position, connection.m_Segments.back().position, *this);
        }
        
        for (auto& connection : m_FallingConnections) {
            connection.drawCable(g, connection.m_Segments.front().position, connection.m_Segments.back().position, *this);
            connection.simulationStepEndpoints();
        }

        // Remove falling connections that are outside the window
        for (auto it = m_FallingConnections.begin(); it != m_FallingConnections.end();) {
            bool oneAbove = false;
            for (auto& segment : it->m_Segments) {
                if (segment.position.y() < height() + 40) {
                    oneAbove = true;
                    break;
                }
            }

            if (!oneAbove) {
                it = m_FallingConnections.erase(it);
            } else ++it;
        }

        m_CurrentConnection.drawJacks(g, m_LastMousePosition, *this);
        m_CurrentConnection.drawCable(g, m_LastMousePosition, *this);
    }

    // ------------------------------------------------

    void PatchBay::beginCable(JackId id) {
        m_Changing = true;
        m_CurrentConnection.begin = id;
        m_CurrentConnection.end = npos;
        m_CurrentConnection.initPosition(*this);
    }

    void PatchBay::removeCable(JackId id) {
        m_Changing = true;
        for (std::size_t i = m_Connections.size(); i > 0; --i) {
            auto& connection = m_Connections[i - 1];
            bool isBegin = connection.begin == id;
            bool isEnd = connection.end == id;
            if (isBegin || isEnd) {
                m_CurrentConnection = connection;
                if (m_CurrentConnection.begin == id) m_CurrentConnection.begin = npos;
                if (m_CurrentConnection.end == id) m_CurrentConnection.end = npos;
                removeConnection(connection);
                m_Connections.erase(m_Connections.begin() + (i - 1));
                break;
            }
        }
    }

    void PatchBay::moveCable(Point<> to) {
        m_Changing = true;
        m_LastLastMousePosition = m_LastMousePosition;
        m_LastMousePosition = to;
    }

    bool PatchBay::finishCable() {
        m_Changing = true;
        if (m_CurrentConnection.begin == npos && m_CurrentConnection.end == npos) return false;
        auto mouseId = m_CurrentConnection.begin == npos ? m_CurrentConnection.end : m_CurrentConnection.begin;
        // Find which jack hovering over
        for (JackId id = 0; id < m_Jacks.size(); ++id) {
            auto& jack = m_Jacks[id];
            auto& begin = m_Jacks[mouseId];
            if (id != mouseId && // Not same jack
                jack->input() != begin->input() && // Not both same type
                jack->output() != begin->output() &&
                jack->localDimensions().contains(jack->getLocalPoint(this, m_LastMousePosition))) // Hover
            {
                if (m_CurrentConnection.end == npos) m_CurrentConnection.end = id;
                if (m_CurrentConnection.begin == npos) m_CurrentConnection.begin = id;

                // Check for existing connection
                for (auto& connection : m_Connections) {
                    if (connection == m_CurrentConnection) {
                        dropCable();
                        return false; // Connection already exists, do not add it again
                    }
                }

                addCurrentConnection(); // Finalize connection
                return true;
            }
        }

        dropCable(); // No connection added, so drop the cable
        return false;
    }

    void PatchBay::dropCable() {
        auto maxVelocity = 10;
        auto velocity = m_LastMousePosition - m_LastLastMousePosition;
        velocity = Rect<>{ -maxVelocity, -maxVelocity, 2 * maxVelocity, 2 * maxVelocity }.getConstrainedPoint(velocity);

        if (m_CurrentConnection.begin == npos) {
            m_CurrentConnection.m_Segments.front().velocity = velocity;
        }

        if (m_CurrentConnection.end == npos) {
            m_CurrentConnection.m_Segments.back().velocity = velocity;
        }

        if (Storage::flag(CablePhysics)) {
            m_FallingConnections.push_back(m_CurrentConnection);
        }

        m_CurrentConnection.reset();
    }

    // ------------------------------------------------
    
    void PatchBay::presetLoaded() {
        auto& data = context.data<ControllerData>().connections;

        if (Storage::flag(CablePhysics)) {
            for (auto& connection : m_Connections) {
                m_FallingConnections.push_back(connection);
            }
        }

        m_Connections.clear();
        m_Changing = true;

        for (auto& connection : data) {
            auto& c = m_Connections.emplace_back();

            bool foundBegin = false;
            bool foundEnd = false;
            for (auto& jack : m_Jacks) {
                if (jack->settings.source == connection.source) {
                    c.begin = jack->m_Id;
                    foundBegin = true;
                }

                if (jack->settings.destination == connection.destination) {
                    c.end = jack->m_Id;
                    foundEnd = true;
                }

                if (foundBegin && foundEnd) break;
            }

            if (!foundBegin || !foundEnd) {
                m_Connections.pop_back();
                continue;
            }

            c.m_Color = connection.color;
            c.initPosition(*this);
        }
    }

    // ------------------------------------------------

    void PatchBay::addCurrentConnection() {
        m_Connections.push_back(m_CurrentConnection);
        addConnection(m_CurrentConnection);
        m_CurrentConnection.reset();
    }

    void PatchBay::addConnection(Connection& con) {
        context.data<ControllerData>().connections.push_back({
            .source = con.source(*this),
            .destination = con.destination(*this),
            .color = static_cast<int>(con.m_Color)
        });
        modifyConnection(con, true);
    }

    void PatchBay::removeConnection(Connection& con) {
        std::erase_if(context.data<ControllerData>().connections,
            [&](ControllerData::Connection& el) {
                return el.destination == con.destination(*this) 
                    && el.source == con.source(*this);
            });

        modifyConnection(con, false);
    }

    void PatchBay::modifyConnection(Connection& con, bool enable) {
        auto& a = m_Jacks[con.begin];
        auto& b = m_Jacks[con.end];

        ModSource source = a->output() ? a->settings.source : b->settings.source;
        ModDestination destination = a->input() ? a->settings.destination : b->settings.destination;

        if (source == ModSource::None || destination == ModDestination::None) return;

        context.interface<Processing::ModInterface>()(source, destination, enable);
    }

    // ------------------------------------------------

    bool PatchBay::changing() {
        if (!m_Changing) return false;

        if (!Storage::flag(CablePhysics)) {
            auto val = m_Changing;
            m_Changing = false;
            return val;
        }

        if (!m_FallingConnections.empty()) {
            m_NotChangingButStillRedraw = false;
            return true;
        }

        if (m_CurrentConnection.changing()) {
            m_NotChangingButStillRedraw = false;
            return true;
        }

        for (auto& connection : m_Connections) {
            if (connection.changing()) {
                m_NotChangingButStillRedraw = false;
                return true;
            }
        }

        // Not actually changing, but still redrawing for another couple millis
        if (!m_NotChangingButStillRedraw) {
            m_LastChanging = std::chrono::steady_clock::now();
            m_NotChangingButStillRedraw = true;
        } else {
            // Check if time since last change is greater than
            auto now = std::chrono::steady_clock::now();
            if (now - m_LastChanging > std::chrono::milliseconds(200)) {
                m_Changing = false; // Finally set changing to false
                m_NotChangingButStillRedraw = false;
            }
        }

        return true;
    }

    // ------------------------------------------------

    PatchBay::JackId PatchBay::addJack(Jack& jack) {
        m_Jacks.push_back(&jack);
        return m_Jacks.size() - 1;
    }

    // ------------------------------------------------

}
    
// ------------------------------------------------
