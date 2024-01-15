#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Theme/Theme.hpp"

// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Controller.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

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

            Jack(Context c, Settings s);

            // ------------------------------------------------

            void paint(juce::Graphics& g) override;

            // ------------------------------------------------

            Point<> holePosition() const;

            // ------------------------------------------------

            void mouseDrag(const juce::MouseEvent& event) override;
            void mouseDown(const juce::MouseEvent& event) override;
            void mouseUp(const juce::MouseEvent& event) override;

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

            bool operator==(const Connection& other) const;

            // ------------------------------------------------

            void reset();

            // ------------------------------------------------

            void paint(juce::Graphics& g, Point<> mouse, PatchBay& self);

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

        PatchBay(Context c);

        // ------------------------------------------------

        void onIdle() override;

        // ------------------------------------------------

        void paint(juce::Graphics& g) override;

        // ------------------------------------------------

        void beginCable(JackId id);
        void removeCable(JackId id);
        void moveCable(Point<> to);
        bool finishCable();

        // ------------------------------------------------

    private:
        std::vector<Jack*> m_Jacks;
        std::vector<Connection> m_Connections;
        Connection m_CurrentConnection{};
        Point<> m_LastMousePosition;
        struct Cable {
            Theme::Drawable end;
            Theme::Color color;
        } m_CableGraphics[5];

        // ------------------------------------------------

        void addCurrentConnection();
        void addConnection(Connection& con);
        void removeConnection(Connection& con);
        void modifyConnection(Connection& con, bool enable);

        // ------------------------------------------------

        JackId addJack(Jack& jack);

        // ------------------------------------------------

    };

    // ------------------------------------------------

    using Jack = PatchBay::Jack;

    // ------------------------------------------------

}

// ------------------------------------------------
