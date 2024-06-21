
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
        
        constexpr static std::string_view FactoryBank = "Factory";
        constexpr static std::string_view FactoryBankPath = "Factory:/";
        constexpr static std::string_view InitPreset = "Init";
        constexpr static std::string_view InitPresetPath = "Factory:/Init";

        // ------------------------------------------------

        Controller& controller;

        // ------------------------------------------------

        PresetDatabase(Controller& controller);

        // ------------------------------------------------

        class Bank;
        class Preset {
        public:

            // ------------------------------------------------

            class Interface {
            public:

                // ------------------------------------------------

                bool exists();
                bool metaDataHasLoaded();
                bool changed();

                // ------------------------------------------------

            private:
                PresetDatabase& m_Database;
                std::filesystem::path m_Path;
                std::size_t m_LastReloadIdentifier = 0;

                // ------------------------------------------------

            };

            // ------------------------------------------------

            Preset(PresetDatabase& database, Bank& bank);
            Preset(PresetDatabase& database, Bank& bank, std::filesystem::path path);
            Preset(PresetDatabase& database, Bank& bank, std::string_view name);

            // ------------------------------------------------

            // It is undefined behaviour to call any of these functions while metaDataLoaded() == false
            std::string_view name() const { return m_PresetData.name; }
            std::string_view author() const { return m_PresetData.author; }
            std::string_view description() const { return m_PresetData.description; }
            std::string_view type() const { return m_PresetData.type; }

            bool metaDataLoaded() const { return m_MetaDataLoaded; }
            bool exists() const { return m_Exists; }

            // ------------------------------------------------

            void load() const;

            // ------------------------------------------------

        private:
            PresetDatabase& m_Database;
            Bank& m_Bank;
            std::filesystem::path m_Path{};
            PresetData m_PresetData{};
            std::future<void> m_MetaDataLoading{};
            std::atomic_bool m_MetaDataLoaded = true;
            std::size_t m_ReloadIdentifier = 0;
            bool m_Exists = true; // When false, preset no longer exists on disk. Set during reload.
            enum { Init, Factory, Normal } m_Type = Init;

            // ------------------------------------------------
            
            void reload();

            // ------------------------------------------------

            friend class PresetDatabase;

            // ------------------------------------------------

        };

        // ------------------------------------------------

        class Bank {
        public:

            // ------------------------------------------------
            
            class Interface {
            public:

                // ------------------------------------------------

                bool valid();
                bool changed();

                // ------------------------------------------------

            private:
                PresetDatabase& m_Database;
                std::filesystem::path m_Path;
                std::size_t m_LastReloadIdentifier = 0;

                // ------------------------------------------------

            };

            // ------------------------------------------------
            
            Bank(PresetDatabase& database);
            Bank(PresetDatabase& database, std::filesystem::path folder);

            // ------------------------------------------------

            std::string_view name() const { return m_Name; }
            std::size_t nofPresets() const { return m_Presets.size(); }
            bool exists() const { return m_Exists; }

            // ------------------------------------------------

            bool preset(std::filesystem::path path, std::function<void(Preset&)> callback);

            // ------------------------------------------------

        private:
            PresetDatabase& m_Database;
            std::string m_Name;
            std::filesystem::path m_Path{};
            std::list<Preset> m_Presets{};
            std::size_t m_ReloadIdentifier = 0;
            bool m_Exists = true; // When false, bank no longer exists on disk. Set during reload.
            enum { Factory, Folder } m_Type = Factory;

            // ------------------------------------------------
            
            void reload();

            // ------------------------------------------------
            
            friend class PresetDatabase;

            // ------------------------------------------------

        };

        // ------------------------------------------------

        bool preset(std::filesystem::path path, std::function<void(Preset&)> callback);
        bool bank(std::filesystem::path path, std::function<void(Bank&)> callback);

        // ------------------------------------------------
        
        void load(std::filesystem::path path);

        // ------------------------------------------------

        void loadNextPreset();
        void loadPreviousPreset();

        // ------------------------------------------------
        
        void reload();

        // ------------------------------------------------

    private:
        std::list<Bank> m_PresetBanks{};

        struct LoadedPreset {
            std::filesystem::path bank{ FactoryBank };
            std::filesystem::path preset{ InitPreset };
        } m_LoadedPreset{};

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------
