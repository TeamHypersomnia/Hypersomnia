#include "application/gui/start_server_gui.h"

#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "augs/misc/imgui/imgui_utils.h"
#include "application/setups/editor/detail/maybe_different_colors.h"

#define SCOPE_CFG_NVP(x) format_field_name(std::string(#x)) + "##" + std::to_string(field_id++), scope_cfg.x

bool start_server_gui_state::perform(
	server_start_input& into
) {
	// int field_id = 0;

	if (!show) {
		return false;
	}

	using namespace augs::imgui;

	bool result = false;

	centered_size_mult = 0.35f;

	auto window = make_scoped_window();

	if (!window) {
		return false;
	}

	{
		auto child = scoped_child("host view", ImVec2(0, -(ImGui::GetFrameHeightWithSpacing() + 4)));
		auto width = scoped_item_width(ImGui::GetWindowWidth() * 0.35f);

		// auto& scope_cfg = into;

		input_text<100>("Address (leave empty to find automatically)", into.ip);
		text_disabled("Tip: the address can be either IPv4 or IPv6.\nFor example, you can put the IPv6 loopback address, which is \"::1\".");

		{
			auto chosen_port = static_cast<int>(into.port);

			ImGui::InputInt("Port", std::addressof(chosen_port), 0, 0);
			into.port = static_cast<unsigned short>(std::clamp(chosen_port, 1024, 65535));
		}

		slider("Max incoming connections", into.max_connections, 1, 64);
		text_disabled("Tip: this number does not include the player at the server machine.\nIf you want to play a 1v1 with someone and not allow anyone else to join or watch,\nyou want to set this value to 1.\n\n");

		text_disabled("Tip: you can tweak many other settings when the server is up and running.\nYou can edit the defaults inside the cache/usr/config.local.lua file,\nin the server section.");
	}

	{
		auto scope = scoped_child("launch cancel");

		ImGui::Separator();

		{
			if (ImGui::Button("Launch!")) {
				result = true;
				//show = false;
			}
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel")) {
			show = false;
		}
	}

	return result;
}

