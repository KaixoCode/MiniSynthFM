
// ------------------------------------------------

#include "Kaixo/MiniSynthFM/PresetDatabase.hpp"

// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Controller.hpp"

// ------------------------------------------------

namespace Kaixo {

    // ------------------------------------------------

    PresetDatabase::PresetDatabase(Controller& c)
        : controller(c)
    {
        loadedPreset.bank = "Factory";
        loadedPreset.name = "Init";
        reloadInformation();
    }

    PresetDatabase::~PresetDatabase() {
        m_LoadBanksThread.join();
    }

    // ------------------------------------------------

    void PresetDatabase::reloadInformation() {
        if (m_LoadBanksThread.joinable())
            m_LoadBanksThread.join(); // Wait for previous reload if exists

        banks.clear();

        banks.emplace_back(*this, Bank::Type::Factory, "Factory");
        banks[0].reloadInformation();

        if (auto optPath = Storage::get<std::string>(PresetPath)) {
            std::filesystem::path path = optPath.value();

            if (!std::filesystem::exists(path)) return;

            banks.emplace_back(*this, Bank::Type::Bank, "User", path);

            for (auto& entry : std::filesystem::directory_iterator(path)) {
                if (entry.is_directory() && entry.exists()) {
                    banks.emplace_back(*this, Bank::Type::Bank, entry.path().stem().string(), entry);
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
            presetData.name = name;
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
            // TODO: load from factory
        } else {
            database.controller.loadPreset(path);
        }
    }

    // ------------------------------------------------

    PresetDatabase::Bank::Bank(PresetDatabase& d, PresetDatabase::Bank::Type t, std::string n, std::filesystem::path f) 
        : database(d), type(t), name(n), folder(f)
    {}

    // ------------------------------------------------

    void PresetDatabase::Bank::reloadInformation() {
        std::lock_guard _{ m_Mutex };
        m_Presets.clear();
        if (type == Type::Factory) {
            m_Presets.emplace_back(Preset{
                .database = database,
                .bank = *this,
                .type = Preset::Type::Init,
                .name = "Init"
            }).reloadInformation();
            // TODO: load all factory presets
        } else {
            for (auto& entry : std::filesystem::directory_iterator(folder)) {
                if (entry.is_regular_file()) {
                    m_Presets.emplace_back(Preset{
                        .database = database,
                        .bank = *this,
                        .type = Preset::Type::Normal,
                        .path = entry.path()
                    }).reloadInformation();
                }
            }
        }
    }

    // ------------------------------------------------

    const std::vector<PresetDatabase::Bank::Preset>& PresetDatabase::Bank::presets() {
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

}

// ------------------------------------------------
 