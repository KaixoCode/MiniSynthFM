
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

    LoadPresetTab::Filter::Filter(Context c, Settings s)
        : View(c), settings(std::move(s))
    {
        graphics = T.display.loadPreset.bank;
        animation(graphics);
    }

    // ------------------------------------------------

    void LoadPresetTab::Filter::mouseDown(const juce::MouseEvent& e) {
        settings.self.select(*this);
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

    LoadPresetTab::Preset::Preset(Context c, Settings s)
        : View(c), settings(std::move(s))
    {
        setWantsKeyboardFocus(true);
        graphics = T.display.loadPreset.preset;
        reloadDisplayName();
        animation(graphics);
    }

    // ------------------------------------------------

    void LoadPresetTab::Preset::paint(juce::Graphics& g) {
        reloadDisplayName();
        graphics.draw({
            .graphics = g,
            .bounds = localDimensions(),
            .text = { { "$name", displayName } },
            .state = state()
        });
    }

    // ------------------------------------------------

    void LoadPresetTab::Preset::reloadDisplayName() {
        auto pdata = settings.preset.presetData();
        displayName = pdata.name;
        if (!pdata.type.empty()) displayName += " (" + pdata.type + ")";
    }

    // ------------------------------------------------

    bool LoadPresetTab::Preset::matchesSearch(std::string_view search) {
        auto pdata = settings.preset.presetData();
        return matches_search(pdata.type + " " + pdata.name + " " + pdata.author, search);
    }

    // ------------------------------------------------

    LoadPresetTab::LoadPresetTab(Context c, Settings s)
        : View(c), settings(s)
    {
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

        m_Search = &add<TextView>({ 113, 6, 159, 20 }, {
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

        m_FilterTabs.tab(0).addCallback([&](bool v) { if (v) reloadFilters(FilterType::Bank); });
        m_FilterTabs.tab(1).addCallback([&](bool v) { if (v) reloadFilters(FilterType::Type); });
        m_FilterTabs.tab(2).addCallback([&](bool v) { if (v) reloadFilters(FilterType::Author); });

        m_SortButton = &add<Button>({ 274, 6, 32, 20 }, {
            .callback = [&](bool) { sortPresets(); },
            .graphics = T.display.loadPreset.sortButton,
            .behaviour = Button::Behaviour::Toggle
        });

        m_FilterTabs.select(0);

        // ------------------------------------------------

        add<ImageView>({ .image = T.display.loadPreset.foreground, .enableMouse = false });

        // ------------------------------------------------

        reload();

        // ------------------------------------------------

    }

    // ------------------------------------------------

    void LoadPresetTab::reloadFilters(FilterType type) {
        bool loadingSameType = m_CurrentType == type;
        m_CurrentType = type;
        if (loadingSameType) saveState(); // Save current state

        m_Search->settings.text = "";
        m_Search->repaint();
        m_Filters->clear();

        auto& database = context.controller<MiniSynthFMController>().presetDatabase;

        switch (type) {
        case FilterType::Bank: {
            database.banks([&](PresetDatabase::Bank& bank) {
                m_Filters->add<Filter>({ Width, 20 }, Filter::Settings{
                    .self = *this,
                    .match = [interface = bank.interface()](Preset& preset) mutable -> bool {
                        bool match = false;
                        interface.presets([&](PresetDatabase::Preset& other) {
                            match = other.path() == preset.settings.preset.path();
                        });
                        return match;
                    },
                    .name = bank.name()
                });
            });
            break;
        }
        case FilterType::Type: {
            std::set<std::string_view> types;
            database.presets([&](PresetDatabase::Preset& preset) {
                if (!preset.metaDataLoaded()) return; // Not yet loaded!

                auto presetTypes = split_types(preset.type());
                for (auto& type : presetTypes) types.insert(type);
            });

            std::vector<std::string_view> sortedTypes{ types.begin(), types.end() };
            std::ranges::sort(sortedTypes);

            for (auto& type : sortedTypes) {
                m_Filters->add<Filter>({ Width, 20 }, Filter::Settings{
                    .self = *this,
                    .match = [type = std::string(type)](Preset& preset) -> bool { 
                        return preset.settings.preset.hasType(type); 
                    },
                    .name = std::string{ type }
                });
            }

            break;
        }       
        case FilterType::Author: {
            std::set<std::string_view> authors;
            
            database.banks([&](PresetDatabase::Bank& bank) {
                bank.presets([&](PresetDatabase::Preset& preset) {
                    if (!preset.metaDataLoaded()) return; // Not yet loaded!

                    authors.insert(preset.author());
                });
            });

            std::vector<std::string_view> sortedAuthors{ authors.begin(), authors.end() };
            std::ranges::sort(sortedAuthors);

            for (auto& author : sortedAuthors) {
                m_Filters->add<Filter>({ Width, 20 }, Filter::Settings{
                    .self = *this,
                    .match = [author = std::string(author)](Preset& preset) -> bool {
                        return preset.settings.preset.hasAuthor(author);
                    },
                    .name = std::string{ author }
                });
            }

            break;
        }
        }

        database.reload();

        if (!m_Filters->views().empty()) {
            select((Filter&)*m_Filters->views().front());
        }

        m_Filters->updateDimensions();
        if (loadingSameType) loadState(); // Load back state
    }

    void LoadPresetTab::reloadPresets() {
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

    void LoadPresetTab::reload() {
        reloadFilters(m_CurrentType);
        reloadPresets();
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
    
    void LoadPresetTab::sortPresets() {
        std::ranges::sort(m_Presets->views(), [](std::unique_ptr<View>& a, std::unique_ptr<View>& b) {
            Preset& f1 = dynamic_cast<Preset&>(*a);
            Preset& f2 = dynamic_cast<Preset&>(*b);
            if (f1.settings.isInit) return true;
            if (f2.settings.isInit) return false;
            return f1.displayName < f2.displayName;
        });

        if (m_SortButton->selected()) std::ranges::reverse(m_Presets->views());
    }

    void LoadPresetTab::showFilteredPresets() {
        for (auto& view : m_Presets->views()) {
            Preset& preset = dynamic_cast<Preset&>(*view);
            preset.settings.preset;
            bool matchesFilter = !m_CurrentFilter || m_CurrentFilter(preset);
            bool matchesSearch = preset.matchesSearch(m_Search->content());
            preset.setVisible(matchesFilter && matchesSearch);
        }

        sortPresets();

        m_Presets->updateDimensions();
    }

    // ------------------------------------------------

}

// ------------------------------------------------
