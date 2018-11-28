#include "application/gui/start_client_gui.h"

#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "augs/misc/imgui/imgui_utils.h"

#define SCOPE_CFG_NVP(x) format_field_name(std::string(#x)) + "##" + std::to_string(field_id++), scope_cfg.x

bool start_client_gui_state::perform(
	client_start_input& into
) {
	if (!show) {
		return false;
	}
	
	using namespace augs::imgui;

	centered_size_mult = 0.3f;
	
	auto window = make_scoped_window();

	if (!window) {
		return false;
	}

	bool result = false;

	auto settings = scoped_window("Connect to server", &show);
	
	{
		auto child = scoped_child("connect view", ImVec2(0, -(ImGui::GetFrameHeightWithSpacing() + 4)));
		auto width = scoped_item_width(ImGui::GetWindowWidth() * 0.35f);

		// auto& scope_cfg = into;

		base::acquire_keyboard_once();
		input_text<100>("Address (ip4:port or [ip6]:port)", into.ip_port);
		input_text<32>("Chosen nickname (3-32 characters)", into.nickname);
	}

	{
		auto scope = scoped_child("connect cancel");

		ImGui::Separator();

		if (ImGui::Button("Connect!")) {
			result = true;
			//show = false;
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel")) {
			show = false;
		}
	}

	return result;
}

