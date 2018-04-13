#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "augs/misc/imgui/path_tree_settings.h"

void path_tree_settings::do_tweakers() {
	using namespace augs::imgui;

	checkbox("Linear view", linear_view);
	ImGui::SameLine();
	checkbox("Prettify filenames", prettify_filenames);
}

