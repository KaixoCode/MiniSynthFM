
// ------------------------------------------------

#include "Kaixo/MiniSynthFM/Gui/Tabs/SavePresetTab.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    void PresetTab::presetSaved() { reloadPresetInformation(); }
    void PresetTab::presetLoaded() { reloadPresetInformation(); }

    void PresetTab::reloadPresetInformation() {
        name->setText(context.data<PresetData>().name);
        author->setText(context.data<PresetData>().author);
        type->setText(context.data<PresetData>().type);
        description->setText(context.data<PresetData>().description);
        context.defaultDescription(context.data<PresetData>().description);
        context.clearDescription();
        repaint();
    }

    // ------------------------------------------------

    PresetTab::PresetTab(Context c, Settings s)
        : View(c), settings(s)
    {

        // ------------------------------------------------

        add<ImageView>({ .image = T.display.savePreset.background });

        // ------------------------------------------------

        name = &add<TextView>({ 6, 6, 300, 20 }, {
            .graphics = T.display.savePreset.name,
            .padding = { 4, 3 },
            .multiline = false,
            .editable = true,
            .lineHeight = 14,
            .maxSize = 32,
            .placeholder = "Name"
        });

        author = &add<TextView>({ 6, 28, 300, 20 }, {
            .graphics = T.display.savePreset.author,
            .padding = { 4, 3 },
            .multiline = false,
            .editable = true,
            .lineHeight = 14,
            .maxSize = 32,
            .placeholder = "Author"
        });

        type = &add<TextView>({ 6, 50, 300, 20 }, {
            .graphics = T.display.savePreset.type,
            .padding = { 4, 3 },
            .multiline = false,
            .editable = true,
            .lineHeight = 14,
            .maxSize = 32,
            .placeholder = "Type"
        });

        description = &add<TextView>({ 6, 72, 300, 65 }, {
            .graphics = T.display.savePreset.description,
            .padding = { 4, 3 },
            .multiline = true,
            .editable = true,
            .lineHeight = 16,
            .maxSize = 255,
            .placeholder = "Description"
        });

        add<Button>({ 160, 139, 146, 20 }, {
            .callback = [&](bool) {
                if constexpr (versionType == VersionType::Demo) {
                    settings.popup.open([](bool) {}, "You cannot save presets in demo mode.", false);
                    return;
                }
                if (name->empty()) settings.popup.open([](bool) {}, "You cannot leave the preset name blank.", false);
                else savePreset(false);
            },
            .graphics = T.display.savePreset.saveButton,
        });

        add<Button>({ 6, 139, 148, 20 }, {
            .callback = [&](bool) {
                context.tabControl(0).select(0);
            },
            .graphics = T.display.savePreset.cancelButton,
        });

        // ------------------------------------------------

        add<ImageView>({ .image = T.display.savePreset.foreground, .enableMouse = false });

        // ------------------------------------------------

        reloadPresetInformation();

        // ------------------------------------------------

    }

    // ------------------------------------------------

    void PresetTab::savePreset(bool force) {
        context.data<PresetData>().name = name->content();
        context.data<PresetData>().author = author->content();
        context.data<PresetData>().type = type->content();
        context.data<PresetData>().description = description->content();

        context.defaultDescription(description->content());

        std::string path = Storage::getOrDefault<std::string>(PresetPath, "No Preset Path");
        std::filesystem::path presetPath = path;
        if (std::filesystem::exists(presetPath)) {
            auto file = presetPath / (name->content() + ".minifm");
            resultHandler(context.savePreset(file, force));
        } else {
            settings.popup.open([this](bool v) {
                context.tabControl(0).select(3);
            }, "No preset path is specified, please select one.", false);
        }
    }

    // ------------------------------------------------

    void PresetTab::resultHandler(SaveResult result) {
        switch (result) {
        case SaveResult::Success:
            context.tabControl(0).select(0);
            return settings.popup.open([](bool) {}, "Preset saved!", false);
        case SaveResult::AlreadyExists:
            return settings.popup.open([this](bool v) {
                if (!v) return;
                savePreset(true);
            }, "File already exists, do you wish to overwrite?");
        case SaveResult::CannotWrite:
            return settings.popup.open([](bool) {}, "Cannot write to file, check folder permissions.", false);
        case SaveResult::InvalidPath:
            return settings.popup.open([](bool) {}, "Cannot save preset.", false);
        }
    }

    // ------------------------------------------------

}

// ------------------------------------------------
