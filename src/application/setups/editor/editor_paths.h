#pragma once

#include "augs/filesystem/path.h"
#include "augs/filesystem/file.h"
#include "augs/filesystem/directory.h"

#define EDITOR_DIR LOCAL_FILES_DIR "/editor"

inline auto get_last_folders_path() {
	return EDITOR_DIR "/last_folders.lua";
}

inline auto get_recent_paths_path() {
	return EDITOR_DIR "/editor_recent_paths.lua";
}

inline auto get_editor_gui_state_path() {
	return EDITOR_DIR "/editor_gui_state.bin";
}

inline auto get_untitled_dir() {
	return EDITOR_DIR "/untitled";
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

template <class T>
auto get_first_free_untitled_path(const T& path_template) {
	return augs::first_free_path(get_path_in_untitled(path_template));
}

inline auto get_unsaved_path(augs::path_type path) {
	return path.replace_extension(path.extension() += ".unsaved");
}

struct editor_paths {
	augs::path_type int_file;
	augs::path_type view_file;
	augs::path_type hist_file;
	augs::path_type autosave_path;

	editor_paths(
		const augs::path_type& target_folder,
		const std::string& project_name
	);
};

