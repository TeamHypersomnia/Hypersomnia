#include "augs/string/typesafe_sprintf.h"
#include "augs/string/string_templates.h"

#include "application/intercosm.h"
#include "application/setups/debugger/debugger_folder.h"
#include "application/setups/debugger/debugger_paths.h"
#include "augs/misc/imgui/simple_popup.h"
#include "augs/misc/maybe_official_path.h"

#include "augs/readwrite/byte_file.h"
#include "augs/readwrite/lua_file.h"
#include "game/cosmos/entity_handle.h"

#include "hypersomnia_version.h"

#include "application/arena/arena_handle.hpp"

std::string debugger_folder::get_display_path() const {
	return ::get_project_name(current_path);
}

debugger_folder::debugger_folder(const augs::path_type& p) : 
	current_path(p), 
	commanded(std::make_unique<debugger_commanded_state>()) 
{
	cosmic::set_flavour_id_cache_enabled(true, commanded->work.world);
}

debugger_paths debugger_folder::get_paths() const {
	return { current_path, ::get_project_name(current_path) };
}

void debugger_folder::set_folder_path(const augs::path_type& path) {
	current_path = path;
}

bool debugger_folder::is_untitled() const {
	return is_untitled_path(current_path);
}

void debugger_folder::save_folder(const debugger_save_type mode) const {
	save_folder(current_path, mode);
}

void debugger_folder::save_folder(const augs::path_type& to, const debugger_save_type mode) const {
	save_folder(to, ::get_project_name(to), mode);
}

void debugger_folder::save_folder(const augs::path_type& to, const augs::path_type name, const debugger_save_type mode) const {
	const auto paths = debugger_paths(to, name);

	augs::create_directories_for(paths.version_info_file);

	if (mode == debugger_save_type::ONLY_VIEW) {
		augs::save_as_bytes(view, paths.view_file);
		return;
	}

	/* For convenience, create subdirectories for content */

	augs::create_directory(to / "gfx");
	augs::create_directory(to / "sfx");
	augs::create_directory(paths.default_export_path);

	commanded->work.save_as_bytes(paths.arena.int_paths);

	augs::save_as_bytes(commanded->view_ids, paths.view_ids_file);
	augs::save_as_bytes(commanded->rulesets, paths.arena.rulesets_file_path);
	augs::save_as_bytes(view, paths.view_file);
	augs::save_as_bytes(history, paths.hist_file);
	augs::save_as_bytes(player, paths.player_file);

	augs::save_as_text(paths.version_info_file, hypersomnia_version().get_summary());

	const auto old_autosave_path = paths.autosave_path;
	augs::remove_directory(old_autosave_path);
}

void debugger_folder::load_folder() {
	return load_folder(current_path);
}

void debugger_folder::load_folder(const augs::path_type& from) {
	load_folder(from, ::get_project_name(from));
}

void debugger_folder::load_folder(const augs::path_type& from, const augs::path_type& name) {
	const auto paths = debugger_paths(from, name);

	try {
		load_intercosm_and_rulesets(
			paths.arena,
			commanded->work,
			commanded->rulesets
		);
	}
	catch (const std::exception& err) {
		simple_popup p;

		p.title = "Error";
		p.message = typesafe_sprintf("A problem occured when trying to load project folder \"%x\".", augs::filename_first(from));
		p.details = err.what();

		throw p;
	}

	try {
		augs::load_from_bytes(commanded->view_ids, paths.view_ids_file);
		augs::load_from_bytes(view, paths.view_file);
		augs::load_from_bytes(history, paths.hist_file);
		augs::load_from_bytes(player, paths.player_file);
	}
	catch (const augs::file_open_error&) {
		/* We just let it happen. These files are not necessary. */
	}
}

void debugger_folder::mark_as_just_saved() {
	history.mark_as_just_saved();
	player.dirty = false;
}

bool debugger_folder::empty() const {
	if (player.has_testing_started()) {
		return false;
	}

	return history.empty();
}

bool debugger_folder::allow_close() const {
	if (player.dirty) {
		return false;
	}

	return history.at_saved_revision();
}


