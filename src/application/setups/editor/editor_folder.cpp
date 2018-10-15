#include "augs/string/string_templates.h"

#include "application/intercosm.h"
#include "application/setups/editor/editor_folder.h"
#include "application/setups/editor/editor_paths.h"
#include "application/setups/editor/editor_popup.h"
#include "augs/misc/maybe_official_path.h"

#include "augs/readwrite/byte_file.h"
#include "augs/readwrite/lua_file.h"
#include "game/cosmos/entity_handle.h"

std::string editor_folder::get_display_path() const {
	return ::get_project_name(current_path);
}

editor_folder::editor_folder(const augs::path_type& p) : 
	current_path(p), 
	commanded(std::make_unique<editor_commanded_state>()) 
{}

editor_paths editor_folder::get_paths() const {
	return { current_path, ::get_project_name(current_path) };
}

void editor_folder::set_folder_path(const augs::path_type& path) {
	current_path = path;
}

bool editor_folder::is_untitled() const {
	return is_untitled_path(current_path);
}

void editor_folder::save_folder() const {
	save_folder(current_path);
}

void editor_folder::save_folder(const augs::path_type& to) const {
	save_folder(to, ::get_project_name(to));
}

void editor_folder::save_folder(const augs::path_type& to, const augs::path_type name) const {
	const auto paths = editor_paths(to, name);

	augs::create_directories_for(paths.int_file);

	/* For convenience, create subdirectories for content */

	augs::create_directory(to / maybe_official_path<assets::image_id>::get_content_suffix());
	augs::create_directory(to / maybe_official_path<assets::sound_id>::get_content_suffix());
	augs::create_directory(paths.default_export_path);

	commanded->work.save_as_int(paths.int_file);

	augs::save_as_bytes(commanded->view_ids, paths.view_ids_file);
	augs::save_as_bytes(commanded->mode_vars, paths.modes_file);
	augs::save_as_bytes(view, paths.view_file);
	augs::save_as_bytes(history, paths.hist_file);
	augs::save_as_bytes(player, paths.player_file);

	const auto old_autosave_path = paths.autosave_path;
	augs::remove_directory(old_autosave_path);
}

void editor_folder::load_folder() {
	return load_folder(current_path);
}

void editor_folder::load_folder(const augs::path_type& from) {
	load_folder(from, ::get_project_name(from));
}

void editor_folder::load_folder(const augs::path_type& from, const augs::path_type& name) {
	const auto paths = editor_paths(from, name);

	try {
		commanded->work.load_from_int(paths.int_file);
	}
	catch (const intercosm_loading_error& err) {
		editor_popup p;

		p.title = err.title;
		p.details = err.details;
		p.message = err.message;

		throw p;
	}

	try {
		augs::load_from_bytes(commanded->view_ids, paths.view_ids_file);
		augs::load_from_bytes(commanded->mode_vars, paths.modes_file);
		augs::load_from_bytes(view, paths.view_file);
		augs::load_from_bytes(history, paths.hist_file);
		augs::load_from_bytes(player, paths.player_file);
	}
	catch (const augs::file_open_error&) {
		/* We just let it happen. These files are not necessary. */
	}
}

void editor_folder::mark_as_just_saved() {
	history.mark_as_just_saved();
	player.dirty = false;
}

bool editor_folder::empty() const {
	if (player.has_testing_started()) {
		return false;
	}

	return history.empty();
}

bool editor_folder::allow_close() const {
	if (player.dirty) {
		return false;
	}

	if (player.has_testing_started()) {
		return true;
	}

	return history.at_saved_revision();
}


entity_id editor_folder::get_viewed_character_id() const {
	const auto& overridden = view.overridden_viewed;

	if (overridden.is_set()) {
		return overridden;
	}

	return commanded->work.world[player.lookup_character(view.local_player)].get_id();
}

std::optional<editor_warning> editor_folder::open_most_relevant_content(sol::state& lua) {
	const auto& real_path = current_path;

	try {
		/* Try to import the lua files first. */
		import_folder(lua, current_path);
	}
	catch (...) {
		/* We don't care if it fails, then just try to load the actual binaries. */
	}

	try {
		/* First try to load from the neighbouring autosave folder. */
		const auto autosave_path = get_autosave_path();
		load_folder(autosave_path, ::get_project_name(real_path));

		if (!augs::exists(real_path)) {
			const auto display_autosave = augs::filename_first(autosave_path);
			const auto display_real = augs::filename_first(real_path);

			const auto message = typesafe_sprintf(
				"Found the autosave file %x,\nbut there is no %x!\nSave the file immediately!",
				display_autosave,
				display_real
			);

			return editor_warning { "Warning", message, "" };
		}
	}
	catch (const editor_popup& p) {
		/* If no autosave folder was found, try the real path. */
		*this = editor_folder(real_path);
		load_folder();
		mark_as_just_saved();
	}

	return std::nullopt;
}

augs::path_type editor_folder::get_autosave_path() const {
	return current_path / "autosave";
}

bool editor_folder::should_autosave() const {
	if (player.has_testing_started()) {
		return player.dirty;
	}

	return player.dirty || history.at_unsaved_revision() || history.was_modified();
}

void editor_folder::autosave_if_needed() const {
	if (should_autosave()) {
		const auto autosave_path = get_autosave_path();
		augs::create_directories(autosave_path);

		try {
			save_folder(autosave_path, ::get_project_name(current_path));
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

void editor_folder::export_folder(sol::state& lua, const augs::path_type& to) const {
	const auto name = ::get_project_name(to);
	const auto paths = editor_paths(to, name);

	commanded->work.save_as_lua({ lua, paths.int_lua_file });
	augs::save_as_lua_table(lua, commanded->mode_vars, paths.modes_lua_file);
}

void editor_folder::import_folder(sol::state& lua, const augs::path_type& from) {
	const auto name = ::get_project_name(from);
	const auto paths = editor_paths(from, name);

	try {
		commanded->work.load_from_lua({ lua, paths.int_lua_file });
	}
	catch (const intercosm_loading_error& err) {
		editor_popup p;

		p.title = err.title;
		p.details = err.details;
		p.message = err.message;

		throw p;
	}

	try {
		augs::load_from_lua_table(lua, commanded->mode_vars, paths.modes_lua_file);
	}
	catch (...) {
		/* It's not necessary that we have the modes. */
	}

	const auto& new_path = paths.imported_folder_path;

	augs::create_directory(new_path);
	set_folder_path(new_path);
}
