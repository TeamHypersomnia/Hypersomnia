#include "augs/string/string_templates.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "augs/misc/imgui/path_tree_structs.h"

void path_tree_settings::do_tweakers() {
	using namespace augs::imgui;

	checkbox("Linear view", linear_view);
	ImGui::SameLine();
	checkbox("Prettify filenames", prettify_filenames);
}

void path_tree_settings::do_name_location_columns() const {
	using namespace augs::imgui;

	text_disabled(prettify_filenames ? "Name" : "Filename");
	ImGui::NextColumn();
	text_disabled("Location");
	ImGui::NextColumn();
}

std::string path_tree_settings::get_prettified(const std::string& filename) const {
	return prettify_filenames ? format_field_name(augs::path_type(filename).stem()) : filename;
}
