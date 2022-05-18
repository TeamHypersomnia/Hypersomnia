#include "augs/filesystem/directory.h"

#include "application/intercosm.h"

#include "augs/misc/imgui/simple_popup.h"
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
		if (f.empty()) {
			/* Drop empty projects without any changes whatsoever */
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
		f.autosave_if_needed();
	}
}

void open_last_folders(
	sol::state& lua,
	editor_significant& signi
) {
	ensure(signi.folders.empty());

	std::vector<simple_popup> failures;

	try {
		const auto opened_folders = augs::load_from_lua_table<editor_last_folders>(lua, get_last_folders_path());

		for (const auto& real_path : opened_folders.paths) {
			try {
				auto new_folder = editor_folder(real_path);

				if (const auto warning = new_folder.open_most_relevant_content(lua)) {
					failures.push_back(*warning);
				}

				signi.folders.emplace_back(std::move(new_folder));
			}
			catch (const simple_popup& p) {
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
		throw simple_popup::sum_all(failures);
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
