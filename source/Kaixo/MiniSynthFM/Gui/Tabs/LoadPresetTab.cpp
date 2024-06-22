
// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Gui/Tabs/LoadPresetTab.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------
    
    std::vector<std::string_view> split_types(std::string_view type) {
        auto res = split(type, ",");
        for (auto& r : res) {
            r = trim(r);
        }
        return res;
    }

    // ------------------------------------------------
    
    bool matches_search(const auto& haystack, const auto& needle) {
        constexpr auto tolower = [](char c) -> char { return std::tolower(c); };
        constexpr auto equals = [](char a, char b) { return a == b; };
        if (needle.empty()) return true;
        bool matches = true;
        auto parts = split(needle, " ");
        for (auto& part : parts) {
            if (part.empty()) continue;
            auto res = std::ranges::search(haystack, part, equals, tolower, tolower);
            matches &= !res.empty();
        }
        return matches;
    }

    // ------------------------------------------------
    //                    Filter
    // ------------------------------------------------

    LoadPresetTab::Filter::Filter(Context c, Settings s)
        : View(c), settings(std::move(s))
    {
        graphics = T.display.loadPreset.bank;
        animation(graphics);
    }

    // ------------------------------------------------

    void LoadPresetTab::Filter::paint(juce::Graphics& g) {
        auto& database = context.controller<MiniSynthFMController>().presetDatabase;
        auto name = settings.name;
        if (name.empty()) name = "Other";

        graphics.draw({
            .graphics = g,
            .bounds = localDimensions(),
            .text = { { "$name", name } },
            .state = state()
        });
    }

    // ------------------------------------------------
    //                    Preset
    // ------------------------------------------------

    LoadPresetTab::Preset::Preset(Context c, Settings s)
        : View(c), settings(std::move(s))
    {
        setWantsKeyboardFocus(true);
        graphics = T.display.loadPreset.preset;
        wantsIdle(true);
        animation(graphics);
        reloadDisplayName();
    }

    // ------------------------------------------------

    void LoadPresetTab::Preset::paint(juce::Graphics& g) {
        graphics.draw({
            .graphics = g,
            .bounds = localDimensions(),
            .text = { { "$name", displayName } },
            .state = state()
        });
    }

    // ------------------------------------------------

    void LoadPresetTab::Preset::onIdle() {
        settings.preset.onChanged([&](PresetDatabase::Preset&) {
            reloadDisplayName();
        });
    }

    // ------------------------------------------------

    void LoadPresetTab::Preset::reloadDisplayName() {
        settings.preset.access([&](PresetDatabase::Preset& preset) {
            displayName = preset.name();
            switch (displayNameType) {
            case FilterType::Bank:
                if (!preset.type().empty()) {
                    displayName += std::format(" ({})", preset.type());
                }
                break;
            case FilterType::Author:
                if (!preset.type().empty()) {
                    displayName += std::format(" ({})", preset.type());
                }
                [[fallthrough]];
            case FilterType::Type:
                displayName += std::format(" - {}", preset.bank().name());
                break;
            }
            repaint();
        });
    }

    bool LoadPresetTab::Preset::matchesSearch(std::string_view search) {
        bool matches = false;
        settings.preset.access([&](PresetDatabase::Preset& preset) {
            matches = matches_search(std::format("{} {} {}", preset.type(), preset.name(), preset.author()), search);
        });
        return matches;
    }

    // ------------------------------------------------
    //                 LoadPresetTab
    // ------------------------------------------------

    LoadPresetTab::LoadPresetTab(Context c, Settings s)
        : View(c), settings(s)
    {
        // ------------------------------------------------
        
        wantsIdle(true);

        // ------------------------------------------------

        add<ImageView>({ .image = T.display.loadPreset.background });

        // ------------------------------------------------

        m_Filters = &add<ScrollView>({ 6, 28, 100, Height - 34 }, {
            .scrollbar = T.display.loadPreset.scrollbar,
            .margin = { 0, 0, 0, 0 },
            .gap = 2,
            .barThickness = 5,
            .barPadding = { 2, 0, 0, 0 },
            .keepBarSpace = false,
            .alignChildren = Theme::Align::Left
        });

        m_Search = &add<TextView>({ 113, 6, 137, 20 }, {
            .graphics = T.display.loadPreset.search,
            .padding = { 4, 3 },
            .multiline = false,
            .editable = true,
            .lineHeight = 14,
            .maxSize = 24,
            .placeholder = "Search..."
        });

        m_Search->addCallback([this](std::string_view) { showFilteredPresets(); });

        m_Presets = &add<ScrollView>({ 113, 28, 193, Height - 34 }, {
            .scrollbar = T.display.loadPreset.scrollbar,
            .margin = { 0, 0, 0, 0 },
            .gap = 2,
            .barThickness = 5,
            .barPadding = { 2, 0, 0, 0 },
            .keepBarSpace = false,
            .alignChildren = Theme::Align::Left
        });

        m_FilterTabs.addButton(0, add<Button>({ 6, 6, 32, 20 }, {
            .graphics = T.display.loadPreset.bankTab
        }));
        
        m_FilterTabs.addButton(1, add<Button>({ 40, 6, 32, 20 }, {
            .graphics = T.display.loadPreset.typeTab
        }));
        
        m_FilterTabs.addButton(2, add<Button>({ 74, 6, 32, 20 }, {
            .graphics = T.display.loadPreset.authorTab
        }));

        m_FilterTabs.tab(0).addCallback([&](bool v) { if (v) createFilters(FilterType::Bank); });
        m_FilterTabs.tab(1).addCallback([&](bool v) { if (v) createFilters(FilterType::Type); });
        m_FilterTabs.tab(2).addCallback([&](bool v) { if (v) createFilters(FilterType::Author); });

        m_SortButton = &add<Button>({ 252, 6, 32, 20 }, {
            .callback = [&](bool v) { sortPresets(v); },
            .graphics = T.display.loadPreset.sortButton,
            .behaviour = Button::Behaviour::Toggle
        });
        
        m_ReloadButton = &add<Button>({ 286, 6, 20, 20 }, {
            .callback = [&](bool v) { reload(); },
            .graphics = T.display.loadPreset.reloadButton,
            .behaviour = Button::Behaviour::Click
        });

        m_FilterTabs.select(0);

        // ------------------------------------------------

        add<ImageView>({ .image = T.display.loadPreset.foreground, .enableMouse = false });

        // ------------------------------------------------
        
        reload();

        // ------------------------------------------------

    }

    // ------------------------------------------------
    
    void LoadPresetTab::onIdle() {
        if (m_Reloaded) {
            if (context.controller<MiniSynthFMController>().presetDatabase.allLoaded()) {
                createAll();
                m_Reloaded = false;
                loadState();
            }
        }
    }

    // ------------------------------------------------

    void LoadPresetTab::select(Filter& filter) {
        if (filter.selected()) return; // already selected

        m_Search->settings.text = "";
        m_Search->repaint();

        for (auto& b : m_Filters->views()) {
            b->selected(false);
            b->repaint();
        }
        filter.selected(true);

        m_CurrentFilter = filter.settings.match;

        showFilteredPresets();
    }

    // ------------------------------------------------

    void LoadPresetTab::reload() {
        if (m_Reloaded) return; // Already reloading
        saveState();
        m_Reloaded = true;
        context.controller<MiniSynthFMController>().presetDatabase.reload();
        m_Filters->clear();
        m_Presets->clear();
    }

    // ------------------------------------------------

    void LoadPresetTab::createAll() {
        createFilters(m_CurrentType);
        createPresets();

        showFilteredPresets();
    }

    void LoadPresetTab::createFilters(FilterType type) {
        m_CurrentType = type;
        m_Filters->clear();

        auto& database = context.controller<MiniSynthFMController>().presetDatabase;

        auto basicFilter = [&](auto addEntries, auto match) {
            std::set<std::string_view> entries{};
            database.presets([&](PresetDatabase::Preset& preset) {
                if (!preset.metaDataLoaded()) return; // Not yet loaded!
                addEntries(entries, preset);
            });

            std::vector<std::string_view> sortedEntries{ entries.begin(), entries.end() };
            std::ranges::sort(sortedEntries);

            for (auto& entry : sortedEntries) {
                m_Filters->add<Filter>({ Width, 20 }, Filter::Settings{
                    .self = *this,
                    .match = [entry = std::string(entry), match](Preset& preset) -> bool {
                        bool matches = false;
                        preset.settings.preset.access([&](PresetDatabase::Preset& preset) {
                            matches = match(entry, preset);
                        });
                        return matches;
                    },
                    .name = std::string{ entry }
                });
            }
        };

        switch (type) {
        case FilterType::Bank: {
            m_Filters->add<Filter>({ Width, 20 }, Filter::Settings{
                .self = *this,
                .match = [](Preset& preset) -> bool { return true; },
                .name = "All"
            });

            database.banks([&](PresetDatabase::Bank& bank) {
                m_Filters->add<Filter>({ Width, 20 }, Filter::Settings{
                    .self = *this,
                    .match = [path = bank.path()](Preset& preset) -> bool {
                        return path == preset.settings.preset.bankPath();
                    },
                    .name = bank.name()
                });
            });
            break;
        }
        case FilterType::Type: {
            basicFilter([&](std::set<std::string_view>& entries, PresetDatabase::Preset& preset) {
                auto presetTypes = split_types(preset.type());
                for (auto& type : presetTypes) entries.insert(type);
            }, [&](std::string_view entry, PresetDatabase::Preset& preset) {
                return preset.type().contains(entry);
            });
            break;
        }       
        case FilterType::Author: {
            basicFilter([&](std::set<std::string_view>& entries, PresetDatabase::Preset& preset) {
                entries.insert(preset.author());
            }, [&](std::string_view entry, PresetDatabase::Preset& preset) {
                    return preset.author() == entry;
            });
            break;
        }
        }

        if (!m_Filters->views().empty()) {
            select((Filter&)*m_Filters->views().front());
        }

        m_Filters->updateDimensions();
    }

    void LoadPresetTab::createPresets() {
        m_Presets->clear();
        context.controller<MiniSynthFMController>().presetDatabase.presets([&](PresetDatabase::Preset& preset) {
            m_Presets->add<Preset>({ Width, 20 }, {
                .self = *this,
                .preset = preset.interface(),
                .isInit = preset.isInit()
            });
        });
        m_Presets->updateDimensions();
    }

    // ------------------------------------------------

    void LoadPresetTab::saveState() {
        m_State.search = m_Search->settings.text;
        m_State.scrolled = m_Presets->scrolled();
        m_State.type = m_CurrentType;
        for (auto& b : m_Filters->views()) {
            if (!b->selected()) continue;
            if (auto bank = dynamic_cast<Filter*>(b.get())) {
                m_State.value = bank->settings.name;
                return;
            }
        }

        m_State.type = FilterType::Bank;
        m_State.value = "Factory";
    }

    void LoadPresetTab::loadState() {
        for (auto& b : m_Filters->views()) {
            if (auto bank = dynamic_cast<Filter*>(b.get())) {
                auto& database = context.controller<MiniSynthFMController>().presetDatabase;
                if (m_State.value == bank->settings.name) {
                    select(*bank);
                    break;
                }
            }
        }

        m_Search->settings.text = m_State.search;
        m_Search->repaint();
        showFilteredPresets();

        m_Presets->scrollTo(m_State.scrolled);
    }

    // ------------------------------------------------
    
    void LoadPresetTab::sortPresets(bool reverse) {
        std::ranges::sort(m_Presets->views(), [](std::unique_ptr<View>& a, std::unique_ptr<View>& b) {
            Preset& f1 = dynamic_cast<Preset&>(*a);
            Preset& f2 = dynamic_cast<Preset&>(*b);
            if (f1.settings.isInit) return true;
            if (f2.settings.isInit) return false;
            return f1.displayName < f2.displayName;
        });

        if (reverse) std::ranges::reverse(m_Presets->views());

        m_Presets->updateDimensions();
    }

    void LoadPresetTab::showFilteredPresets() {
        for (auto& view : m_Presets->views()) {
            Preset& preset = dynamic_cast<Preset&>(*view);
            preset.settings.preset;
            bool matchesFilter = !m_CurrentFilter || m_CurrentFilter(preset);
            bool matchesSearch = preset.matchesSearch(m_Search->content());
            preset.setVisible(matchesFilter && matchesSearch);
            preset.displayNameType = m_CurrentType;
            preset.reloadDisplayName();
        }

        sortPresets(m_SortButton->selected());

        m_Presets->scrollTo(0);
    }

    // ------------------------------------------------

}

// ------------------------------------------------
