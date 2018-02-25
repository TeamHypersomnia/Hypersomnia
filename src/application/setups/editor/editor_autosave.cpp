#include "application/intercosm.h"

#include "application/setups/editor/editor_autosave.h"
#include "application/setups/editor/editor_paths.h"
#include "application/setups/editor/editor_significant.h"

#include "augs/readwrite/lua_file.h"

void editor_autosave::save(
	sol::state& lua,
	const editor_significant& signi
) const {
	editor_saved_tabs saved_tabs;

	const auto& tabs = signi.tabs;
	const auto& works = signi.works;

	saved_tabs.tabs = tabs;
	saved_tabs.current_tab_index = signi.current_index;

	for (std::size_t i = 0; i < tabs.size(); ++i) {
		const auto& t = tabs[i];
		const auto& w = *works[i];

		if (t.is_untitled()) {
			/* The work is untitled anyway, so we save it in place. */ 

			const auto saving_path = t.current_path;
			w.save({ lua, saving_path });
		}
		else if (t.has_unsaved_changes()) {
			/* 
				The work was explicitly saved at some point, so create a backup file with yet unsaved changes. 
				The .unsaved file will be prioritized when loading.
			*/ 

			auto extension = t.current_path.extension();
			const auto saving_path = augs::path_type(t.current_path).replace_extension(extension += ".unsaved");
			w.save({ lua, saving_path });
		}
	}

	augs::save_as_lua_table(lua, saved_tabs, get_editor_tabs_path());
}

void editor_autosave::advance(
	sol::state& lua,
	const editor_significant& signi,
	const editor_autosave_settings& settings
) {
	if (last_settings != settings) {
		autosave_timer = {};
	}

	last_settings = settings;

	if (settings.enabled 
		&& settings.once_every_min <= autosave_timer.get<std::chrono::minutes>()
	) {
		save(lua, signi);
		autosave_timer.reset();
	}
}
