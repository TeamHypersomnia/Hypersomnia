#pragma once
#include "augs/filesystem/path.h"
#include "augs/filesystem/file.h"
#include "augs/filesystem/directory.h"
#include "all_paths.h"

inline auto get_editor_gui_state_path() {
	return USER_DIR / "editor_gui_state.bin";
}

inline auto get_editor_last_project_path() {
	return USER_DIR / "editor_last_project.txt";
}
