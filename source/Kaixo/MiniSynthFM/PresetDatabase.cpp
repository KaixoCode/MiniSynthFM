
// ------------------------------------------------

#include "Kaixo/MiniSynthFM/PresetDatabase.hpp"

// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Controller.hpp"

// ------------------------------------------------

#include "DefaultPresets.hpp"

// ------------------------------------------------

namespace Kaixo {

    // ------------------------------------------------
    
    struct FactoryPresets {
        static std::string_view findPair(std::string_view search) {
            for (auto& [name, json] : Presets) {
                if (name == search) return json;
            }
            return "";
        }

        static PresetData find(std::string_view search) {
            auto json = findPair(search);
            if (auto result = basic_json::parse(json)) {
                auto pdata = typeid(PresetData).name();
                if (result->contains(pdata)) {
                    PresetData data;
                    data.deserialize(result.value()[pdata]);
                    return data;
                }
            }

            return {};
        }

        static basic_json get(std::string_view search) {
            auto json = findPair(search);
            if (auto result = basic_json::parse(json)) {
                return result.value();
            }

            return {};
        }
    };

    // ------------------------------------------------

    PresetDatabase::PresetDatabase(Controller& c)
        : controller(c)
    {
        loadedPreset.bank = "Factory";
        loadedPreset.name = "Init";
        reloadInformation();
    }

    PresetDatabase::~PresetDatabase() {
        if (m_LoadBanksThread.joinable())
            m_LoadBanksThread.join();
    }

    // ------------------------------------------------

    void PresetDatabase::reloadInformation() {
        if (m_LoadBanksThread.joinable())
            m_LoadBanksThread.join(); // Wait for previous reload if exists

        banks.clear();

        banks.emplace_back(*this, Bank::Type::Factory, m_BankIdCounter++, "Factory");
        banks[0].reloadInformation();

        if (auto optPath = Storage::get<std::string>(PresetPath)) {
            std::filesystem::path path = optPath.value();

            if (!std::filesystem::exists(path)) return;

            banks.emplace_back(*this, Bank::Type::Bank, m_BankIdCounter++, "User", path);

            for (auto& entry : std::filesystem::directory_iterator(path)) {
                if (entry.is_directory() && entry.exists()) {
                    banks.emplace_back(*this, Bank::Type::Bank, m_BankIdCounter++, entry.path().stem().string(), entry);
                }
            }
        }

        m_LoadBanksThread = std::thread([this]() {
            for (std::size_t i = 1; i < banks.size(); ++i) {
                auto& bank = banks[i];
                bank.reloadInformation();
            }
        });
    }

    // ------------------------------------------------

    void PresetDatabase::Bank::Preset::reloadInformation() {
        if (type == Type::Init) {
            name = "Init";
            presetData.name = name;
        } else if (type == Type::Factory) {
            presetData = FactoryPresets::find(name);
        } else {
            if (auto json = basic_json::parse(file_to_string(path))) {
                auto pdata = typeid(PresetData).name();
                if (json->contains(pdata)) {
                    presetData.deserialize(json.value()[pdata]);
                }
            }
            name = presetData.name;
        }
    }

    // ------------------------------------------------

    void PresetDatabase::Bank::Preset::load() const {
        database.loadedPreset.bank = bank.name;
        database.loadedPreset.name = name;

        if (type == Type::Init) {
            database.controller.initPreset();
        } else if (type == Type::Factory) {
            auto json = FactoryPresets::get(name);
            database.controller.loadPresetFromJson(json);
        } else {
            database.controller.loadPreset(path);
        }
    }

    // ------------------------------------------------

    PresetDatabase::Bank::Bank(PresetDatabase& d, PresetDatabase::Bank::Type t, std::size_t id, std::string n, std::filesystem::path f)
        : database(d), type(t), name(n), folder(f), id(id)
    {}

    // ------------------------------------------------