entity_id debugger_folder::get_controlled_character_id() const {
	const auto& overridden = view.overridden_viewed;

	if (overridden.is_set()) {
		return overridden;
	}

	return player.lookup_character(view.local_player_id);
}

std::optional<debugger_warning> debugger_folder::open_most_relevant_content(sol::state& lua) {
	const auto& real_path = current_path;

	try {
		import_folder(lua, current_path);
		return std::nullopt;
	}
	catch (const augs::file_open_error& err) {
		/* The importable files don't exist. Proceed with loading the binary files. */
	}
	catch (const std::exception& err) {
		ensure(false);
		simple_popup p;

		p.title = "Error";
		p.message = typesafe_sprintf("A problem occured when trying to import %x.", augs::filename_first(real_path));
		p.details = err.what();

		throw p;
	}

	auto reseek_player_to_stay_deterministic = [&]() {
		if (player.get_current_step() > 0) {
			const auto cmd_in = debugger_command_input::make_dummy_for(lua, *this);
			const auto target_step = player.get_current_step();

			const auto pre_seek_revision = history.get_current_revision();

			player.pause();
			player.seek_to(target_step - 1, cmd_in, false);
			player.seek_to(target_step, cmd_in, false);

			history.seek_to_revision(pre_seek_revision, cmd_in);
		}
	};

	try {
		/* First try to load from the neighbouring autosave folder. */
		const auto autosave_path = get_autosave_path();
		load_folder(autosave_path, ::get_project_name(real_path));
		reseek_player_to_stay_deterministic();

		if (!augs::exists(real_path)) {
			const auto display_autosave = augs::filename_first(autosave_path);
			const auto display_real = augs::filename_first(real_path);

			const auto message = typesafe_sprintf(
				"Found the autosave file %x,\nbut there is no %x!\nSave the file immediately!",
				display_autosave,
				display_real
			);

			return debugger_warning { "Warning", message, "" };
		}
	}
	catch (const simple_popup& p) {
		/* If no autosave folder was found, try the real path. */
		*this = debugger_folder(real_path);
		load_folder();
		reseek_player_to_stay_deterministic();
		mark_as_just_saved();
	}

	return std::nullopt;
}

augs::path_type debugger_folder::get_autosave_path() const {
	return current_path / "autosave";
}

bool debugger_folder::should_autosave() const {
#if 0
	if (player.has_testing_started()) {
		return player.dirty;
	}
#endif

	return player.dirty || history.at_unsaved_revision() || history.is_dirty();
}

void debugger_folder::autosave_if_needed() const {
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
	else {
		/* Always implicitly save at least the view information */
		save_folder(debugger_save_type::ONLY_VIEW);
	}
}

void debugger_folder::export_folder(sol::state& lua, const augs::path_type& to) const {
	const auto name = ::get_project_name(to);
	const auto paths = debugger_paths(to, name);

	commanded->work.save_as_lua({ lua, paths.int_lua_file });
	augs::save_as_lua_table(lua, commanded->rulesets, paths.rulesets_lua_file);
}

void debugger_folder::import_folder(sol::state& lua, const augs::path_type& from) {
	const auto name = ::get_project_name(from);
	const auto paths = debugger_paths(from, name);

	const auto& int_lua_path = paths.int_lua_file;

	commanded->work.load_from_lua({ lua, int_lua_path });

	try {
		augs::load_from_lua_table(lua, commanded->rulesets, paths.rulesets_lua_file);
	}
	catch (...) {
		/* It's not necessary that we have the modes. */
	}

	const auto& new_path = paths.imported_folder_path;

	augs::create_directory(new_path);
	set_folder_path(new_path);
}

double debugger_folder::get_inv_tickrate() const {
	if (player.has_testing_started()) {
		return std::visit(
			[&](const auto& typed_mode) {
				using M = remove_cref<decltype(typed_mode)>;

				if constexpr(std::is_same_v<test_mode, M>) {
					return commanded->work.world.get_fixed_delta().in_seconds<double>();
				}
				else {
					return typed_mode.get_round_speeds().calc_inv_tickrate();
				}
			},
			player.get_current_mode()
		);
	}

	return 1 / 60.0;
}

double debugger_folder::get_audiovisual_speed() const {
	return player.get_audiovisual_speed(*this);
}
