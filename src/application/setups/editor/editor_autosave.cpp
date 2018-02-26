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

std::optional<editor_popup> open_last_tabs(
	sol::state& lua,
	editor_significant& signi
) {
	ensure(signi.tabs.empty());
	ensure(signi.works.empty());

	std::vector<editor_popup> failures;

	try {
		auto opened_tabs = augs::load_from_lua_table<editor_saved_tabs>(lua, get_editor_tabs_path());

		if (!opened_tabs.tabs.empty()) {
			/* Reload intercosms */

			for (std::size_t i = 0; i < opened_tabs.tabs.size(); ++i) {
				auto new_intercosm_ptr = std::make_unique<intercosm>();
				auto& new_intercosm = *new_intercosm_ptr;

				if (!opened_tabs.tabs[i].is_untitled()) {
					/* 
						This work was explicitly named.
						First try to load an adjacent .unsaved file, if it exists.
					*/

					const auto maybe_unsaved_path = get_unsaved_path(opened_tabs.tabs[i].current_path);

					if (const auto popup = open_intercosm(new_intercosm, { lua, maybe_unsaved_path })) {
						const auto real_path = opened_tabs.tabs[i].current_path;

						if (const auto popup = open_intercosm(new_intercosm, { lua, real_path })) {
							failures.push_back(*popup);
							continue;
						}
					}
				}
				else {
					/* 
						This work was untitled, thus always written to in place, 
						so it cannot have an "unsaved" neighbor.
					*/
					const auto untitled_path = opened_tabs.tabs[i].current_path;

					if (const auto popup = open_intercosm(new_intercosm, { lua, untitled_path })) {
						failures.push_back(*popup);
						continue;
					}
				}

				signi.tabs.emplace_back(std::move(opened_tabs.tabs[i]));
				signi.works.emplace_back(std::move(new_intercosm_ptr));
			}

			if (signi.tabs.empty()) {
				signi.current_index = static_cast<tab_index_type>(-1);
			}
			else {
				/* The tab that was originally specified as current could have failed to load */

				signi.current_index = std::min(
					static_cast<tab_index_type>(signi.tabs.size()) - 1,
					opened_tabs.current_tab_index
				);
			}
		}
	}
	catch (...) {

	}

	if (failures.empty()) {
		return std::nullopt;
	}

	return editor_popup::sum_all(failures);
}

std::optional<editor_popup> open_intercosm(intercosm& work, const intercosm_path_op op) {
	if (op.path.empty()) {
		return std::nullopt;
	}

	try {
		work.open(op);
	}
	catch (const intercosm_loading_error err) {
		editor_popup p;

		p.title = err.title;
		p.details = err.details;
		p.message = err.message;

		return p;
	}

	return std::nullopt;
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
