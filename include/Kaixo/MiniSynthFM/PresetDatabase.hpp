
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
        constexpr static std::string_view FactoryBankPath = "Factory:";
        constexpr static std::string_view InitPreset = "Init";
        constexpr static std::string_view InitPresetPath = "Factory:/Init";

        // ------------------------------------------------

        class Bank;
        class Preset {
        public:

            // ------------------------------------------------

            class Interface {
            public:

                // ------------------------------------------------

                Interface() = default;

                // ------------------------------------------------

                bool exists();
                bool metaDataHasLoaded();
                bool changed();

                bool onChanged(std::function<void(Preset&)> callback);
                bool access(std::function<void(Preset&)> callback);

                // ------------------------------------------------
                
                void load();

                // ------------------------------------------------

                std::filesystem::path path() const { return m_Path; }
                std::filesystem::path bankPath() const { return m_Path.parent_path(); }

                // ------------------------------------------------

            private:
                PresetDatabase* m_Database;
                std::filesystem::path m_Path;
                std::size_t m_LastReloadIdentifier = 0;

                // ------------------------------------------------
                
                Interface(PresetDatabase& database, std::filesystem::path path);

                // ------------------------------------------------

                friend class Preset;

                // ------------------------------------------------

            };

            // ------------------------------------------------

            Preset(PresetDatabase& database, Bank& bank);
            Preset(PresetDatabase& database, Bank& bank, std::filesystem::path path);
            Preset(PresetDatabase& database, Bank& bank, std::string_view name);

            // ------------------------------------------------

            std::string_view type() const { return m_PresetData.type; }
            std::string_view author() const { return m_PresetData.author; }
            std::string_view name() const { return m_PresetData.name; }
            std::string_view description() const { return m_PresetData.description; }

            std::filesystem::path path() const { return m_Path; }

            bool metaDataLoaded() const { return m_MetaDataLoaded; }
            bool exists() const { return m_Exists; }
            bool isInit() const { return m_Type == Init; }

            // ------------------------------------------------

            void load() const;

            // ------------------------------------------------

            Interface interface();

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
            
            Bank(PresetDatabase& database);
            Bank(PresetDatabase& database, std::filesystem::path folder, bool user = false);

            // ------------------------------------------------

            std::string name() const { return m_Name; }
            std::filesystem::path path() const { return m_Path; }

            bool exists() const { return m_Exists; }
            bool isFactory() const { return m_Type == Factory; }
            bool isUser() const { return m_Type == User; }

            // ------------------------------------------------

            bool preset(std::filesystem::path path, std::function<void(Preset&)> callback);
            void presets(std::function<void(Preset&)> callback);

            // ------------------------------------------------

        private:
            PresetDatabase& m_Database;
            std::string m_Name;
            std::filesystem::path m_Path{};
            std::list<Preset> m_Presets{};
            std::size_t m_ReloadIdentifier = 0;
            bool m_Exists = true; // When false, bank no longer exists on disk. Set during reload.
            enum { Factory, User, Folder } m_Type = Factory;

            // ------------------------------------------------
            
            void reload();

            // ------------------------------------------------
            
            friend class PresetDatabase;

            // ------------------------------------------------

        };

        // ------------------------------------------------

        Controller& controller;

        // ------------------------------------------------

        PresetDatabase(Controller& controller);

        // ------------------------------------------------

        bool bank(std::filesystem::path path, std::function<void(Bank&)> callback);
        void banks(std::function<void(Bank&)> callback);
        bool preset(std::filesystem::path path, std::function<void(Preset&)> callback);
        void presets(std::function<void(Preset&)> callback);

        // ------------------------------------------------
        
        void load(std::filesystem::path path);

        // ------------------------------------------------

        void loadNextPreset();
        void loadPreviousPreset();

        // ------------------------------------------------
        
        void reload();

        bool allLoaded();

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
