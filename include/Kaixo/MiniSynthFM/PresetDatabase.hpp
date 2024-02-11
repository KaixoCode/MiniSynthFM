
// ------------------------------------------------

#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Storage.hpp"
#include "Kaixo/Core/Controller.hpp"

// ------------------------------------------------

#include "Kaixo/MiniSynthFM/PresetData.hpp"

// ------------------------------------------------

namespace Kaixo {

    // ------------------------------------------------

    class Controller;

    // ------------------------------------------------

    class PresetDatabase {
    public:

        // ------------------------------------------------

        Controller& controller;

        // ------------------------------------------------

        PresetDatabase(Controller& controller);
        ~PresetDatabase();

        // ------------------------------------------------

        void reloadInformation();

        // ------------------------------------------------

        struct LoadedPreset {
            std::string bank{};
            std::string name{};
        } loadedPreset{};

        // ------------------------------------------------

        struct Bank {

            // ------------------------------------------------

            struct Preset {

                // ------------------------------------------------

                PresetDatabase& database;
                Bank& bank;

                // ------------------------------------------------
                
                std::size_t id;

                // ------------------------------------------------

                enum class Type { Init, Factory, Normal } type = Type::Init;

                // ------------------------------------------------

                std::string name{};
                std::filesystem::path path{};

                // ------------------------------------------------

                PresetData presetData;

                // ------------------------------------------------

                void reloadInformation();

                // ------------------------------------------------

                void load() const;

                // ------------------------------------------------

            };

            // ------------------------------------------------

            PresetDatabase& database;

            // ------------------------------------------------

            enum class Type { Factory, Bank } type = Type::Factory;

            // ------------------------------------------------

            std::string name{};
            std::filesystem::path folder{};

            std::size_t id;

            // ------------------------------------------------

            Bank(PresetDatabase& d, Type t, std::size_t id, std::string n = {}, std::filesystem::path f = {});

            // ------------------------------------------------

            void reloadInformation();

            // ------------------------------------------------
            
            const std::vector<Preset>& presets() const;

            // ------------------------------------------------

        private:
            mutable std::mutex m_Mutex{};
            std::atomic_bool m_Loaded = false;
            std::vector<Preset> m_Presets{};

            // ------------------------------------------------

        };

        std::deque<Bank> banks{};

        // ------------------------------------------------

        void loadNextPreset();
        void loadPreviousPreset();

        // ------------------------------------------------
        
        const Bank::Preset* preset(std::size_t id);
        const Bank* bank(std::size_t id);

        // ------------------------------------------------
        
    private:
        std::thread m_LoadBanksThread{};
        std::size_t m_PresetIdCounter{};
        std::size_t m_BankIdCounter{};

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------
