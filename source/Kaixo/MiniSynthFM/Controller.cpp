
// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Controller.hpp"

// ------------------------------------------------

#include "Kaixo/Core/Theme/Theme.hpp"

// ------------------------------------------------

namespace Kaixo {

    // ------------------------------------------------
    
    void PresetData::init() {
        name = "";
        author = "";
        type = "";
        description = "";
    }

    basic_json PresetData::serialize() {
        basic_json json;
        json["name"] = name;
        json["author"] = author;
        json["type"] = type;
        json["description"] = description;
        return json;
    }

    void PresetData::deserialize(basic_json& json) {
        json.try_get_or_default("name", name, "");
        json.try_get_or_default("author", author, "");
        json.try_get_or_default("type", type, "");
        json.try_get_or_default("description", description, "");
    }

    // ------------------------------------------------

    void ControllerData::init() {
        connections.clear();
    }

    basic_json ControllerData::serialize() {
        basic_json json;

        json = basic_json::array();
        for (auto& connection : connections) {
            basic_json c = basic_json::object();
            c["source"] = toString(connection.source);
            c["destination"] = toString(connection.destination);
            c["color"] = connection.color;
            json.push_back(c);
        }

        return json;
    }

    void ControllerData::deserialize(basic_json& data) {
        connections.clear();
        data.foreach([&](basic_json& c) {
            if (c.contains("source", basic_json::String) &&
                c.contains("destination", basic_json::String) &&
                c.contains("color", basic_json::Number))
            {
                auto& connection = connections.emplace_back();
                connection.source = sourceFromString(c["source"].as<std::string>());
                connection.destination = destFromString(c["destination"].as<std::string>());
                connection.color = c["color"].as<int>();
            }
        });
    }

    // ------------------------------------------------

    MiniSynthFMController::MiniSynthFMController() {

        // ------------------------------------------------

        Gui::T.initialize();

        // ------------------------------------------------
        
        data<ControllerData>();
        data<PresetData>();

        // ------------------------------------------------
        
        linkPitchWheel(Synth.pitchBendParameter);
        linkModWheel(Synth.modWheelParameter);

        // ------------------------------------------------

    }

    // ------------------------------------------------

    Controller* createController() { return new MiniSynthFMController; }

    // ------------------------------------------------

}
    
// ------------------------------------------------
