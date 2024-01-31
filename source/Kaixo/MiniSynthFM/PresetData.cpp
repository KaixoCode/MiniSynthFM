
// ------------------------------------------------

#include "Kaixo/MiniSynthFM/PresetData.hpp"

// ------------------------------------------------

namespace Kaixo {

    // ------------------------------------------------

    void PresetData::init() {
        name = "Init";
        author = "";
        type = "";
        description = "MiniFM is a 3-Operator FM synth with a driven lowpass filter and a simple delay effect, all of which can be modulated with envelopes and an lfo.";
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

}

// ------------------------------------------------
