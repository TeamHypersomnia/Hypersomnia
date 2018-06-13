#include "augs/filesystem/directory.h"

#include "application/intercosm.h"

#include "application/setups/editor/editor_popup.h"
#include "application/setups/editor/editor_autosave.h"
#include "application/setups/editor/editor_paths.h"
#include "application/setups/editor/editor_significant.h"

#include "augs/readwrite/lua_file.h"

static void save_last_folders(
	sol::state& lua,
	const editor_significant& signi
) {
	editor_last_folders last_folders;
	last_folders.current_index = signi.current_index;
	last_folders.paths.reserve(signi.folders.size());

	for (const auto& f : signi.folders) {
		if (const auto& h = f.history; h.empty()) {
			/* Drop empty projects without any changes */
			continue;
		}

		last_folders.paths.push_back(f.current_path);
	}

	augs::save_as_lua_table(lua, last_folders, get_last_folders_path());
}

void editor_autosave::save(
	sol::state& lua,
	const editor_significant& signi
) const {
	save_last_folders(lua, signi);

	for (const auto& f : signi.folders) {
		if (const auto& h = f.history;
			h.at_unsaved_revision() || h.was_modified()
		) {
			auto autosave_path = f.get_autosave_path();
			augs::create_directories(autosave_path);

			try {
				f.save_folder(autosave_path, ::get_project_name(f.current_path));
			}
			catch (...) {
				/* 
					Uhh... that would suck, 
					but at this point the user might be forcibly closing the app,
					so there's no reason to try to communicate failure here.
				*/
			}
		}
	}
}

void open_last_folders(
	sol::state& lua,
	editor_significant& signi
) {
	ensure(signi.folders.empty());

	std::vector<editor_popup> failures;

	try {
		const auto opened_folders = augs::load_from_lua_table<editor_last_folders>(lua, get_last_folders_path());

		for (const auto& real_path : opened_folders.paths) {
			try {
				auto new_folder = editor_folder(real_path);

				if (const auto warning = new_folder.load_folder_maybe_autosave()) {
					failures.push_back(*warning);
				}

				signi.folders.emplace_back(std::move(new_folder));
			}
			catch (const editor_popup& p) {
				/* Could load neither from autosave nor the real path. Utter failure. */
				failures.push_back(p);
			}
		}

		if (signi.folders.empty()) {
			signi.current_index = static_cast<folder_index>(-1);
		}
		else {
			/* The folder that was originally specified as current could have failed to load */

			signi.current_index = std::min(
				static_cast<folder_index>(signi.folders.size()) - 1,
				opened_folders.current_index
			);
		}
	}
	catch (...) {

	}

	if (!failures.empty()) {
		throw editor_popup::sum_all(failures);
	}
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
