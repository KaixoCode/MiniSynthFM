
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
            if (auto result = basic_json::parse(json)) return result.value();
            return {};
        }
    };

    // ------------------------------------------------
    //               Preset::Interface
    // ------------------------------------------------

    bool PresetDatabase::Preset::Interface::exists() {
        return m_Database && m_Database->preset(m_Path, [](auto&) {});
    }
    
    bool PresetDatabase::Preset::Interface::metaDataHasLoaded() {
        if (!m_Database) return false;
        bool hasLoaded = false;
        m_Database->preset(m_Path, [&](Preset& preset) {
            hasLoaded = preset.metaDataLoaded();
        });
        return hasLoaded;
    }
    
    bool PresetDatabase::Preset::Interface::changed() {
        if (!m_Database) return false;
        bool changed = false;
        m_Database->preset(m_Path, [&](Preset& preset) {
            changed = preset.m_ReloadIdentifier != m_LastReloadIdentifier;
        });
        return changed;
    }

    // ------------------------------------------------

    bool PresetDatabase::Preset::Interface::hasType(std::string_view view) {
        if (!m_Database) return false;
        bool match = false;
        m_Database->preset(m_Path, [&](Preset& preset) {
            if (!preset.metaDataLoaded()) return;
            match = preset.type().contains(view);
        });
        return match;
    }

    bool PresetDatabase::Preset::Interface::hasAuthor(std::string_view view) {
        if (!m_Database) return false;
        bool match = false;
        m_Database->preset(m_Path, [&](Preset& preset) {
            if (!preset.metaDataLoaded()) return;
            match = preset.author() == view;
        });
        return match;
    }

    PresetData PresetDatabase::Preset::Interface::presetData() {
        PresetData result{};
        result.name = m_Path.stem().string();

        if (!metaDataHasLoaded()) return result;

        m_Database->preset(m_Path, [&](Preset& preset) { result = preset.m_PresetData; });

        return result;
    }

    void PresetDatabase::Preset::Interface::load() {
        if (!m_Database) return;
        m_Database->preset(m_Path, [&](Preset& preset) { preset.load(); });
    }

    // ------------------------------------------------

    PresetDatabase::Preset::Interface PresetDatabase::Preset::interface() {
        return { m_Database, m_Path };
    }

    // ------------------------------------------------

    PresetDatabase::Preset::Interface::Interface(PresetDatabase& database, std::filesystem::path path) 
        : m_Database(&database), m_Path(path)
    {}

    // ------------------------------------------------
    //                     Preset
    // ------------------------------------------------

    PresetDatabase::Preset::Preset(PresetDatabase& database, Bank& bank)
        : m_Database(database), m_Bank(bank), m_Type(Init), m_Path(InitPresetPath)
    {
        m_PresetData.name = InitPreset;
        reload();
    }

    PresetDatabase::Preset::Preset(PresetDatabase& database, Bank& bank, std::filesystem::path path)
        : m_Database(database), m_Bank(bank), m_Type(Normal), m_Path(path)
    {
        reload();
    }

    PresetDatabase::Preset::Preset(PresetDatabase& database, Bank& bank, std::string_view name)
        : m_Database(database), m_Bank(bank), m_Type(Factory), m_Path(std::format("{}/{}", FactoryBankPath, name))
    {
        reload();
    }

    void PresetDatabase::Preset::load() const {
        if (!metaDataLoaded()) return;

        m_Database.m_LoadedPreset.bank = m_Bank.m_Name;
        m_Database.m_LoadedPreset.preset = m_PresetData.name;

        switch (m_Type) {
        case Init:
            m_Database.controller.initPreset();
            break;
        case Factory: {
            auto json = FactoryPresets::get(m_Path.filename().string());
            m_Database.controller.loadPresetFromJson(json);
            break;
        }
        case Normal:
            m_Database.controller.loadPreset(m_Path);
            break;
        }
    }

    // ------------------------------------------------
    
    void PresetDatabase::Preset::reload() {
        if (!metaDataLoaded()) return; // Already reloading

        m_MetaDataLoaded = false;

        switch (m_Type) {
        case Normal:
            m_MetaDataLoading = std::async(std::launch::async, [this] {
                std::ifstream file{ m_Path };

                if (file.is_open()) {
                    std::string content = file_to_string(file);

                    if (auto json = basic_json::parse(content)) {
                        auto key = typeid(PresetData).name();
                        if (json->contains(key)) m_PresetData.deserialize(json->at(key));
                    }
                }

                m_ReloadIdentifier++;
                m_MetaDataLoaded = true;
            });
            break;
        case Factory:
            m_MetaDataLoading = std::async(std::launch::async, [this, name = m_Path.stem().string()] {
                m_PresetData = FactoryPresets::find(name);
                m_ReloadIdentifier++;
                m_MetaDataLoaded = true;
            });
            break;
        case Init:
            m_MetaDataLoaded = true;
            break;
        }
    }

    // ------------------------------------------------
    //               Bank::Interface
    // ------------------------------------------------

    bool PresetDatabase::Bank::Interface::valid() {
        return m_Database && m_Database->bank(m_Path, [](auto&) {});
    }

    bool PresetDatabase::Bank::Interface::changed() {
        if (!m_Database) return false;
        bool changed = false;
        m_Database->bank(m_Path, [&](Bank& bank) { 
            changed = bank.m_ReloadIdentifier != m_LastReloadIdentifier;
        });
        return changed;
    }

    // ------------------------------------------------

    void PresetDatabase::Bank::Interface::presets(std::function<void(Preset&)> callback) {
        if (!m_Database) return;
        m_Database->bank(m_Path, [&](Bank& bank) { bank.presets(std::move(callback)); });
    }

    // ------------------------------------------------
    
    PresetDatabase::Bank::Interface PresetDatabase::Bank::interface() {
        return { m_Database, m_Path };
    }

    // ------------------------------------------------

    bool PresetDatabase::Bank::preset(std::filesystem::path path, std::function<void(Preset&)> callback) {
        for (auto& preset : m_Presets) if (preset.m_Path == path) return (callback(preset), true);
        return false;
    }

    void PresetDatabase::Bank::presets(std::function<void(Preset&)> callback) {
        for (auto& preset : m_Presets) callback(preset);
    }

    // ------------------------------------------------

    PresetDatabase::Bank::Interface::Interface(PresetDatabase& database, std::filesystem::path path)
        : m_Database(&database), m_Path(path)
    {}

    // ------------------------------------------------
    //                      Bank
    // ------------------------------------------------

    PresetDatabase::Bank::Bank(PresetDatabase& database)
        : m_Database(database), m_Type(Factory), 
          m_Name(FactoryBank), m_Path(FactoryBankPath)
    {
        reload();
    }

    PresetDatabase::Bank::Bank(PresetDatabase& database, std::filesystem::path folder)
        : m_Database(database), m_Type(Folder), 
          m_Name(folder.stem().string()), m_Path(folder)
    {
        reload();
    }

    // ------------------------------------------------

    void PresetDatabase::Bank::reload() {
        for (auto& preset : m_Presets) preset.m_Exists = false; 

        switch (m_Type) {
        case Factory: {
            // Reload or add init preset
            if (!preset(InitPresetPath, [](Preset& preset) {
                preset.reload();
                preset.m_Exists = true;
            })) {
                m_Presets.emplace_back(m_Database, *this);
            }

            // Reload or add all factory presets
            for (auto& [name, json] : Presets) {
                if (!preset(std::format("{}/{}", FactoryBankPath, name), [](Preset& preset) {
                    preset.reload();
                    preset.m_Exists = true;
                })) {
                    m_Presets.emplace_back(m_Database, *this, name);
                }
            }
            break;
        }
        case Folder: {
            std::vector<std::filesystem::path> files;
            for (auto& file : std::filesystem::directory_iterator(m_Path)) {
                if (file.exists() && file.is_regular_file() &&
                    file.path().extension() == ".minifm") 
                {
                    files.push_back(file.path());
                }
            }
            std::ranges::sort(files);

            // Reload or add all presets in the folder
            for (auto& path : files) {
                if (!preset(path, [](Preset& preset) {
                    preset.reload();
                    preset.m_Exists = true;
                })) {
                    m_Presets.emplace_back(m_Database, *this, path);
                }
            }
            break;
        }
        }

        // Erase all nonvalid presets
        for (auto it = m_Presets.begin(); it != m_Presets.end();) {
            if (!it->exists()) it = m_Presets.erase(it);
            else ++it;
        }

        m_ReloadIdentifier++;
    }

    // ------------------------------------------------
    //                PresetDatabase
    // ------------------------------------------------

    PresetDatabase::PresetDatabase(Controller& c)
        : controller(c)
    {}

    // ------------------------------------------------

    bool PresetDatabase::preset(std::filesystem::path path, std::function<void(Preset&)> callback) {
        bool found = false;
        bank(path.parent_path(), [&](Bank& bank) {
            found = bank.preset(path, std::move(callback));
        });

        return found;
    }
    
    bool PresetDatabase::bank(std::filesystem::path path, std::function<void(Bank&)> callback) {
        for (auto& bank : m_PresetBanks) {
            if (bank.m_Path == path) {
                callback(bank);
                return true;
            }
        }

        return false;
    }

    void PresetDatabase::banks(std::function<void(Bank&)> callback) {
        for (auto& bank : m_PresetBanks) {
            callback(bank);
        }
    }
    
    void PresetDatabase::presets(std::function<void(Preset&)> callback) {
        banks([&](PresetDatabase::Bank& bank) {
            bank.presets(callback);
        });
    }

    // ------------------------------------------------
    
    void PresetDatabase::load(std::filesystem::path path) {
        bank(path.parent_path(), [&](Bank& bank) {
            bank.preset(path, [](Preset& preset) {
                preset.load();
            });
        });
    }

    // ------------------------------------------------
    
    void PresetDatabase::reload() {
        auto presetPathFromStorage = Storage::get<std::string>(PresetPath);
        if (!presetPathFromStorage) return;
        
        std::filesystem::path presetPath = presetPathFromStorage.value();
        if (!std::filesystem::exists(presetPath)) return;

        for (auto& bank : m_PresetBanks) bank.m_Exists = false;

        // Reload or add factory bank
        if (!bank(FactoryBankPath, [](Bank& bank) {
            bank.reload();
            bank.m_Exists = true;
        })) {
            m_PresetBanks.emplace_back(*this);
        }
        
        // Reload or add User bank (root path of preset directory)
        if (!bank(presetPath, [](Bank& bank) {
            bank.reload();
            bank.m_Exists = true;
        })) {
            m_PresetBanks.emplace_back(*this, presetPath);
        }

        // Reload or add all banks in presetPath
        for (auto& entry : std::filesystem::directory_iterator(presetPath)) {
            if (entry.is_directory() && entry.exists()) {
                if (!bank(entry.path(), [](Bank& bank) {
                    bank.reload();
                    bank.m_Exists = true;
                })) {
                    m_PresetBanks.emplace_back(*this, entry.path());
                }
            }
        }

        // Erase all nonvalid banks
        for (auto it = m_PresetBanks.begin(); it != m_PresetBanks.end();) {
            if (!it->exists()) it = m_PresetBanks.erase(it);
            else ++it;
        }
    }

    // ------------------------------------------------

    void PresetDatabase::loadNextPreset() {
        for (auto bank = m_PresetBanks.begin(); bank != m_PresetBanks.end(); ++bank) {
            if (bank->m_Path != m_LoadedPreset.bank) continue;

            for (auto preset = bank->m_Presets.begin(); preset != bank->m_Presets.end(); ++preset) {
                if (preset->m_Path != m_LoadedPreset.preset) continue;
                if (++preset != bank->m_Presets.end()) return preset->load();
                
                while (true) {
                    if (++bank == m_PresetBanks.end()) return load(InitPresetPath);
                    if (!bank->m_Presets.empty()) return bank->m_Presets.front().load();
                }
            }
        }
    }

    void PresetDatabase::loadPreviousPreset() {
        for (auto bank = m_PresetBanks.begin(); bank != m_PresetBanks.end(); ++bank) {
            if (bank->m_Path != m_LoadedPreset.bank) continue;

            for (auto preset = bank->m_Presets.begin(); preset != bank->m_Presets.end(); ++preset) {
                if (preset->m_Path != m_LoadedPreset.preset) continue;
                if (preset != bank->m_Presets.begin()) return (--preset)->load();

                while (true) {
                    if (bank == m_PresetBanks.begin()) bank = --m_PresetBanks.end();
                    else --bank;
                    if (!bank->m_Presets.empty()) return bank->m_Presets.back().load();
                }
            }
        }
    }

    // ------------------------------------------------

}

// ------------------------------------------------
 