    void PresetDatabase::Bank::reloadInformation() {
        std::lock_guard _{ m_Mutex };
        m_Presets.clear();
        if (type == Type::Factory) {
            m_Presets.emplace_back(Preset{
                .database = database,
                .bank = *this,
                .id = database.m_PresetIdCounter++,
                .type = Preset::Type::Init,
                .name = "Init"
            }).reloadInformation();

            for (auto& [name, json] : Presets) {
                m_Presets.emplace_back(Preset{
                    .database = database,
                    .bank = *this,
                    .id = database.m_PresetIdCounter++,
                    .type = Preset::Type::Factory,
                    .name = std::string{ name }
                }).reloadInformation();
            }
        } else {
            std::vector<std::filesystem::path> files;
            for (auto& file : std::filesystem::directory_iterator(folder)) {
                if (file.is_regular_file() && file.path().extension() == ".minifm") 
                    files.push_back(file.path());
            }
            std::ranges::sort(files);
            for (auto& path : files) {
                m_Presets.emplace_back(Preset{
                    .database = database,
                    .bank = *this,
                    .id = database.m_PresetIdCounter++,
                    .type = Preset::Type::Normal,
                    .path = path
                }).reloadInformation();
            }
        }
        m_Loaded = true;
    }

    // ------------------------------------------------

    const std::vector<PresetDatabase::Bank::Preset>& PresetDatabase::Bank::presets() const {
        while (!m_Loaded) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        std::lock_guard _{ m_Mutex };
        return m_Presets;
    }

    // ------------------------------------------------

    void PresetDatabase::loadNextPreset() {
        for (std::size_t j = 0; j < banks.size(); ++j) {
            auto& bank = banks[j];
            if (bank.name != loadedPreset.bank) continue;

            auto& presets = bank.presets();
            for (std::size_t i = 0; i < presets.size(); ++i) {
                auto& preset = presets[i];
                if (preset.name != loadedPreset.name) continue;

                if (i != presets.size() - 1) { // Not final preset in bank, just load
                    auto& next = presets[i + 1];
                    next.load();
                    return; // Done
                } else while (true) { // Otherwise go to next bank
                    if (j == banks.size() - 1) { // Final bank, Wrap around to first = init preset
                        controller.initPreset();
                        loadedPreset.name = "Init";
                        loadedPreset.bank = "Factory";
                        return; // Done
                    }

                    auto& nextBankPresets = banks[j + 1].presets();

                    // Next bank has no presets -> try next bank
                    if (nextBankPresets.size() == 0) {
                        j++;
                        continue; // while (true)
                    } else {
                        auto& next = nextBankPresets.front();
                        next.load();
                        return; // Done
                    }
                }
            }
        }
    }

    void PresetDatabase::loadPreviousPreset() {
        for (std::size_t j = 0; j < banks.size(); ++j) {
            auto& bank = banks[j];
            if (bank.name != loadedPreset.bank) continue;

            auto& presets = bank.presets();
            for (std::size_t i = 0; i < presets.size(); ++i) {
                auto& preset = presets[i];
                if (preset.name != loadedPreset.name) continue;

                if (i != 0) { // Not first preset in bank, just load
                    auto& previous = presets[i - 1];
                    previous.load();
                    return; // Done
                } else while (true) { // Otherwise go to next bank
                    if (j == 0) { // First bank, Wrap around to last
                        j = banks.size();
                        continue; // Done
                    }

                    auto& previousBankPresets = banks[j - 1].presets();

                    // Next bank has no presets -> try previous bank
                    if (previousBankPresets.size() == 0) {
                        j--;
                        continue; // while (true)
                    } else {
                        auto& previous = previousBankPresets.back();
                        previous.load();
                        return; // Done
                    }
                }
            }
        }
    }

    // ------------------------------------------------

    const PresetDatabase::Bank::Preset* PresetDatabase::preset(std::size_t id) {
        for (auto& bank : banks) {
            for (auto& preset : bank.presets()) {
                if (preset.id == id) {
                    return &preset;
                }
            }
        }

        return nullptr;
    }
    
    const PresetDatabase::Bank* PresetDatabase::bank(std::size_t id) {
        for (auto& bank : banks) {
            if (bank.id == id) return &bank;
        }

        return nullptr;
    }

    // ------------------------------------------------

}

// ------------------------------------------------
 