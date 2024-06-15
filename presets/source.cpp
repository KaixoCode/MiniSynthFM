
// ------------------------------------------------

#include <filesystem>
#include <iostream>
#include <fstream>
#include <ranges>
#include <vector>
#include <string_view>

// ------------------------------------------------

#include "Kaixo/Utils/StringUtils.hpp"
#include "Kaixo/Utils/json.hpp"

// ------------------------------------------------

int main(const int argc, char const* const* const argv) {

    // ------------------------------------------------

    std::vector<std::string_view> args{ argv, std::next(argv, static_cast<std::ptrdiff_t>(argc)) };

    if (args.size() != 3) return 1;

    // ------------------------------------------------

    std::string presetDir{ args[1] };
    std::string output{ args[2] };

    std::ofstream out{ output };

    out << "#pragma once\n";
    out << "// THIS FILE IS GENERATED, DO NOT EDIT\n";
    out << "\n";
    out << "// ------------------------------------------------\n";
    out << "\n";
    out << "namespace Kaixo {\n";
    out << "    \n";
    out << "    // ------------------------------------------------\n";
    out << "    \n";
    out << "    constexpr static std::pair<std::string_view, std::string_view> Presets[]{\n";

    for (auto& file : std::filesystem::directory_iterator(presetDir)) {
        if (!file.is_regular_file()) continue;
        if (file.path().extension() != ".minifm") continue;

        std::ifstream in{ file.path() };
        std::string content = Kaixo::file_to_string(in);

        if (auto json = Kaixo::basic_json::parse(content)) {
            auto& preset = json.value();

            out << "        { \"" << preset["class Kaixo::PresetData"]["name"].as<std::string>() << "\", R\"~~(" << content << ")~~\" },\n";
        }
    }

    out << "    };\n";
    out << "    \n";
    out << "    // ------------------------------------------------\n";
    out << "    \n";
    out << "}\n";
    out << "\n";
    out << "// ------------------------------------------------\n";
    out << "\n";

    // ------------------------------------------------

    return 0;

    // ------------------------------------------------

}

// ------------------------------------------------
