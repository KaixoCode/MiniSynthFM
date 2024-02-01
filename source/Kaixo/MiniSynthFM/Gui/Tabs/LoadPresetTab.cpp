
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

    LoadPresetTab::Bank::Bank(Context c, Settings s)
        : View(c), settings(std::move(s))
    {
        graphics = T.display.loadPreset.bank;
        animation(graphics);
    }

    // ------------------------------------------------

    void LoadPresetTab::Bank::mouseDown(const juce::MouseEvent& e) {
        settings.self.select(*this);
    }

    // ------------------------------------------------

    void LoadPresetTab::Bank::paint(juce::Graphics& g) {
        auto& database = context.controller<MiniSynthFMController>().presetDatabase;
        if (settings.bankIndex >= database.banks.size()) return; // Does not exist anymore!

        graphics.draw({
            .graphics = g,
            .bounds = localDimensions(),
            .text = database.banks[settings.bankIndex].name,
            .state = state()
        });
    }

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
        if (settings.bankIndex >= database.banks.size()) return {}; // Does not exist anymore!
        auto& presets = database.banks[settings.bankIndex].presets();
        if (settings.presetIndex >= presets.size()) return {}; // Does not exist anymore!
        return presets[settings.presetIndex].presetData;
    }

    void LoadPresetTab::Preset::load() {
        auto& database = context.controller<MiniSynthFMController>().presetDatabase;
        if (settings.bankIndex >= database.banks.size()) return; // Does not exist anymore!
        auto& presets = database.banks[settings.bankIndex].presets();
        if (settings.presetIndex >= presets.size()) return; // Does not exist anymore!
        presets[settings.presetIndex].load();
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

        m_Banks = &add<ScrollView>({ 6, 6, 100, Height - 12 }, {
            .scrollbar = T.scrollbar,
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
            for (auto& view : m_Presets->views()) {
                if (auto entry = dynamic_cast<Preset*>(view.get())) {
                    entry->setVisible(entry->matchesSearch(m_Search->content()));
                }
            }
            m_Presets->updateDimensions();
        });

        m_Presets = &add<ScrollView>({ 113, 28, 193, Height - 34 }, {
            .scrollbar = T.scrollbar,
            .margin = { 0, 0, 0, 0 },
            .gap = 2,
            .barThickness = 5,
            .barPadding = { 2, 0, 0, 0 },
            .keepBarSpace = false,
            .alignChildren = Theme::Align::Left
        });

        reloadBanks();
    }

    // ------------------------------------------------

    void LoadPresetTab::reloadBanks() {
        m_Banks->clear();

        auto& database = context.controller<MiniSynthFMController>().presetDatabase;
        database.reloadInformation();

        std::size_t index = 0;
        for (auto& bank : database.banks) {
            m_Banks->add<Bank>({ Width, 20 }, Bank::Settings{
                .self = *this,
                .bankIndex = index++
                });
        }

        select((Bank&)*m_Banks->views().front());
    }

    // ------------------------------------------------

    void LoadPresetTab::select(Bank& bank) {
        m_Presets->clear();

        for (auto& b : m_Banks->views()) {
            b->selected(false);
            b->repaint();
        }
        bank.selected(true);

        auto& database = context.controller<MiniSynthFMController>().presetDatabase;

        if (bank.settings.bankIndex >= database.banks.size()) return; // Does not exist anymore!

        auto& presets = database.banks[bank.settings.bankIndex].presets();

        std::size_t index = 0;
        for (auto& preset : presets) {
            m_Presets->add<Preset>({ Width, 20 }, {
                .self = *this,
                .bankIndex = bank.settings.bankIndex,
                .presetIndex = index++
                });
        }

        m_Presets->updateDimensions();
    }

    // ------------------------------------------------

}

// ------------------------------------------------
