#pragma once
#include "augs/filesystem/path.h"
#include "augs/filesystem/file.h"
#include "augs/filesystem/directory.h"

#define EDITOR_PROJECTS_DIR (augs::path_type(USER_FILES_DIR) / "projects")

inline auto get_project_selector_gui_state_path() {
	return EDITOR_PROJECTS_DIR / "project_selector.lua";
}

inline auto get_editor_gui_state_path() {
	return EDITOR_PROJECTS_DIR / "editor_gui_state.bin";
}
