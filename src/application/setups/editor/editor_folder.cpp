#include "augs/string/string_templates.h"

#include "application/intercosm.h"
#include "application/setups/editor/editor_folder.h"
#include "application/setups/editor/editor_paths.h"
#include "application/setups/editor/editor_recent_paths.h"
#include "application/setups/editor/editor_popup.h"
#include "view/maybe_official_path.h"

#include "augs/readwrite/byte_file.h"

std::string editor_folder::get_display_path() const {
	return ::get_project_name(current_path);
}

editor_folder::editor_folder(const augs::path_type& p) : current_path(p), work(std::make_unique<intercosm>()) {}

editor_paths editor_folder::get_paths() const {
	return { current_path, ::get_project_name(current_path) };
}

augs::path_type editor_folder::get_autosave_path() const {
	return current_path / "autosave";
}

void editor_folder::set_folder_path(
	sol::state& lua, 
	const augs::path_type& path, 
	editor_recent_paths& recent
) {
	current_path = path;
	recent.add(lua, path);
}

bool editor_folder::at_unsaved_revision() const {
	return history.at_unsaved_revision();
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

	work->save_as_int(paths.int_file);
	augs::save_as_bytes(view, paths.view_file);
	augs::save_as_bytes(history, paths.hist_file);

	const auto old_autosave_path = paths.autosave_path;
	augs::remove_directory(old_autosave_path);
}

std::optional<editor_warning> editor_folder::load_folder_maybe_autosave() {
	const auto& real_path = current_path;

	try {
		/* First try to load from the neighbouring autosave folder. */
		const auto autosave_path = get_autosave_path();
		load_folder(autosave_path, ::get_project_name(real_path));

		if (!augs::exists(real_path)) {
			const auto display_autosave = augs::to_display(autosave_path);
			const auto display_real = augs::to_display(real_path);

			const auto message = typesafe_sprintf(
				"Found the autosave file %x,\nbut there is no %x!\nSave the file immediately!",
				display_autosave,
				display_real
			);

			return editor_warning {"Warning", message, ""};
		}
	}
	catch (editor_popup p) {
		/* If no autosave folder was found, try the real path. */
		*this = editor_folder(real_path);
		load_folder();
		history.mark_as_just_saved();
	}

	return std::nullopt;
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
		work->load_from_int(paths.int_file);
	}
	catch (const intercosm_loading_error err) {
		editor_popup p;

		p.title = err.title;
		p.details = err.details;
		p.message = err.message;

		throw p;
	}

	try {
		augs::load_from_bytes(view, paths.view_file);
		augs::load_from_bytes(history, paths.hist_file);
	}
	catch (augs::file_open_error) {
		/* We just let it happen. These files are not necessary. */
	}
}
