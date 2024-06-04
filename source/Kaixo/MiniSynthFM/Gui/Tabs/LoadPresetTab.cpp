
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
        setWantsKeyboardFocus(true);
        graphics = T.display.loadPreset.bank;
        animation(graphics);
    }

    // ------------------------------------------------

    void LoadPresetTab::Filter::mouseDown(const juce::MouseEvent& e) {
        settings.self.select(*this);
    }
    
    bool LoadPresetTab::Filter::keyPressed(const juce::KeyPress& e) {
        if (e.isKeyCode(e.upKey)) {
            auto& filters = settings.self.m_Filters->views();
            auto it = std::find_if(filters.begin(), filters.end(),
                [this](std::unique_ptr<View>& o) { return o.get() == dynamic_cast<View*>(this); });
            if (it != filters.begin()) {
                (*--it)->focused(true);
                settings.self.select(*dynamic_cast<Filter*>(it->get()));
                settings.self.m_Filters->scrollToKeepVisible(it->get());
                settings.self.repaint();
            }

            return true;
        }        

        if (e.isKeyCode(e.downKey)) {
            auto& filters = settings.self.m_Filters->views();
            auto it = std::find_if(filters.begin(), filters.end(),
                [this](std::unique_ptr<View>& o) { return o.get() == dynamic_cast<View*>(this); });
            if (it != filters.end() && ++it != filters.end()) {
                (*it)->focused(true);
                settings.self.select(*dynamic_cast<Filter*>(it->get()));
                settings.self.m_Filters->scrollToKeepVisible(it->get());
                settings.self.repaint();
            }

            return true;
        }

        if (e.isKeyCode(e.rightKey)) {
            auto& presets = settings.self.m_Presets->views();
            if (!presets.empty()) {
                presets.front()->focused(true);
                settings.self.m_Presets->scrollToKeepVisible(presets.front().get());
                settings.self.repaint();
            }

            return true;
        }
    }
    // ------------------------------------------------

    void LoadPresetTab::Filter::paint(juce::Graphics& g) {
        auto& database = context.controller<MiniSynthFMController>().presetDatabase;
        auto name = identifier();
        if (name.empty()) name = "Other";

        graphics.draw({
            .graphics = g,
            .bounds = localDimensions(),
            .text = { { "$name", name } },
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
                    if (preset.presetData.type.contains(settings.value)) {
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
        setWantsKeyboardFocus(true);
        graphics = T.display.loadPreset.preset;
        reloadDisplayName();
        animation(graphics);
    }

    // ------------------------------------------------

    void LoadPresetTab::Preset::mouseDown(const juce::MouseEvent& e) {
        load();
    }

    bool LoadPresetTab::Preset::keyPressed(const juce::KeyPress& e) {
        if (e.isKeyCode(e.returnKey)) {
            load();
            return true;
        }

        if (e.isKeyCode(e.upKey)) {
            auto& presets = settings.self.m_Presets->views();
            auto it = std::find_if(presets.begin(), presets.end(),
                [this](std::unique_ptr<View>& o) { return o.get() == dynamic_cast<View*>(this); });
            if (it != presets.begin()) {
                (*--it)->focused(true);
                settings.self.m_Presets->scrollToKeepVisible(it->get());
                settings.self.repaint();
            }

            return true;
        }        

        if (e.isKeyCode(e.downKey)) {
            auto& presets = settings.self.m_Presets->views();
            auto it = std::find_if(presets.begin(), presets.end(),
                [this](std::unique_ptr<View>& o) { return o.get() == dynamic_cast<View*>(this); });
            if (it != presets.end() && ++it != presets.end()) {
                (*it)->focused(true);
                settings.self.m_Presets->scrollToKeepVisible(it->get());
                settings.self.repaint();
            }

            return true;
        }

        if (e.isKeyCode(e.leftKey)) {
            auto& filters = settings.self.m_Filters->views();
            auto it = std::find_if(filters.begin(), filters.end(),
                [this](std::unique_ptr<View>& o) { return o->selected(); });
            if (it != filters.end()) {
                (*it)->focused(true);
                settings.self.select(*dynamic_cast<Filter*>(it->get()));
                settings.self.m_Filters->scrollToKeepVisible(it->get());
                settings.self.repaint();
            } else if (!filters.empty()) {
                filters.front()->focused(true);
                settings.self.select(*dynamic_cast<Filter*>(filters.front().get()));
                settings.self.m_Filters->scrollToKeepVisible(filters.front().get());
                settings.self.repaint();
            }

            return true;
        }
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

        m_Search = &add<TextView>({ 113, 6, 159, 20 }, {
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

        m_SortButton = &add<Button>({ 274, 6, 32, 20 }, {
            .callback = [&](bool v) {
                sortPresets(v);
            },
            .graphics = T.display.loadPreset.sortButton,
            .behaviour = Button::Behaviour::Toggle
        });

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
                    auto presetTypes = split_types(preset.presetData.type);
                    for (auto& type : presetTypes) {
                        types.insert(type);
                    }
                }
            }

            std::vector<std::string_view> sortedTypes{ types.begin(), types.end() };
            std::ranges::sort(sortedTypes);

            for (auto& type : sortedTypes) {
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

            std::vector<std::string_view> sortedAuthors{ authors.begin(), authors.end() };
            std::ranges::sort(sortedAuthors);

            for (auto& author : sortedAuthors) {
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
                .presetId = preset.id,
                .isInit = preset.type == PresetDatabase::Bank::Preset::Type::Init
            });
        });

        sortPresets(m_SortButton->selected());
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

    // ------------------------------------------------

}

// ------------------------------------------------
