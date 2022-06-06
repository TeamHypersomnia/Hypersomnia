#include "application/setups/debugger/gui/asset_path_browser_settings.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"

void asset_path_browser_settings::do_tweakers() {
	using namespace augs::imgui;

	tree_settings.do_tweakers();
	ImGui::SameLine();
	checkbox("Orphaned", show_orphaned);

	ImGui::SameLine();
	checkbox("Using locations", show_using_locations);

	ImGui::SameLine();
	checkbox("Properties column", show_properties_column);
}


