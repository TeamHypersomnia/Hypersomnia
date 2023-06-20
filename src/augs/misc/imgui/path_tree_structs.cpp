#include "augs/string/string_templates.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "augs/misc/imgui/path_tree_structs.h"
#include "augs/filesystem/path.h"

void path_tree_settings::do_tweakers() {
	using namespace augs::imgui;

#if TODO_LINEAR_VIEW
	checkbox("Linear view", linear_view);
	ImGui::SameLine();
#endif
	checkbox("Pretty names", pretty_names);
}

void path_tree_settings::do_name_location_columns() const {
	using namespace augs::imgui;

	text_disabled(pretty_names ? "Name" : "Filename");
	ImGui::NextColumn();
	text_disabled("Location");
	ImGui::NextColumn();
}

std::string path_tree_settings::get_prettified(const std::string& filename) const {
	return pretty_names ? format_field_name(augs::path_type(filename).stem().string()) : filename;
}
