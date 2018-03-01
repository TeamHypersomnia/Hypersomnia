#pragma once
#include "augs/filesystem/path.h"
#include "augs/filesystem/file.h"

#define EDITOR_DIR LOCAL_FILES_DIR "editor/"

inline auto get_last_folders_path() {
	return EDITOR_DIR "last_folders.lua";
}

inline auto get_recent_paths_path() {
	return EDITOR_DIR "editor_recent_paths.lua";
}

inline auto get_untitled_dir() {
	return EDITOR_DIR "untitled/";
}

inline bool is_untitled_path(augs::path_type path) {
	const auto untitled_dir = augs::path_type(get_untitled_dir()).make_preferred().string();
	const auto checked_path = path.make_preferred().string();

	return untitled_dir == checked_path.substr(0, untitled_dir.length());
}

template <class T>
auto get_path_in_untitled(const T& p) {
	return augs::path_type(get_untitled_dir()) += p;
}

template <class T>
auto get_first_free_untitled_path(const T& path_template) {
	return augs::first_free_path(get_path_in_untitled(path_template));
}

inline auto get_unsaved_path(augs::path_type path) {
	return path.replace_extension(path.extension() += ".unsaved");
}
