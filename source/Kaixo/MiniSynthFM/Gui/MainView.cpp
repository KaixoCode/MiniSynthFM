
// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Gui/MainView.hpp"

// ------------------------------------------------

#include "Kaixo/Core/Gui/Views/ImageView.hpp"
#include "Kaixo/Core/Gui/Views/ScrollView.hpp"
#include "Kaixo/Core/Gui/Views/TextView.hpp"
#include "Kaixo/Core/Gui/Views/FilterDisplay.hpp"
#include "Kaixo/Core/Gui/Knob.hpp"
#include "Kaixo/Core/Gui/Button.hpp"
#include "Kaixo/Core/Theme/Theme.hpp"

// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Controller.hpp"
#include "Kaixo/MiniSynthFM/Processing/Processor.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    class Catenary {
    public:
        Catenary(float x0, float y0, float x1, float y1, float addLength, int numIterations = 1) {
            assert(x0 != x1);
            assert(addLength > 0.f);
            // make sure x1 is right
            if (x0 > x1) {
                std::swap(x0, x1);
                std::swap(y0, y1);
            }

            auto x_d = x1 - x0;
            auto y_d = y1 - y0;

            auto d = std::hypot(x_d, y_d);
            auto L = d + addLength;

            auto x_f = std::sqrt(L * L - y_d * y_d) / x_d; // always > 1

            // need to solve sinh(xi)/xi - x_f = 0

            //  auto xi_approx = 1.15f*std::log(x_f - 0.8f) + 1.9f; // manual fit for xi>2, guaranteed too low.
            auto xi_approx = 1.16f * std::log(x_f - 0.75f) + 1.9f; // manual fit for large 2<xi<10..
            if (xi_approx < 2.f) {
                // taylor-approx: 1 + xi^2/6 + xi^4/120 + xi^6/5040 - x_f = 0
                // use inverse taylor series for smallish values. Always slightly too high.
                auto tmp = std::cbrt(std::sqrt(15680.f) * std::sqrt(x_f * (405.f * x_f + 198.f) + 62.f) + 2520.f * x_f + 616.f);
                xi_approx = std::sqrt(tmp - 84.f / tmp - 14.f);
            }

            auto xi = xi_approx;
            for (auto n = 0; n < numIterations; ++n) { // newton iterations to improve precision
                auto x = xi;
                auto exp_half = 0.5f * std::exp(x); // sinh and cosh at the same time from exp
                auto iexp_half = 0.25f / exp_half;
                auto sinhx = exp_half - iexp_half;
                auto coshx = exp_half + iexp_half;
                auto val = sinhx / x - x_f;
                auto der = (coshx * x - sinhx) / (x * x);
                xi -= val / der;
            }

            a = 0.5f * x_d / xi;
            inva = 1.f / a;
            b = 0.5f * (x0 + x1) - a * std::asinh(0.5f * y_d / (a * std::sinh(xi)));
            c = y0 - (a * std::cosh((x0 - b) * inva));
        }

        // determine the y pos for a given x.
        float calcY(float x) const {
            auto y = a * std::cosh((x - b) * inva) + c;
            return y;
        }

        // get curve vertex = lowest point
        std::pair<float, float> getVertex() const {
            return { b, a + c };
        }

    private:
        float a; // curvature = radius of the circle fitting inside the curve at the vertex
        float inva; // inverse of a to avoid a division per point
        float b; // x offset. xpos of the vertex
        float c; // y offset. note that cosh(0) = 1
    };
    // ------------------------------------------------
    
    class PatchBay : public View {
    public:

        // ------------------------------------------------

        using JackId = std::size_t;

        // ------------------------------------------------

        class Jack : public View {
        public:

            // ------------------------------------------------

            enum class Type { Input, Output };

            // ------------------------------------------------

            struct Settings {

                // ------------------------------------------------

                Theme::Drawable graphics;
                Type type = Type::Input;

                // ------------------------------------------------

                PatchBay& patchBay;

                // ------------------------------------------------
                
                ModSource source;
                ModDestination destination;

                // ------------------------------------------------
                
                std::string name = "";

                // ------------------------------------------------

            } settings;

            // ------------------------------------------------

            Jack(Context c, Settings s)
                : View(c), settings(std::move(s))
            {
                m_Id = settings.patchBay.addJack(*this);
            }

            // ------------------------------------------------

            void paint(juce::Graphics& g) override {

                // ------------------------------------------------

                settings.graphics.draw({
                    .graphics = g,
                    .bounds = localDimensions(),
                    .text = settings.name
                });

                // ------------------------------------------------

            }

            // ------------------------------------------------
            
            Point<> holePosition() const {
                return position() + Point{ 32, 15 };
            }

            // ------------------------------------------------

            void mouseDrag(const juce::MouseEvent& event) override {
                auto e = event.getEventRelativeTo(&settings.patchBay);
                settings.patchBay.moveCable({ e.x, e.y });
            }

            void mouseDown(const juce::MouseEvent& event) override {
                if (event.mods.isLeftButtonDown()) {
                    settings.patchBay.beginCable(m_Id);
                    auto e = event.getEventRelativeTo(&settings.patchBay);
                    settings.patchBay.moveCable({ e.x, e.y });
                }

                if (event.mods.isRightButtonDown()) {
                    settings.patchBay.removeCable(m_Id);
                    auto e = event.getEventRelativeTo(&settings.patchBay);
                    settings.patchBay.moveCable({ e.x, e.y });
                }
            }

            void mouseUp(const juce::MouseEvent& event) override {
                settings.patchBay.finishCable();
            }

            // ------------------------------------------------

            bool input() const { return settings.type == Type::Input; }
            bool output() const { return settings.type == Type::Output; }

            // ------------------------------------------------

        private:
            PatchBay::JackId m_Id = npos;

            // ------------------------------------------------

        };

        // ------------------------------------------------

        struct Connection {

            // ------------------------------------------------
            
            constexpr static std::size_t Segments = 7;

            // ------------------------------------------------

            JackId begin = npos;
            JackId end = npos;

            // ------------------------------------------------

            bool operator==(const Connection& other) const {
                return other.begin == begin && other.end == end ||
                    other.begin == end && other.end == begin;
            }

            // ------------------------------------------------

            void reset() {
                begin = npos;
                end = npos;

                for (auto& segment : m_Segments) {
                    segment.y = 0;
                    segment.vy = 0;
                }

                m_Color = (m_Color + 1) % 5;
            }

            // ------------------------------------------------

            void paint(juce::Graphics& g, Point<> mouse, PatchBay& self) {
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

        private:
            struct Segment {
                float y = 0;
                float vy = 0;
            };

            std::array<Segment, Segments> m_Segments;

            float m_Distance = 0;
            float m_DeltaDistance = 0;

            std::size_t m_Color = 0;

            // ------------------------------------------------
            
            friend class PatchBay;

            // ------------------------------------------------

        };

        // ------------------------------------------------

        PatchBay(Context c) 
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
        
        void onIdle() override {
            repaint();
        }

        // ------------------------------------------------
        
        void paint(juce::Graphics& g) override {
            for (auto& connection : m_Connections) {
                connection.paint(g, m_LastMousePosition, *this);
            }
            m_CurrentConnection.paint(g, m_LastMousePosition, *this);
        }

        // ------------------------------------------------
        
        void beginCable(JackId id) {
            m_CurrentConnection.begin = id;
            m_CurrentConnection.end = npos;
        }

        void removeCable(JackId id) {
            for (std::size_t i = m_Connections.size(); i > 0; --i) {
                auto& connection = m_Connections[i - 1];
                if (connection.begin == id) {
                    m_CurrentConnection = connection;
                    m_CurrentConnection.begin = connection.end;
                    m_CurrentConnection.end = npos;
                    removeConnection(connection);
                    m_Connections.erase(m_Connections.begin() + (i - 1));
                    return;
                } else if (connection.end == id) {
                    m_CurrentConnection = connection;
                    m_CurrentConnection.begin = connection.begin;
                    m_CurrentConnection.end = npos;
                    removeConnection(connection);
                    m_Connections.erase(m_Connections.begin() + (i - 1));
                    return;
                }
            }
        }

        void moveCable(Point<> to) {
            m_LastMousePosition = to;
        }

        void finishCable() {
            if (m_CurrentConnection.begin == npos) return;
            for (JackId id = 0; id < m_Jacks.size(); ++id) {
                auto& jack = m_Jacks[id];
                auto& begin = m_Jacks[m_CurrentConnection.begin];
                if (id != m_CurrentConnection.begin && // Not same jack
                    jack->input()  != begin->input()  && // Not both same type
                    jack->output() != begin->output() &&
                    jack->dimensions().contains(m_LastMousePosition)) 
                {
                    m_CurrentConnection.end = id;

                    for (auto& connection : m_Connections) {
                        if (connection == m_CurrentConnection) {
                            m_CurrentConnection.reset();
                            return; // Connection already exists, do not add it again
                        }
                    }

                    addCurrentConnection(); // Finalize connection
                    return;
                }
            }

            m_CurrentConnection.reset(); // No connection added, so reset
        }

        // ------------------------------------------------

    private:
        std::vector<Jack*> m_Jacks;
        std::vector<Connection> m_Connections;
        Connection m_CurrentConnection{};
        Point<> m_LastMousePosition;
        Point<> m_PreviousMousePos;
        Point<float> m_MouseVelocity;
        struct Cable {
            Theme::Drawable end;
            Theme::Color color;
        } m_CableGraphics[5];

        // ------------------------------------------------
        
        void addCurrentConnection() {
            m_Connections.push_back(m_CurrentConnection);
            addConnection(m_CurrentConnection);
            m_CurrentConnection.reset();
        }

        void addConnection(Connection& con) {
            auto& a = m_Jacks[con.begin];
            auto& b = m_Jacks[con.end];

            ModSource source = a->output() ? a->settings.source : b->settings.source;
            ModDestination destination = a->input() ? a->settings.destination : b->settings.destination;

            if (source == ModSource::None || destination == ModDestination::None) return;

            context.interface<Processing::ModInterface>()->call(source, destination, true);
        }

        void removeConnection(Connection& con) {
            auto& a = m_Jacks[con.begin];
            auto& b = m_Jacks[con.end];

            ModSource source = a->output() ? a->settings.source : b->settings.source;
            ModDestination destination = a->input() ? a->settings.destination : b->settings.destination;

            if (source == ModSource::None || destination == ModDestination::None) return;

            context.interface<Processing::ModInterface>()->call(source, destination, false);
        }

        // ------------------------------------------------

        JackId addJack(Jack& jack) { 
            m_Jacks.push_back(&jack); 
            return m_Jacks.size() - 1;
        }

        // ------------------------------------------------

    };

    // ------------------------------------------------
    
    using Jack = PatchBay::Jack;

    // ------------------------------------------------
    
    class Led : public View {
    public:

        // ------------------------------------------------

        struct Settings {

            // ------------------------------------------------

            Theme::Drawable graphics;
            Processing::InterfaceStorage<float()> value;

            // ------------------------------------------------

        } settings;

        // ------------------------------------------------

        Led(Context c, Settings settings)
            : View(c), settings(std::move(settings))
        {
            wantsIdle(true);
        }

        // ------------------------------------------------

        void paint(juce::Graphics& g) override {

            // ------------------------------------------------

            settings.graphics.draw({
                .graphics = g,
                .bounds = localDimensions(),
                .value = m_Value
            });

            // ------------------------------------------------

        }

        // ------------------------------------------------

        void onIdle() override {
            float value = settings.value();
            if (m_Value != value) {
                m_Value = value;
                repaint();
            }
        }

        // ------------------------------------------------
    private:
        float m_Value = 0;

        // ------------------------------------------------

    };

    // ------------------------------------------------

    MainView::MainView(Context c)
        : View(c)
    {
        // ------------------------------------------------

        wantsIdle(true);

        // ------------------------------------------------
        
        context.tooltip().background(T.tooltip.background);
        context.tooltip().font(T.tooltip.font);
        context.tooltip().textColor(T.tooltip.textColor);

        // ------------------------------------------------

        add<ImageView>({ T.background });

        // ------------------------------------------------

        add<TextView>({ 300, 150, 100, 100 }, {
            .graphics = T.text
        });

        // ------------------------------------------------
        
        auto& patchBay = add<PatchBay>();
        
        // ------------------------------------------------

        add<Button>({ 411, 33, 75, 23 }, {
            .graphics = T.toggle,
            .behaviour = Button::Behaviour::Toggle,
            .param = Synth.filter.enable
        });

        add<Knob>({ 353, 64, 64, 64 }, {
            .graphics = T.knob,
            .tooltipName = false,
            .tooltipValue = false,
            .param = Synth.filter.frequency
        });

        add<Jack>({ 353, 140, 64, 52 }, {
            .graphics = T.inputJack,
            .type = Jack::Type::Input,
            .patchBay = patchBay,
            .destination = ModDestination::FilterFreq,
            .name = "Cutoff"
        });

        add<Knob>({ 422, 64, 64, 64 }, {
            .graphics = T.knob,
            .tooltipName = false,
            .tooltipValue = false,
            .param = Synth.filter.resonance
        });
        
        add<Knob>({ 422, 128, 64, 64 }, {
            .graphics = T.knob,
            .tooltipName = false,
            .tooltipValue = false,
            .param = Synth.filter.drive
        });

        // ------------------------------------------------

        for (std::size_t i = 0; i < Oscillators; ++i) {

            // ------------------------------------------------

            add<Knob>({ 37 + i * 280, 244, 64, 64 }, {
                .graphics = T.knob,
                .tooltipName = false,
                .tooltipValue = false,
                .param = Synth.oscillator[i].volume
            });
            
            add<Knob>({ 37 + i * 280, 308, 64, 64 }, {
                .graphics = T.knob,
                .tooltipName = false,
                .tooltipValue = false,
                .param = Synth.oscillator[i].wave
            });

            add<Knob>({ 103 + i * 280, 308, 64, 64 }, {
                .graphics = T.knob,
                .tooltipName = false,
                .tooltipValue = false,
                .param = Synth.oscillator[i].fm
            });
            
            add<Knob>({ 103 + i * 280, 244, 64, 64 }, {
                .graphics = T.knob,
                .tooltipName = false,
                .tooltipValue = false,
                .param = Synth.oscillator[i].tune
            });
            
            add<Jack>({ 169 + i * 280, 256, 64, 52 }, {
                .graphics = T.inputJack,
                .type = Jack::Type::Input,
                .patchBay = patchBay,
                .destination = i == 0 ? ModDestination::Op1FM
                             : i == 1 ? ModDestination::Op2FM
                             : ModDestination::Op3FM,
                .name = "FM"
            });
            
            add<Jack>({ 169 + i * 280, 320, 64, 52 }, {
                .graphics = T.inputJack,
                .type = Jack::Type::Input,
                .patchBay = patchBay,
                .destination = i == 0 ? ModDestination::Op1Amount
                             : i == 1 ? ModDestination::Op2Amount
                             :          ModDestination::Op3Amount,
                .name = "Amount"
            });

            add<Jack>({ 235 + i * 280, 256, 64, 52 }, {
                .graphics = T.inputJack,
                .type = Jack::Type::Input,
                .patchBay = patchBay,
                .destination = i == 0 ? ModDestination::Op1Sync
                             : i == 1 ? ModDestination::Op2Sync
                             : ModDestination::Op3Sync,
                .name = "Sync"
            });
            
            add<Jack>({ 235 + i * 280, 320, 64, 52 }, {
                .graphics = T.outputJack,
                .type = Jack::Type::Output,
                .patchBay = patchBay,
                .source = i == 0 ? ModSource::Op1
                        : i == 1 ? ModSource::Op2
                        :          ModSource::Op3
            });

            // ------------------------------------------------
            
            add<Button>({ 215 + i * 280, 213, 80, 23 }, {
                .graphics = T.toggle,
                .behaviour = Button::Behaviour::Toggle,
                .param = Synth.oscillator[i].output
            });

            // ------------------------------------------------
            
            add<Knob>({ 92 + i * 280, 213, 80, 23 }, {
                .graphics = T.threewayToggle,
                .type = Knob::Type::Both,
                .tooltipName = false,
                .tooltipValue = false,
                .param = Synth.oscillator[i].octave
            });

            // ------------------------------------------------

        }

        // ------------------------------------------------
        
        for (std::size_t i = 0; i < Lfos; ++i) {

            // ------------------------------------------------

            add<Knob>({ 195, 64, 64, 64 }, {
                .graphics = T.knob,
                .tooltipName = false,
                .tooltipValue = false,
                .param = Synth.lfo[i].depth
            });
            
            add<Knob>({ 264, 64, 64, 64 }, {
                .graphics = T.knob,
                .tooltipName = false,
                .tooltipValue = false,
                .param = Synth.lfo[i].time
            });

            add<Knob>({ 195, 128, 64, 64 }, {
                .graphics = T.knob,
                .tooltipName = false,
                .tooltipValue = false,
                .param = Synth.lfo[i].waveform
            });

            // ------------------------------------------------

            add<Button>({ 239, 33, 75, 23 }, {
                .graphics = T.toggle,
                .behaviour = Button::Behaviour::Toggle,
                .param = Synth.lfo[i].synced
            });

            // ------------------------------------------------

            add<Jack>({ 264, 140, 64, 52 }, {
                .graphics = T.outputJack,
                .type = Jack::Type::Output,
                .patchBay = patchBay,
                .source = ModSource::LFO
            });

            // ------------------------------------------------
            
            add<Led>({ 315, 38, 13, 13 }, {
                .graphics = T.led,
                .value = context.interface<Processing::LfoInterface>({ .index = i })
            });

            // ------------------------------------------------

        }

        // ------------------------------------------------
        
        for (std::size_t i = 0; i < Envelopes; ++i) {
            add<Knob>({ 880, 71 + i * 180, 64, 64 }, {
                .graphics = T.knob,
                .tooltipName = false,
                .tooltipValue = false,
                .param = Synth.envelope[i].level
            });
                        
            add<Knob>({ 957, 52 + i * 180, 60, 140 }, {
                .graphics = T.slider,
                .tooltipName = false,
                .tooltipValue = false,
                .param = Synth.envelope[i].attack
            });

            add<Knob>({ 1017, 52 + i * 180, 60, 140 }, {
                .graphics = T.slider,
                .tooltipName = false,
                .tooltipValue = false,
                .param = Synth.envelope[i].decay
            });

            add<Knob>({ 1077, 52 + i * 180, 60, 140 }, {
                .graphics = T.slider,
                .tooltipName = false,
                .tooltipValue = false,
                .param = Synth.envelope[i].sustain
            });

            add<Knob>({ 1137, 52 + i * 180, 60, 140 }, {
                .graphics = T.slider,
                .tooltipName = false,
                .tooltipValue = false,
                .param = Synth.envelope[i].release
            });
            
            // ------------------------------------------------
            
            add<Jack>({ 880, 140 + i * 180, 64, 52 }, {
                .graphics = T.outputJack,
                .type = Jack::Type::Output,
                .patchBay = patchBay,
                .source = i == 0 ? ModSource::Envelope1 : ModSource::Envelope2
            });
            
            // ------------------------------------------------
            
            add<Button>({ 942, 33 + i * 180, 80, 23 }, {
                .graphics = T.toggle,
                .behaviour = Button::Behaviour::Toggle,
                .param = Synth.envelope[i].loop
            });
            
            // ------------------------------------------------

            add<Led>({ 1182, 38 + i * 180, 13, 13 }, {
                .graphics = T.led,
                .value = context.interface<Processing::EnvelopeInterface>({ .index = i })
            });

            // ------------------------------------------------

        }

        // ------------------------------------------------
        
        // Move patch bay to end of views, so it draws on top
        removeChildComponent(&patchBay);
        addChildComponent(&patchBay);

        // ------------------------------------------------
        
    }

    // ------------------------------------------------

    void MainView::mouseMove(const juce::MouseEvent& event) {}

    // ------------------------------------------------

    View* createBaseView(Context context) { return new MainView{ context }; }

    // ------------------------------------------------

}
    
// ------------------------------------------------
