#include "application/setups/editor/gui/asset_browser_settings.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"

void asset_browser_settings::do_tweakers() {
	using namespace augs::imgui;

	tree_settings.do_tweakers();
	ImGui::SameLine();
	checkbox("Show unused", show_unused);

	ImGui::Separator();
}


