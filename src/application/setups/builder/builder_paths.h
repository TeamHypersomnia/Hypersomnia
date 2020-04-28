#pragma once
#include "augs/filesystem/path.h"
#include "augs/filesystem/file.h"
#include "augs/filesystem/directory.h"

#include "application/arena/arena_paths.h"

#define BUILDER_DIR (augs::path_type(USER_FILES_DIR) / "builder")

inline auto get_project_selector_gui_state_path() {
	return BUILDER_DIR / "project_selector.lua";
}

inline auto get_builder_gui_state_path() {
	return BUILDER_DIR / "builder_gui_state.bin";
}

inline auto get_project_name(const augs::path_type& p) {
	return p.filename().string();
}
