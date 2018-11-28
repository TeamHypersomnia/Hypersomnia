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

		{
			auto chosen_port = static_cast<int>(into.port);

			ImGui::InputInt("Port", std::addressof(chosen_port), 0, 0);
			into.port = static_cast<unsigned short>(std::clamp(chosen_port, 1024, 65535));
		}

		checkbox("IPv4", into.ipv4.is_enabled);

		{
			auto ind = scoped_indent();
			auto maybe_disabled = maybe_disabled_cols({}, !into.ipv4.is_enabled);
			input_text<100>("Address (leave empty to find automatically)##4", into.ipv4.value);
		}

		checkbox("IPv6", into.ipv6.is_enabled);

		{
			auto ind = scoped_indent();
			auto maybe_disabled = maybe_disabled_cols({}, !into.ipv6.is_enabled);
			input_text<100>("Address (leave empty to find automatically)##6", into.ipv6.value);
		}

		text_disabled("Tip: you can tweak many other settings when the server is up and running.\nYou can edit the defaults inside the cache/usr/config.local.lua file,\nin the default_server_vars section.");
	}

	{
		auto scope = scoped_child("launch cancel");

		ImGui::Separator();

		{
			auto maybe_disabled = maybe_disabled_cols({}, !into.ipv6.is_enabled && !into.ipv4.is_enabled);

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

