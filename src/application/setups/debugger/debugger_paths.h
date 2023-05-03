#pragma once
#include "augs/filesystem/path.h"
#include "augs/filesystem/file.h"
#include "augs/filesystem/directory.h"

#include "application/arena/arena_paths.h"

#define DEBUGGER_DIR (augs::path_type(USER_FILES_DIR) / "debugger")

inline auto get_last_folders_path() {
	return DEBUGGER_DIR / "last_folders.lua";
}

inline auto get_recent_paths_path() {
	return DEBUGGER_DIR / "debugger_recent_paths.lua";
}

inline auto get_debugger_gui_state_path() {
	return DEBUGGER_DIR / "debugger_gui_state.bin";
}

inline auto get_untitled_dir() {
	return DEBUGGER_DIR / "untitled";
}

inline bool is_untitled_path(augs::path_type path) {
	const auto untitled_dir = augs::path_type(get_untitled_dir()).make_preferred().string();
	const auto checked_path = path.make_preferred().string();

	return checked_path.find(untitled_dir) != std::string::npos;
}

inline auto get_project_name(const augs::path_type& p) {
	return p.filename().string();
}

template <class T>
auto get_path_in_untitled(const T& p) {
	return augs::path_type(get_untitled_dir()) / p;
}

inline auto get_unsaved_path(augs::path_type path) {
	return path.replace_extension(path.extension() += ".unsaved");
}

struct debugger_paths {
	arena_paths arena;

	augs::path_type view_file;
	augs::path_type view_ids_file;
	augs::path_type hist_file;
	augs::path_type player_file;
	augs::path_type autosave_path;
	augs::path_type entropies_live_file;

	augs::path_type int_lua_file;
	augs::path_type json_export_file;
	augs::path_type rulesets_lua_file;

	augs::path_type default_export_path;
	augs::path_type imported_folder_path;

	augs::path_type version_info_file;

	debugger_paths(
		const augs::path_type& target_folder,
		const std::string& project_name
	);

	debugger_paths(
		const augs::path_type& target_folder,
		const augs::path_type& project_name
	);
};

