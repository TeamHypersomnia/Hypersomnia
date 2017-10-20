#include "application/setups/editor/editor_tab.h"
#include "application/setups/editor/editor_paths.h"
#include "application/setups/editor/editor_recent_paths.h"

std::string editor_tab::get_display_path() const {
	return current_path.filename().string();
}

void editor_tab::set_workspace_path(
	sol::state& lua, 
	const augs::path_type& path, 
	editor_recent_paths& recent
) {
	current_path = path;
	recent.add(lua, path);
}

bool editor_tab::has_unsaved_changes() const {
	return true;
}

bool editor_tab::is_untitled() const {
	return is_untitled_path(current_path);
}
