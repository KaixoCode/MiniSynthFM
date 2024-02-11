
// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Gui/Tabs/LoadPresetTab.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

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
        auto name = identifier();
        if (name.empty()) name = "Other";

        graphics.draw({
            .graphics = g,
            .bounds = localDimensions(),
            .text = name,
            .state = state()
        });
    }

    // ------------------------------------------------

    void LoadPresetTab::Filter::foreach(std::function<void(const PresetDatabase::Bank::Preset&)> callback) {
        auto& database = context.controller<MiniSynthFMController>().presetDatabase;
        switch (settings.type) {
        case Type::Bank:
            if (auto bank = database.bank(settings.bankId)) {
                for (auto& preset : bank->presets()) {
                    callback(preset);
                }
            }
            break;
        case Type::Type:
            for (auto& bank : database.banks) {
                for (auto& preset : bank.presets()) {
                    if (preset.presetData.type == settings.value) {
                        callback(preset);
                    }
                }
            }
            break;        
        case Type::Author:
            for (auto& bank : database.banks) {
                for (auto& preset : bank.presets()) {
                    if (preset.presetData.author == settings.value) {
                        callback(preset);
                    }
                }
            }
            break;
        }
    }

    // ------------------------------------------------

    std::string LoadPresetTab::Filter::identifier() { return settings.value; }

    // ------------------------------------------------

    LoadPresetTab::Preset::Preset(Context c, Settings s)
        : View(c), settings(std::move(s))
    {
        graphics = T.display.loadPreset.preset;
        reloadDisplayName();
        animation(graphics);
    }

    // ------------------------------------------------

    void LoadPresetTab::Preset::mouseDown(const juce::MouseEvent& e) {
        load();
    }

    // ------------------------------------------------

    void LoadPresetTab::Preset::paint(juce::Graphics& g) {
        reloadDisplayName();
        graphics.draw({
            .graphics = g,
            .bounds = localDimensions(),
            .text = displayName,
            .state = state()
        });
    }

    // ------------------------------------------------

    PresetData LoadPresetTab::Preset::presetData() {
        auto& database = context.controller<MiniSynthFMController>().presetDatabase;
        if (auto preset = database.preset(settings.presetId)) return preset->presetData;
        return {};
    }

    void LoadPresetTab::Preset::load() {
        auto& database = context.controller<MiniSynthFMController>().presetDatabase;
        if (auto preset = database.preset(settings.presetId)) return preset->load();
    }

    // ------------------------------------------------

    void LoadPresetTab::Preset::reloadDisplayName() {
        auto pdata = presetData();
        displayName = pdata.name;
        if (!pdata.type.empty()) displayName += " (" + pdata.type + ")";
    }

    // ------------------------------------------------

    bool LoadPresetTab::Preset::matchesSearch(std::string_view search) {
        auto pdata = presetData();
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

        m_Search = &add<TextView>({ 113, 6, 193, 20 }, {
            .graphics = T.display.loadPreset.search,
            .padding = { 4, 3 },
            .multiline = false,
            .editable = true,
            .lineHeight = 14,
            .maxSize = 24,
            .placeholder = "Search..."
        });

        m_Search->addCallback([this](std::string_view content) {
            applySearch();
        });

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

        m_FilterTabs.tab(0).addCallback([&](bool v) { if (v) reloadFilters(Filter::Type::Bank); });
        m_FilterTabs.tab(1).addCallback([&](bool v) { if (v) reloadFilters(Filter::Type::Type); });
        m_FilterTabs.tab(2).addCallback([&](bool v) { if (v) reloadFilters(Filter::Type::Author); });

        m_FilterTabs.select(0);

        // ------------------------------------------------

        add<ImageView>({ .image = T.display.loadPreset.foreground, .enableMouse = false });

        // ------------------------------------------------

    }

    // ------------------------------------------------

    void LoadPresetTab::reloadFilters(Filter::Type type) {
        bool loadingSameType = m_CurrentType == type;
        m_CurrentType = type;
        if (loadingSameType) saveState(); // Save current state

        m_Search->settings.text = "";
        m_Search->repaint();
        m_Filters->clear();

        auto& database = context.controller<MiniSynthFMController>().presetDatabase;
        database.reloadInformation();

        switch (type) {
        case Filter::Type::Bank: {
            for (auto& bank : database.banks) {
                m_Filters->add<Filter>({ Width, 20 }, Filter::Settings{
                    .self = *this,
                    .type = Filter::Type::Bank,
                    .bankId = bank.id,
                    .value = bank.name
                });
            }
            break;
        }
        case Filter::Type::Type: {
            std::set<std::string_view> types;
            
            for (auto& bank : database.banks) {
                for (auto& preset : bank.presets()) {
                    types.insert(preset.presetData.type);
                }
            }

            for (auto& type : types) {
                m_Filters->add<Filter>({ Width, 20 }, Filter::Settings{
                    .self = *this,
                    .type = Filter::Type::Type,
                    .value = std::string(type),
                });
            }

            break;
        }       
        case Filter::Type::Author: {
            std::set<std::string_view> authors;
            
            for (auto& bank : database.banks) {
                for (auto& preset : bank.presets()) {
                    authors.insert(preset.presetData.author);
                }
            }

            for (auto& author : authors) {
                m_Filters->add<Filter>({ Width, 20 }, Filter::Settings{
                    .self = *this,
                    .type = Filter::Type::Author,
                    .value = std::string(author),
                });
            }

            break;
        }
        }

        select((Filter&)*m_Filters->views().front());
        m_Filters->updateDimensions();
        if (loadingSameType) loadState(); // Load back state
    }

    void LoadPresetTab::reload() {
        reloadFilters(m_CurrentType);
    }

    // ------------------------------------------------

    void LoadPresetTab::select(Filter& bank) {
        if (bank.selected()) return; // already selected

        m_Search->settings.text = "";
        m_Search->repaint();

        m_Presets->clear();

        for (auto& b : m_Filters->views()) {
            b->selected(false);
            b->repaint();
        }
        bank.selected(true);

        bank.foreach([&](const PresetDatabase::Bank::Preset& preset) {
            m_Presets->add<Preset>({ Width, 20 }, {
                .self = *this,
                .presetId = preset.id
            });
        });

        m_Presets->updateDimensions();
    }

    // ------------------------------------------------

    void LoadPresetTab::saveState() {
        m_State.search = m_Search->settings.text;
        m_State.scrolled = m_Presets->scrolled();
        for (auto& b : m_Filters->views()) {
            if (!b->selected()) continue;
            if (auto bank = dynamic_cast<Filter*>(b.get())) {
                m_State.type = bank->settings.type;
                m_State.value = bank->identifier();
                return;
            }
        }

        m_State.type = Filter::Type::Bank;
        m_State.value = "Factory";
    }

    void LoadPresetTab::loadState() {
        for (auto& b : m_Filters->views()) {
            if (auto bank = dynamic_cast<Filter*>(b.get())) {
                auto& database = context.controller<MiniSynthFMController>().presetDatabase;
                if (m_State.value == bank->identifier()) {
                    select(*bank);
                    break;
                }
            }
        }

        m_Search->settings.text = m_State.search;
        m_Search->repaint();
        applySearch();

        m_Presets->scrollTo(m_State.scrolled);
    }

    // ------------------------------------------------
    
    void LoadPresetTab::applySearch() {
        for (auto& view : m_Presets->views()) {
            if (auto entry = dynamic_cast<Preset*>(view.get())) {
                entry->setVisible(entry->matchesSearch(m_Search->content()));
            }
        }
        m_Presets->updateDimensions();
    }

    // ------------------------------------------------

}

// ------------------------------------------------
